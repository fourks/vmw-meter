/* Makes a clock */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "i2c_lib.h"

#define SEG_A 0x0001
#define SEG_B 0x0002
#define SEG_C 0x0004
#define SEG_D 0x0008
#define SEG_E 0x0010
#define SEG_F 0x0020
#define SEG_G 0x0040
#define SEG_H 0x0080
#define SEG_K 0x0100
#define SEG_M 0x0200
#define SEG_N 0x0400
#define SEG_P 0x0800
#define SEG_R 0x1000
#define SEG_S 0x2000
#define SEG_T 0x4000
#define SEG_U 0x8000

unsigned short font_7seg[10] = {
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,	// 0
	SEG_B|SEG_C,				// 1
	SEG_A|SEG_B|SEG_G|SEG_E|SEG_D,		// 2
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_G,		// 3
	SEG_B|SEG_C|SEG_F|SEG_G,		// 4
	SEG_A|SEG_C|SEG_D|SEG_F|SEG_G,		// 5
	SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,	// 6
	SEG_A|SEG_B|SEG_C,			// 7
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,	// 8
	SEG_A|SEG_B|SEG_C|SEG_F|SEG_G,		// 9
};

unsigned short font_16seg[128] = {
	0x0000,	// 0x00
	0x0000,	// 0x01
	0x0000,	// 0x02
	0x0000,	// 0x03
	0x0000,	// 0x04
	0x0000,	// 0x05
	0x0000,	// 0x06
	0x0000,	// 0x07
	0x0000,	// 0x08
	0x0000,	// 0x09
	0x0000,	// 0x0a
	0x0000,	// 0x0b
	0x0000,	// 0x0c
	0x0000,	// 0x0d
	0x0000,	// 0x0e
	0x0000,	// 0x0f
	0x0000,	// 0x10
	0x0000,	// 0x11
	0x0000,	// 0x12
	0x0000,	// 0x13
	0x0000,	// 0x14
	0x0000,	// 0x15
	0x0000,	// 0x16
	0x0000,	// 0x17
	0x0000,	// 0x18
	0x0000,	// 0x19
	0x0000,	// 0x1a
	0x0000,	// 0x1b
	0x0000,	// 0x1c
	0x0000,	// 0x1d
	0x0000,	// 0x1e
	0x0000,	// 0x1f
	0x0000,	// 0x20 ' '
	0x0000,	// 0x21
	0x0000,	// 0x22
	0x0000,	// 0x23
	0x0000,	// 0x24
	0x0000,	// 0x25
	0x0000,	// 0x26
	0x0000,	// 0x27
	0x0000,	// 0x28
	0x0000,	// 0x29
	0x0000,	// 0x2a
	0x0000,	// 0x2b
	0x0000,	// 0x2c
	0x0000,	// 0x2d
	0x0000,	// 0x2e
	0x0000,	// 0x2f
	0x0000,	// 0x30
	0x0000,	// 0x31
	0x0000,	// 0x32
	0x0000,	// 0x33
	0x0000,	// 0x34
	0x0000,	// 0x35
	0x0000,	// 0x36
	0x0000,	// 0x37
	0x0000,	// 0x38
	0x0000,	// 0x39
	0x0000,	// 0x3a
	0x0000,	// 0x3b
	0x0000,	// 0x3c
	0x0000,	// 0x3d
	0x0000,	// 0x3e
	0x0000,	// 0x3f
	0x0000,	// 0x40 '@'
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_G|SEG_H|SEG_U|SEG_P,	// 0x41 'A'
	SEG_A|SEG_B|SEG_E|SEG_F|SEG_G|SEG_H|SEG_N|SEG_R|SEG_U,	// 0x42 'B'
	SEG_A|SEG_B|SEG_E|SEG_F|SEG_G|SEG_H,			// 0x43 'C'
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_M|SEG_S,	// 0x44 'D'
	SEG_A|SEG_B|SEG_E|SEG_F|SEG_G|SEG_U|SEG_H,		// 0x45 'E'
	SEG_A|SEG_B|SEG_G|SEG_H|SEG_U|SEG_P,			// 0x46 'F'
	SEG_A|SEG_B|SEG_D|SEG_E|SEG_F|SEG_G|SEG_H|SEG_P,	// 0x47 'G'
	SEG_C|SEG_D|SEG_G|SEG_H|SEG_U|SEG_P,			// 0x48 'H'
	SEG_A|SEG_B|SEG_E|SEG_F|SEG_M|SEG_S,			// 0x49 'I'
	SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,				// 0x4a 'J'
	SEG_N|SEG_R|SEG_H|SEG_U|SEG_G,				// 0x4b 'K'
	SEG_E|SEG_F|SEG_G|SEG_H,				// 0x4c 'L'
	SEG_C|SEG_D|SEG_G|SEG_H|SEG_K|SEG_N,			// 0x4d 'M'
	SEG_C|SEG_D|SEG_G|SEG_H|SEG_K|SEG_R,			// 0x4e 'N'
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G|SEG_H,	// 0x4f 'O'
	SEG_A|SEG_B|SEG_C|SEG_G|SEG_H|SEG_U|SEG_P,		// 0x50 'P'
	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G|SEG_H|SEG_R,	// 0x51 'Q'
	SEG_A|SEG_B|SEG_C|SEG_G|SEG_H|SEG_P|SEG_R|SEG_U,	// 0x52 'R'
	SEG_A|SEG_B|SEG_D|SEG_E|SEG_F|SEG_H|SEG_U|SEG_P,	// 0x53 'S'
	SEG_A|SEG_B|SEG_M|SEG_S,				// 0x54 'T'
	SEG_C|SEG_D|SEG_E|SEG_F|SEG_G|SEG_H,			// 0x55 'U'
	SEG_H|SEG_G|SEG_T|SEG_M,				// 0x56 'V'
	SEG_C|SEG_D|SEG_H|SEG_G|SEG_T|SEG_R,			// 0x57 'W'
	SEG_K|SEG_N|SEG_R|SEG_T,				// 0x58 'X'
	SEG_S|SEG_K|SEG_N,					// 0x59 'Y'
	SEG_A|SEG_B|SEG_F|SEG_E|SEG_N|SEG_T,			// 0x5a 'Z'
	0x0000,	// 0x5b
	0x0000,	// 0x5c
	0x0000,	// 0x5d
	0x0000,	// 0x5e
	0x0000,	// 0x5f
	0x0000,	// 0x60
	0x0000,	// 0x61
	0x0000,	// 0x62
	0x0000,	// 0x63
	0x0000,	// 0x64
	0x0000,	// 0x65
	0x0000,	// 0x66
	0x0000,	// 0x67
	0x0000,	// 0x68
	0x0000,	// 0x69
	0x0000,	// 0x6a
	0x0000,	// 0x6b
	0x0000,	// 0x6c
	0x0000,	// 0x6d
	0x0000,	// 0x6e
	0x0000,	// 0x6f
	0x0000,	// 0x70
	0x0000,	// 0x71
	0x0000,	// 0x72
	0x0000,	// 0x73
	0x0000,	// 0x74
	0x0000,	// 0x75
	0x0000,	// 0x76
	0x0000,	// 0x77
	0x0000,	// 0x78
	0x0000,	// 0x79
	0x0000,	// 0x7a
	0x0000,	// 0x7b
	0x0000,	// 0x7c
	0x0000,	// 0x7d
	0x0000,	// 0x7e
	0x0000,	// 0x7f
};

