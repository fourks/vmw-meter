/* Makes a spinning pattern on the display */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/i2c-dev.h>

#include "i2c_lib.h"

#define HT16K33_REGISTER_ADDRESS_POINTER	0x00
#define HT16K33_REGISTER_SYSTEM_SETUP		0x20
#define HT16K33_REGISTER_KEY_DATA_POINTER	0x40
#define HT16K33_REGISTER_INT_ADDRESS_POINTER	0x60
#define HT16K33_REGISTER_DISPLAY_SETUP		0x80
#define HT16K33_REGISTER_ROW_INT_SET		0xA0
#define HT16K33_REGISTER_TEST_MODE		0xD0
#define HT16K33_REGISTER_DIMMING		0xE0

/* Blink rate */
#define HT16K33_BLINKRATE_OFF			0x00
#define HT16K33_BLINKRATE_2HZ			0x01
#define HT16K33_BLINKRATE_1HZ			0x02
#define HT16K33_BLINKRATE_HALFHZ		0x03

int shutdown_display(int i2c_fd) {

	if (i2c_fd>=0) {
		return close(i2c_fd);
	}

	return -1;
}



void reset_display(unsigned short *display_state) {

	int i;

	for(i=0;i<DISPLAY_LINES;i++) {
		display_state[i]=0;
	}
}

int update_display_rotated(int i2c_fd, int i2c_addr,
				unsigned short *display_state) {

	unsigned char buffer[17];

	int big_hack[8][8];

	int i,x,y,newi;

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			i2c_addr);
		return -1;
	}


   /* only update if there's been a change */
//     if ( (existing_state[i*2]!=state[i*2]) ||
//        (existing_state[(i*2)+1]!=state[(i*2)+1])) {

//        existing_state[i*2]=state[i*2];
//        existing_state[(i*2)+1]=state[(i*2)+1];

      /* ugh bug in backpack?  bit 0 is actually LED 1, bit 128 LED 0 */
      /* Verify that somehow the python code is outputting like this */
      /* Fix bits mirrored error */

      for(y=0;y<8;y++) {
	 for(x=0;x<8;x++) {
            big_hack[x][y]=((display_state[y]>>(x))&1);
         }
      }

      /* write out to hardware */
   buffer[0]=0x00;
   for(i=0;i<DISPLAY_LINES;i++) {
      /* Fix off by one error */


      /* reconstruct */

	/* no rotate */
#if 0
      buffer[(i*2)+1]=0;
      buffer[(i*2)+1]|=big_hack[6][i]<<0;
      buffer[(i*2)+1]|=big_hack[5][i]<<1;
      buffer[(i*2)+1]|=big_hack[4][i]<<2;
      buffer[(i*2)+1]|=big_hack[3][i]<<3;
      buffer[(i*2)+1]|=big_hack[2][i]<<4;
      buffer[(i*2)+1]|=big_hack[1][i]<<5;
      buffer[(i*2)+1]|=big_hack[0][i]<<6;
      buffer[(i*2)+1]|=big_hack[7][i]<<7;
#endif
	/* rotate 270 degrees clockwise */
#if 0
      newi=i;
      if (newi==0) newi=7;
      else newi-=1;
      buffer[(newi*2)+1]=0;
      buffer[(newi*2)+1]|=big_hack[i][7]<<7;
      buffer[(newi*2)+1]|=big_hack[i][6]<<6;
      buffer[(newi*2)+1]|=big_hack[i][5]<<5;
      buffer[(newi*2)+1]|=big_hack[i][4]<<4;
      buffer[(newi*2)+1]|=big_hack[i][3]<<3;
      buffer[(newi*2)+1]|=big_hack[i][2]<<2;
      buffer[(newi*2)+1]|=big_hack[i][1]<<1;
      buffer[(newi*2)+1]|=big_hack[i][0]<<0;
#endif
      /* rotate 90 degrees clockwise */
      newi=i;
//      if (newi==0) newi=7;
//      else newi-=1;
		buffer[(newi*2)+1]=0;
		buffer[(newi*2)+1]|=big_hack[7-i][0]<<6;
		buffer[(newi*2)+1]|=big_hack[7-i][1]<<5;
		buffer[(newi*2)+1]|=big_hack[7-i][2]<<4;
		buffer[(newi*2)+1]|=big_hack[7-i][3]<<3;
		buffer[(newi*2)+1]|=big_hack[7-i][4]<<2;
		buffer[(newi*2)+1]|=big_hack[7-i][5]<<1;
		buffer[(newi*2)+1]|=big_hack[7-i][6]<<0;
		buffer[(newi*2)+1]|=big_hack[7-i][7]<<7;

		buffer[(newi*2)+2]=0x00;
	}

	if ( (write(i2c_fd, buffer, 17)) !=17) {
		fprintf(stderr,"Erorr writing display!\n");
		return -1;
	}

	return 0;
}


int update_display(int i2c_fd, int i2c_addr, unsigned short *display_state) {

	unsigned char buffer[17];

	int i;

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			i2c_addr);
		return -1;
	}

	buffer[0]=0x00;

	for(i=0;i<8;i++) {
		buffer[1+(i*2)]=display_state[i]&0xff;
		buffer[2+(i*2)]=(display_state[i]>>8)&0xff;
	}

	if ( (write(i2c_fd, buffer, 17)) !=17) {
		fprintf(stderr,"Erorr writing display!\n");
      		return -1;
	}

	return 0;
}


