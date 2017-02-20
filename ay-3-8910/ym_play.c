/* Parse/Play a YM AY-3-8910 Music File */
/* Used file info found here: http://leonard.oxg.free.fr/ymformat.html */
/* Also useful: ftp://ftp.modland.com/pub/documents/format_documentation/Atari%20ST%20Sound%20Chip%20Emulator%20YM1-6%20(.ay,%20.ym).txt */

#define VERSION "0.4"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <sys/resource.h>

#include <bcm2835.h>

#include "ay-3-8910.h"
#include "display.h"
#include "load_ym.h"

#define AY38910_CLOCK	1000000	/* 1MHz on our board */

static int play_music=1;
static int dump_info=0;
static int visualize=1;
static int display_type=DISPLAY_I2C;
static int shift_size=16;
static int music_repeat=0;
static int music_paused=0;
static int music_loop=0;
static int diff_mode=0;

static void quiet_and_exit(int sig) {

	if (play_music) {
		quiet_ay_3_8910(shift_size);
		close_ay_3_8910();
	}

	display_shutdown(display_type);

	printf("Quieting and exiting\n");
	_exit(0);

}

static void print_help(int just_version, char *exec_name) {

	printf("\nym_play version %s by Vince Weaver <vince@deater.net>\n\n",VERSION);
	if (just_version) exit(0);

	printf("This plays YM5 music files out of the VMW AY-3-8910 device\n");
	printf("on a suitably configured Raspberry Pi machine.\n\n");

	printf("Usage:\n");
	printf("\t%s [-h] [-v] [-d] [-i] [-t] [-m] [-s] [-n] filename\n\n",
		exec_name);
	printf("\t-h: this help message\n");
	printf("\t-v: version info\n");
	printf("\t-d: print debug messages\n");
	printf("\t-D: debug diff mode\n");
	printf("\t-i: use i2c LEDs for visualization\n");
	printf("\t-t: use ASCII text visualization\n");
	printf("\t-m: use mono (only one AY-3-8910 hooked up)\n");
	printf("\t-s: use stereo (two AY-3-8910s hooked up)\n");
	printf("\t-n: no sound (useful when debugging)\n");
	// printf("\t-k: kiosk mode (standalone without keyboard/monitor)\n");
	printf("\tfilename: must be uncompressed YM5 file for now\n\n");

	exit(0);
}