int decode_key(long long keycode) {

	int key=0;

	switch(keycode) {
		case 0x0001:	key=0;	break;
		case 0x0002:	key=1;	break;
		case 0x0004:	key=2;	break;
		case 0x0008:	key=3;	break;
		case 0x0010:	key=4;	break;
		case 0x0020:	key=5;	break;
		case 0x0040:	key=6;	break;
		case 0x0080:	key=7;  break;
		case 0x0100:	key=8;	break;
		case 0x0200:	key=9;	break;
		case 0x0400:	key=10; break;
		case 0x0800:	key=11;	break;
		default:	key=-1;	break;
	}
	return key;
}

#define FIELD_MONTH_1  0
#define FIELD_MONTH_2  1
#define FIELD_DATE_1   2
#define FIELD_DATE_2   3
#define FIELD_YEAR_1   4
#define FIELD_YEAR_2   5
#define FIELD_YEAR_3   6
#define FIELD_YEAR_4   7
#define FIELD_HOUR_1   8
#define FIELD_HOUR_2   9
#define FIELD_MIN_1   10
#define FIELD_MIN_2   11
#define FIELD_DONE    12

long long old_keypad=0;

time_t keypad_change_time(long long keypad_in) {

	time_t delta_time=0,entered_time;
	struct tm new_time;
	int keypresses=0;
	int which_field=FIELD_MONTH_1,which_key=0;
	long long keypad_result,keypad_change;
	int done=0;
	int new_month=0;
	int new_date=0,new_year=0,new_hour=0,new_minute=0;

	while(1) {

		keypad_result=read_keypad();

		keypad_change=old_keypad&~keypad_result;
		if (keypad_change) {
			which_key=decode_key(keypad_change);
			printf("Keypad: %d %d\n",keypresses,which_key);

			switch(which_field) {
				case FIELD_MONTH_1:
					if (which_key>1) {
						new_month=which_key;
						printf("Month now %d\n",new_month);
						which_field=FIELD_DATE_1;
					}
					else {
						new_month=which_key*10;
						which_field=FIELD_MONTH_2;
					}
					break;
				case FIELD_MONTH_2:
					new_month+=which_key;
					printf("Month now %d\n",new_month);
					which_field=FIELD_DATE_1;
					break;
				case FIELD_DATE_1:
					new_date=which_key*10;
					which_field=FIELD_DATE_2;
					break;
				case FIELD_DATE_2:
					new_date+=which_key;
					printf("Day now %d\n",new_date);
					which_field=FIELD_YEAR_1;
					break;
				case FIELD_YEAR_1:
					new_year=which_key*1000;
					which_field=FIELD_YEAR_2;
					break;
				case FIELD_YEAR_2:
					new_year+=which_key*100;
					which_field=FIELD_YEAR_3;
					break;
				case FIELD_YEAR_3:
					new_year+=which_key*10;
					which_field=FIELD_YEAR_4;
					break;
				case FIELD_YEAR_4:
					new_year+=which_key;
					printf("Year now %d\n",new_year);
					which_field=FIELD_HOUR_1;
					break;
				case FIELD_HOUR_1:
					new_hour=10*which_key;
					which_field=FIELD_HOUR_2;
					break;
				case FIELD_HOUR_2:
					new_hour+=which_key;
					printf("Hour now %d\n",new_hour);
					which_field=FIELD_MIN_1;
					break;
				case FIELD_MIN_1:
					new_minute=10*which_key;
					which_field=FIELD_MIN_2;
					break;
				case FIELD_MIN_2:
					new_minute+=which_key;
					printf("Minutes now %d\n",new_minute);
					which_field=FIELD_DONE;
					break;

			}

			keypresses++;
		}
		old_keypad=keypad_result;

		if (which_field==FIELD_DONE) done=1;
		if (done) break;

		usleep(20000);
	}

	new_time.tm_year=new_year-1900;
	new_time.tm_mon=new_month-1;
	new_time.tm_mday=new_date;
	new_time.tm_hour=new_hour;
	new_time.tm_min=new_minute;
	new_time.tm_sec=0;
	new_time.tm_isdst=-1;

	entered_time=mktime(&new_time);

	if (entered_time==-1) {
		printf("Error\n");
	}

	delta_time=(time(NULL))-entered_time;

	return delta_time;
}