/* Set brightness from 0 - 15 */
int set_brightness(int i2c_fd, int i2c_addr, int value) {

	unsigned char buffer[17];

	if ((value<0) || (value>15)) {
		fprintf(stderr,"Brightness value of %d out of range (0-15)\n",
			value);
		return -1;
	}

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			i2c_addr);
		return -1;
	}

	/* Set Brightness */
	buffer[0]= HT16K33_REGISTER_DIMMING | value;
	if ( (write(i2c_fd, buffer, 1)) !=1) {
		fprintf(stderr,"Error setting brightness!\n");
		return -1;
	}

	return 0;
}

/* Read keypad */
long long read_keypad(int i2c_fd, int i2c_addr) {

	unsigned char keypad_buffer[6];
	unsigned char buffer[17];
	long long keypress;

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			i2c_addr);
		return -1;
	}

	buffer[0]= HT16K33_REGISTER_KEY_DATA_POINTER;
	if ( (write(i2c_fd, buffer, 1)) !=1) {
		fprintf(stderr,"Error setting data_pointer!\n");
      		return -1;
	}

	read(i2c_fd,keypad_buffer,6);

	//for(i=0;i<6;i++) printf("%x ",keypad_buffer[i]);
	//printf("\n");

	keypress = (long long)keypad_buffer[0] |
		((long long)keypad_buffer[1]<<8) |
		((long long)keypad_buffer[2]<<16) |
		((long long)keypad_buffer[3]<<24) |
		((long long)keypad_buffer[4]<<32) |
		((long long)keypad_buffer[5]<<40);

	return keypress;
}


/* should make the device settable */
int init_display(int i2c_fd, int i2c_addr, int brightness) {

	unsigned char buffer[17];

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			i2c_addr);
		return -1;
	}

	/* Turn the oscillator on */
	buffer[0]= HT16K33_REGISTER_SYSTEM_SETUP | 0x01;
	if ( (write(i2c_fd, buffer, 1)) !=1) {
		fprintf(stderr,"Error starting display!\n");
		return -1;
	}

	/* Turn Display On, No Blink */
	buffer[0]= HT16K33_REGISTER_DISPLAY_SETUP | HT16K33_BLINKRATE_OFF | 0x1;
	if ( (write(i2c_fd, buffer, 1)) !=1) {
		fprintf(stderr,"Error starting display!\n");
		return -1;
	}

	set_brightness(i2c_fd, i2c_addr, brightness);

	return 0;
}


int init_nunchuck(int i2c_fd) {

	unsigned char buffer[17];

	if (ioctl(i2c_fd, I2C_SLAVE, WII_NUNCHUCK_ADDRESS) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			WII_NUNCHUCK_ADDRESS);
		return -1;
        }

	/* Start the nunchuck */
	/* Note: official Nintendo one apparently works by sending 0x40/0x00 */
	/* but third party ones you sed 0xf0/0x55/0xfb/0x00 ???		     */
	/* info from: http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Nuncuck */

        buffer[0]=0xf0;
        buffer[1]=0x55;

	if ( (write(i2c_fd, buffer, 2)) !=2) {
		fprintf(stderr,"Error starting nunchuck! %s\n",
			strerror(errno));
		return -1;
	}

	buffer[0]=0xfb;
	buffer[1]=0x00;

	if ( (write(i2c_fd, buffer, 2)) !=2) {
		fprintf(stderr,"Error starting nunchuck! %s\n",
			strerror(errno));
		return -1;
	}

	return 0;

}


int read_nunchuck(int i2c_fd, struct nunchuck_data *results) {

	char buffer[6];
	int result;

	if (ioctl(i2c_fd, I2C_SLAVE, WII_NUNCHUCK_ADDRESS) < 0) {
		fprintf(stderr,"Error setting i2c address %x\n",
			WII_NUNCHUCK_ADDRESS);
		return -1;
        }

	buffer[0]=0x00;
	if ( (write(i2c_fd, buffer, 1)) !=1) {
		fprintf(stderr,"Error enabling read!\n");
		return -1;
	}

	/* needed? */
	// usleep(100000);

	result=read(i2c_fd,buffer,6);

	if (result!=6) {
		printf("Error reading %d\n",result);
	}

	results->joy_x=buffer[0];
	results->joy_y=buffer[1];
	results->acc_x=(buffer[2]<<2) | ((buffer[5]>>2)&0x3);
        results->acc_y=(buffer[3]<<2) | ((buffer[5]>>4)&0x3);
        results->acc_z=(buffer[4]<<2) | ((buffer[5]>>6)&0x3);
	results->z_pressed=!(buffer[5]&1);
	results->c_pressed=!((buffer[5]&2)>>1);

	return 0;
}


int init_i2c(char *device) {

	int i2c_fd=-1;

	i2c_fd = open(device, O_RDWR);
	if (i2c_fd < 0) {
		fprintf(stderr,"Error opening i2c dev file %s\n",device);
		return -1;
	}

	return i2c_fd;
}