static int play_song(char *filename) {

	unsigned char *ym_file;
	unsigned char frame[YM5_FRAME_SIZE];
	unsigned char last_frame[YM5_FRAME_SIZE];
	long int file_offset=0;

	int i,j;
	int length_seconds;
	double s,n,hz,diff;
	double max_a=0.0,max_b=0.0,max_c=0.0;

	int a_period,b_period,c_period,n_period,e_period;
	double a_freq=0.0, b_freq=0.0, c_freq=0.0,n_freq=0.0,e_freq=0.0;
	int new_a,new_b,new_c,new_n,new_e;

	int display_command=0,result;

	struct timeval start,next;

	struct ym_song_t ym_song;

	printf("\nPlaying song %s\n",filename);

	result=load_ym_song(filename,&ym_song);
	if (result<0) {
		return -1;
	}

	ym_file=ym_song.file_data;

	gettimeofday(&start,NULL);

	/**********************/
	/* Print song summary */
	/**********************/

	printf("\tYM%d",ym_song.type);
	printf("\tSong attributes (%d) : ",ym_song.attributes);
	printf("Interleaved=%s\n",ym_song.interleaved?"yes":"no");
	if (ym_song.num_digidrum>0) {
		printf("Num digidrum samples: %d\n",ym_song.num_digidrum);
	}
	printf("\tFrames: %d, ",ym_song.num_frames);
	printf("Chip clock: %d Hz, ",ym_song.master_clock);
	printf("Frame rate: %d Hz, ",ym_song.frame_rate);
	if (ym_song.frame_rate!=50) {
		fprintf(stderr,"FIX ME framerate %d\n",ym_song.frame_rate);
		exit(1);
	}
	length_seconds=ym_song.num_frames/ym_song.frame_rate;
	printf("Length=%d:%02d\n",length_seconds/60,length_seconds%60);
	printf("\tLoop frame: %d, ",ym_song.loop_frame);
	printf("Extra data size: %d\n",ym_song.extra_data);
	printf("\tSong name: %s\n",ym_song.song_name);
	printf("\tAuthor name: %s\n",ym_song.author);
	printf("\tComment: %s\n",ym_song.comment);

	/******************/
	/* Play the song! */
	/******************/

	file_offset=ym_song.data_begin;

	i=0;
	while(1) {

	if (!music_paused) {
		if (ym_song.interleaved) {
			for(j=0;j<ym_song.frame_size;j++) {
				frame[j]=ym_file[file_offset+j*ym_song.num_frames];
			}
			file_offset++;
		}
		else {
			memcpy(frame,&ym_file[file_offset],ym_song.frame_size);
			file_offset+=ym_song.frame_size;
		}

		/****************************************/
		/* Write out the music			*/
		/****************************************/


		a_period=((frame[1]&0xf)<<8)|frame[0];
		b_period=((frame[3]&0xf)<<8)|frame[2];
		c_period=((frame[5]&0xf)<<8)|frame[4];
		n_period=frame[6]&0x1f;
		e_period=((frame[12]&0xff)<<8)|frame[11];

		if (a_period>0) a_freq=ym_song.master_clock/(16.0*(double)a_period);
		if (b_period>0) b_freq=ym_song.master_clock/(16.0*(double)b_period);
		if (c_period>0) c_freq=ym_song.master_clock/(16.0*(double)c_period);
		if (n_period>0) n_freq=ym_song.master_clock/(16.0*(double)n_period);
		if (e_period>0) e_freq=ym_song.master_clock/(256.0*(double)e_period);

		if (dump_info) {
			printf("%05d:\tA:%04x B:%04x C:%04x N:%02x M:%02x ",
				i,a_period,b_period,c_period,n_period,frame[7]);

			printf("AA:%02x AB:%02x AC:%02x E:%04x,%02x %04x\n",
				frame[8],frame[9],frame[10],
				(frame[12]<<8)+frame[11],frame[13],
				(frame[14]<<8)+frame[15]);

			printf("\t%.1lf %.1lf %.1lf %.1lf %.1lf ",
				a_freq,b_freq,c_freq,n_freq, e_freq);
			printf("N:%c%c%c T:%c%c%c ",
				(frame[7]&0x20)?' ':'C',
				(frame[7]&0x10)?' ':'B',
				(frame[7]&0x08)?' ':'A',
				(frame[7]&0x04)?' ':'C',
				(frame[7]&0x02)?' ':'B',
				(frame[7]&0x01)?' ':'A');

			if (frame[8]&0x10) printf("VA: E ");
			else printf("VA: %d ",frame[8]&0xf);
			if (frame[9]&0x10) printf("VB: E ");
			else printf("VB: %d ",frame[9]&0xf);
			if (frame[10]&0x10) printf("VC: E ");
			else printf("VC: %d ",frame[10]&0xf);

			if (frame[13]==0xff) {
				printf("NOWRITE");
			}
			else {
				if (frame[13]&0x1) printf("Hold");
				if (frame[13]&0x2) printf("Alternate");
				if (frame[13]&0x4) printf("Attack");
				if (frame[13]&0x8) printf("Continue");
			}
			printf("\n");

			if (a_freq>max_a) max_a=a_freq;
			if (b_freq>max_b) max_b=b_freq;
			if (c_freq>max_c) max_c=c_freq;
		}

		if (diff_mode) {
			for(j=0;j<YM5_FRAME_SIZE;j++) {
				if (frame[j]!=last_frame[j]) {
					printf("%d: %d\n",i,j);
				}
			}

			memcpy(last_frame,frame,sizeof(frame));
		}

		/* Scale if needed */
		if (ym_song.master_clock!=AY38910_CLOCK) {

			if (a_period==0) new_a=0;
			else new_a=(double)AY38910_CLOCK/(16.0*a_freq);
			if (b_period==0) new_b=0;
			else new_b=(double)AY38910_CLOCK/(16.0*b_freq);
			if (c_period==0) new_c=0;
			else new_c=(double)AY38910_CLOCK/(16.0*c_freq);
			if (n_period==0) new_n=0;
			else new_n=(double)AY38910_CLOCK/(16.0*n_freq);
			if (e_period==0) new_e=0;
			else new_e=(double)AY38910_CLOCK/(256.0*e_freq);

			if (new_a>0xfff) {
				printf("A TOO BIG %x\n",new_a);
			}
			if (new_b>0xfff) {
				printf("B TOO BIG %x\n",new_b);
			}
			if (new_c>0xfff) {
				printf("C TOO BIG %x\n",new_c);
			}
			if (new_n>0x1f) {
				printf("N TOO BIG %x\n",new_n);
			}
			if (new_e>0xffff) {
				printf("E too BIG %x\n",new_e);
			}

			frame[0]=new_a&0xff;	frame[1]=(new_a>>8)&0xf;
			frame[2]=new_b&0xff;	frame[3]=(new_b>>8)&0xf;
			frame[4]=new_c&0xff;	frame[5]=(new_c>>8)&0xf;
			frame[6]=new_n&0x1f;
			frame[11]=new_e&0xff;	frame[12]=(new_e>>8)&0xff;

			if (dump_info) {
				printf("\t%04x %04x %04x %04x %04x\n",
					new_a,new_b,new_c,new_n,new_e);

			}

		}

		if (play_music) {
			for(j=0;j<13;j++) {
				write_ay_3_8910(j,frame[j],shift_size);
			}

			/* Special case.  Writing r13 resets it,	*/
			/* so special 0xff marker means do not write	*/
			if (frame[13]!=0xff) {
				write_ay_3_8910(13,frame[13],shift_size);
			}
		}
	}
		if (visualize) {
			if (display_type&DISPLAY_TEXT) {
				printf("\033[H\033[2J");
			}

			display_command=display_update(display_type,
						(frame[8]*11)/16,
						(frame[9]*11)/16,
						(frame[10]*11)/16,
						(a_freq)/150,
						(b_freq)/150,
						(c_freq)/150,
						i,ym_song.num_frames,
						filename,0);
		}

		/* Calculate time it took to play/visualize */
		gettimeofday(&next,NULL);
		s=start.tv_sec+(start.tv_usec/1000000.0);
		n=next.tv_sec+(next.tv_usec/1000000.0);
		diff=(n-s)*1000000.0;

		/* Delay until time for next update (often 50Hz) */
		if (play_music) {
			if (diff>0) bcm2835_delayMicroseconds(20000-diff);
			/* often 50Hz = 20000 */
			/* TODO: calculate correctly */
		}
		else {
			if (visualize) usleep(1000000/ym_song.frame_rate);
		}

		/* Calculate time it actually took, and print		*/
		/* so we can see if things are going horribly wrong	*/
		gettimeofday(&next,NULL);
		s=start.tv_sec+(start.tv_usec/1000000.0);
		n=next.tv_sec+(next.tv_usec/1000000.0);

		if (i%100==0) {
			hz=1/(n-s);
			printf("Done frame %d/%d, %.1lfHz\n",i,ym_song.num_frames,hz);
		}
		start.tv_sec=next.tv_sec;
		start.tv_usec=next.tv_usec;

		if (display_command==CMD_EXIT_PROGRAM) {
			free(ym_file);
			return CMD_EXIT_PROGRAM;
		}

		/* prev song */
		if (display_command==CMD_BACK) {
			free(ym_file);
			return CMD_BACK;
		}
		/* next song */
		if (display_command==CMD_FWD) {
			free(ym_file);
			return CMD_FWD;
		}

		/* rewind = Beginning of track */
		if (display_command==CMD_RW) {
			i=0;
			file_offset=ym_song.data_begin;
		}

		/* fastfwd = skip ahead 5s */
		if (display_command==CMD_FF) {
			i+=5*ym_song.frame_rate;
			if (ym_song.interleaved) {
				file_offset+=5*ym_song.frame_rate;
			}
			else {
				file_offset+=(5*ym_song.frame_rate)*ym_song.frame_size;
			}
		}


		if (display_command==CMD_PAUSE) {
			if (music_paused) {
				music_paused=0;
			}
			else {
				music_paused=1;
				quiet_ay_3_8910(shift_size);
			}
		}

		if (display_command==CMD_LOOP) {
			music_loop=!music_loop;
			if (music_loop) printf("MUSIC LOOP ON\n");
			else printf("MUSIC LOOP OFF\n");
		}


		/* increment frame */
		if (!music_paused) i++;

		/* Check to see if done with file */
		if (i>=ym_song.num_frames) {
			if (music_loop) {
				i=ym_song.loop_frame;
				if (ym_song.interleaved) {
					file_offset=ym_song.data_begin+i;
				}
				else {
					file_offset=ym_song.data_begin+i*ym_song.frame_size;
				}
			}
			else {
				break;
			}
		}

	}

	if (i>ym_song.num_frames) {
		printf("Fast-forwarded, skipping end check\n");
	}
	else {

		if (ym_song.interleaved) {
			file_offset+=15*ym_song.num_frames;
		}

		if (ym_song.type>3) {
			/* Read the tail of the file and ensure */
			/* it has the proper trailer */
			if (memcmp(&ym_file[file_offset],"End!",4)) {
				fprintf(stderr,"ERROR! Bad ending! %x\n",
					ym_file[file_offset]);
				return -1;
			}
		}
	}

	/* Free the ym file */
	free(ym_file);

	if (dump_info) {
		printf("Max a=%.2lf b=%.2lf c=%.2lf\n",max_a,max_b,max_c);
	}

	return 0;
}