int main(int argc, char **argv) {

 	int result,pm,hour,blink=0,count=0;

 	unsigned short display_buffer[8];
	long long keypad_result=0,keypad_change;

	time_t current_time;
	time_t delta_time=0,display_time;


	char *ctime_result;

	/* brightness 0 - 15 */
	result=init_display(13);

/*
	display_buffer[0]=font_16seg['A'];
	display_buffer[1]=font_16seg['U'];
	display_buffer[2]=font_16seg['G'];
	display_buffer[3]=font_7seg[2]|(font_7seg[8]<<8);
	display_buffer[4]=font_7seg[1]|(font_7seg[5]<<8);
	display_buffer[5]=font_7seg[2]|(font_7seg[4]<<8);
	display_buffer[6]=font_7seg[0]|(font_7seg[1]<<8);
	display_buffer[7]=font_7seg[1]|(font_7seg[3]<<8);
*/


	while(1) {

		current_time=time(NULL);
		//ctime_result=ctime(&current_time);
		display_time=current_time-delta_time;
		ctime_result=ctime(&display_time );

		//printf("%s\n",ctime_result);
		/* "Wed Aug 21 19:00:52 2013" */

		/* Month */
		display_buffer[0]=font_16seg[toupper(ctime_result[4])];
		display_buffer[1]=font_16seg[toupper(ctime_result[5])];
		display_buffer[2]=font_16seg[toupper(ctime_result[6])];

		/* Date */
		display_buffer[3]=font_7seg[ctime_result[8]-'0'];
		display_buffer[4]=font_7seg[ctime_result[9]-'0'];

		/* Year */
		display_buffer[5]=font_7seg[ctime_result[20]-'0'];
		display_buffer[6]=font_7seg[ctime_result[21]-'0'];
		display_buffer[7]=font_7seg[ctime_result[22]-'0'];
		display_buffer[7]|=(font_7seg[ctime_result[23]-'0'])<<8;

		/* hours */
		display_buffer[6]|=(font_7seg[ctime_result[11]-'0'])<<8;
		display_buffer[5]|=(font_7seg[ctime_result[12]-'0'])<<8;

		hour=((ctime_result[11]-'0')*10)+(ctime_result[12]-'0');
		if (hour>12) pm=1;

		/* minutes */
		display_buffer[4]|=(font_7seg[ctime_result[14]-'0'])<<8;
		display_buffer[3]|=(font_7seg[ctime_result[15]-'0'])<<8;

		/* AM/PM */

		if (pm) {
			display_buffer[6]|=0x8000;
		}
		else {
			display_buffer[5]|=0x8000;
		}

		/* Blink */
		if (blink) {
			display_buffer[3]|=0x8000;
			display_buffer[4]|=0x8000;
		}

		keypad_result=read_keypad();
//		if (keypad_result!=-1) {
//			printf("keypad: %lld\n",keypad_result);
//		}

		keypad_change=old_keypad&~keypad_result;
		if (keypad_change) {
			delta_time=keypad_change_time(keypad_change);
		}
		old_keypad=keypad_result;

        	update_display_raw(display_buffer);


		usleep(100000);
		count++;
		if (count==5) {
			blink=1;
		}
		if (count==10) {
			count=0;
			blink=0;
		}
	}

	return result;
}