int main(int argc, char **argv) {

	char filename[BUFSIZ]="intro2.ym";
	int result;
	int c;
	int next_song,first_song;

	/* Setup control-C handler to quiet the music	*/
	/* otherwise if you force quit it keeps playing	*/
	/* the last tones */
	signal(SIGINT, quiet_and_exit);

	/* Set to have highest possible priority */
	setpriority(PRIO_PROCESS, 0, -20);

	/* Parse command line arguments */
	while ((c = getopt(argc, argv, "dDmhvmsnitr"))!=-1) {
		switch (c) {
			case 'd':
				/* Debug messages */
				printf("Debug enabled\n");
				dump_info=1;
				break;
			case 'D':
				/* diff mode */
				printf("Diff mode\n");
				diff_mode=1;
				break;
			case 'h':
				/* help */
				print_help(0,argv[0]);
				break;
			case 'v':
				/* version */
				print_help(1,argv[0]);
				break;
			case 'm':
				/* mono sound */
				shift_size=8;
				break;
			case 's':
				/* stereo sound */
				shift_size=16;
				break;
			case 'n':
				/* no sound */
				play_music=0;
				break;
			case 'i':
				/* i2c visualization */
				display_type=DISPLAY_I2C;
				break;
			case 't':
				/* text visualization */
				display_type=DISPLAY_TEXT;
				break;
			case 'r':
				/* repeat */
				music_repeat=1;
				break;
			default:
				print_help(0,argv[0]);
				break;
		}
	}

	first_song=optind;
	next_song=0;

	/* Initialize the Chip interface */
	if (play_music) {
		result=initialize_ay_3_8910(1);
		if (result<0) {
			printf("Error initializing bcm2835!\n");
			printf("Maybe try running as root?\n\n");
			exit(0);
		}
	}

	/* Initialize the displays */
	if (visualize) {
		result=display_init(display_type);
		if (result<0) {
			printf("Error initializing display!\n");
			printf("Turning off display for now!\n");
			display_type=0;
		}
	}

	while(1) {

		if (argv[first_song+next_song]!=NULL) {
			strcpy(filename,argv[first_song+next_song]);
			next_song++;
		}
		else {
			break;
		}

		/* Play the song */
		result=play_song(filename);
		if (result==CMD_EXIT_PROGRAM) {
			break;
		}

		if (result==CMD_BACK) {
			next_song-=2;
			if (next_song<0) next_song=0;
		}
		if (result==CMD_FWD) {
			/* already point to next song */
		}


		/* Quiet down the chips */
		if (play_music) {
			quiet_ay_3_8910(shift_size);
		}

		usleep(500000);

	}

	/* Get ready to shut down */

	/* Quiet down the chips */
	if (play_music) {
		quiet_ay_3_8910(shift_size);
		close_ay_3_8910();
	}

	/* Clear out display */
	if (visualize) {
		display_shutdown(display_type);
	}

	return 0;
}
