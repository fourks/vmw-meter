#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define TWELTH_TWO 1.059463094359

// http://www.phy.mtu.edu/~suits/NoteFreqCalcs.html
double note_to_freq(char note, int flat, int sharp, int octave, double sub) {

	double freq=0.0;
	double step=0;

	switch(note) {
		case 'B': step= 2; break;
		case 'A': step= 0; break;
		case 'G': step=-2; break;
		case 'F': step=-4; break;
		case 'E': step=-5; break;
		case 'D': step=-7; break;
		case 'C': step=-9; break;
		default:
			fprintf(stderr,"Unknown note %c\n",note);
	}
	if (flat) step+=flat;
	if (sharp) step-=sharp;

	step-=(4-octave)*12;

	step+=sub/16.0;

	freq=440.0*pow(TWELTH_TWO,step);

	return freq;
}

/* https://sourceforge.net/p/ed2midi/code/ */
/* From a spreadsheet from ed2midi */
/* Ugh, ym5 changes octave on C, but looks like ED does at A */
/*      Octave  1       2       3       4       5
        A       255     128     64      32      16
        A#/B-   240     120     60      30      15
        B       228     114     57      28      14
        C       216     108     54      27      13
        C#/D-   204     102     51      25      12
        D       192      96     48      24      12
        D#/E-   180      90     45      22      11
        E       172      86     43      21      10
        F       160      80     40      20      10
        F#/G-   152      76     38      19      9
        G       144      72     36      18      9
        G#/A-   136      68     34      17      8		*/


/*      Octave  1	2       3       4       5       6
        C       	216     108     54      27      13
        C#/D-   	204     102     51      25      12
        D       	192      96     48      24      12
        D#/E-   	180      90     45      22      11
        E       	172      86     43      21      10
        F       	160      80     40      20      10
        F#/G-   	152      76     38      19      9
        G       	144      72     36      18      9
        G#/A-   	136      68     34      17      8
        A       255     128      64     32      16
        A#/B-   240     120      60     30      15
        B       228     114      57     28      14

*/


/*      Octave  1	2       3       4       5       6
        C       	216     108     54      27      13
        C#/D-   	204     102     51      25      12
        D       	192      96     48      24      12
        D#/E-   	180      90     45      22      11
        E       	172      86     43      21      10
        F       	160      80     40      20      10
        F#/G-   	152      76     38      19      9
        G       	144      72     36      18      9
        G#/A-   	136      68     34      17      8
        A       255     128      64     32      16	8
        A#/B-   240     120      60     30      15	8
        B       228     114      57     28      14	7

*/

static unsigned char ed_freqs[]={
/*      A   A#  B   C   C#  D   D#  E   F   F#  G   G# */
	255,240,228,216,204,192,180,172,160,152,144,136,
	128,120,114,108,102,96, 90, 86, 80, 76, 72, 68,
	64, 60, 57, 54, 51, 48, 45, 43, 40, 38, 36, 34,
	32, 30, 28, 27, 25, 24, 22, 21, 20, 19, 18, 17,
	16, 15, 14, 13, 12, 12, 11, 10, 10, 9,  9,  8,
	8,  8,  7};

/*	c1	c1
	d1	d1
	e1	e1
	f1	f1
	g1	g1
	a1	a2
	b1	b2
	c2	c2

 */
int note_to_ed(char note, int flat, int sharp, int octave) {

	int offset;

	switch(note) {
		case 'A': offset=12; break;
		case 'B': offset=14; break;
		case 'C': offset=3; break;
		case 'D': offset=5; break;
		case 'E': offset=7; break;
		case 'F': offset=8; break;
		case 'G': offset=10; break;
		default:
			fprintf(stderr,"Unknown note %c\n",note);
			return -1;
	}
	if (flat==1) offset--;
	if (sharp==1) offset++;
	if (sharp==2) offset+=2;

	offset+=((octave-1)*12);

	if ((offset<0) || (offset>63)) {
		fprintf(stderr,"Out of range offset %d\n",offset);
		return -1;
	}

	return ed_freqs[offset];
}



static struct note_mapping_type {
	int	actual;
	int	low;
	int	high;
} note_mapping[10][12]={

{ /* Octave 0 */
	{	16,	16,	16,	}, // "C 0"
	{	17,	17,	17,	}, // "C#0"
	{	18,	18,	18,	}, // "D 0"
	{	19,	19,	19,	}, // "D#0"
	{	20,	20,	21,	}, // "E 0"
	{	21,	21,	22,	}, // "F 0"
	{	23,	22,	23,	}, // "F#0"
	{	24,	24,	25,	}, // "G 0"
	{	26,	25,	26,	}, // "G#0"
	{	27,	27,	28,	}, // "A 0"
	{	29,	28,	29,	}, // "A#0"
	{	30,	30,	31,	}, // "B 0"
},
{ /* Octave 1 */
	{	32,	32,	33,	}, // "C 1"
	{	34,	34,	35,	}, // "C#1"
	{	36,	36,	37,	}, // "D 1"
	{	38,	38,	39,	}, // "D#1"
	{	41,	40,	41,	}, // "E 1"
	{	43,	43,	44,	}, // "F 1"
	{	46,	45,	46,	}, // "F#1"
	{	49,	48,	49,	}, // "G 1"
	{	51,	51,	52,	}, // "G#1"
	{	55,	54,	55,	}, // "A 1"
	{	58,	57,	58,	}, // "A#1"
	{	61,	61,	62,	}, // "B 1"
},
{ /* Octave 2 */
	{	65,	64,	65,	}, // "C 2"
	{	69,	68,	69,	}, // "C#2"
	{	73,	72,	73,	}, // "D 2"
	{	77,	77,	78,	}, // "D#2"
	{	82,	81,	82,	}, // "E 2"
	{	87,	86,	87,	}, // "F 2"
	{	92,	92,	93,	}, // "F#2"
	{	98,	97,	98,	}, // "G 2"
	{	103,	103,	104,	}, // "G#2"
	{	110,	109,	110,	}, // "A 2"
	{	116,	116,	117,	}, // "A#2"
	{	123,	123,	124,	}, // "B 2"
},
{ /* Octave 3 */
	{	130,	129,	131,	}, // "C 3"
	{	138,	137,	139,	}, // "C#3"
	{	146,	145,	147,	}, // "D 3"
	{	155,	154,	156,	}, // "D#3"
	{	164,	163,	165,	}, // "E 3"
	{	174,	173,	175,	}, // "F 3"
	{	185,	184,	186,	}, // "F#3"
	{	196,	195,	198,	}, // "G 3"
	{	207,	206,	209,	}, // "G#3"
	{	220,	119,	222,	}, // "A 3"
	{	233,	232,	235,	}, // "A#3"
	{	246,	245,	248,	}, // "B 3"
},
{ /* Octave 4 */
	{	261,	256,	266,	}, // "C 4"
	{	277,	272,	282,	}, // "C#4"
	{	293,	288,	298,	}, // "D 4"
	{	311,	306,	316,	}, // "D#4"
	{	329,	324,	334,	}, // "E 4"
	{	349,	344,	354,	}, // "F 4"
	{	370,	365,	375,	}, // "F#4"
	{	392,	387,	397,	}, // "G 4"
	{	415,	410,	420,	}, // "G#4"
	{	440,	435,	445,	}, // "A 4"
	{	466,	461,	471,	}, // "A#4"
	{	493,	488,	498,	}, // "B 4"
},
{ /* Octave 5 */
	{	523,	513,	533,	}, // "C 5"
	{	554,	544,	564,	}, // "C#5"
	{	587,	577,	597,	}, // "D 5"
	{	622,	612,	632,	}, // "D#5"
	{	659,	649,	669,	}, // "E 5"
	{	698,	688,	708,	}, // "F 5"
	{	740,	730,	750,	}, // "F#5"
	{	784,	774,	794,	}, // "G 5"
	{	830,	820,	840,	}, // "G#5"
	{	880,	870,	890,	}, // "A 5"
	{	932,	922,	942,	}, // "A#5"
	{	987,	977,	997,	}, // "B 5"
},
{ /* Octave 6 */
	{	1046,	1026,	1066,	}, // "C 6"
	{	1108,	1088,	1128,	}, // "C#6"
	{	1174,	1154,	1194,	}, // "D 6"
	{	1244,	1224,	1264,	}, // "D#6"
	{	1318,	1298,	1338,	}, // "E 6"
	{	1396,	1376,	1416,	}, // "F 6"
	{	1480,	1460,	1500,	}, // "F#6"
	{	1568,	1548,	1588,	}, // "G 6"
	{	1661,	1641,	1681,	}, // "G#6"
	{	1760,	1740,	1780,	}, // "A 6"
	{	1864,	1844,	1884,	}, // "A#6"
	{	1975,	1955,	1995,	}, // "B 6"
},
{ /* Octave 7 */
	{	2093,	2063,	2123,	}, // "C 7"
	{	2217,	2187,	2247,	}, // "C#7"
	{	2349,	2319,	2379,	}, // "D 7"
	{	2489,	2459,	2519,	}, // "D#7"
	{	2637,	2607,	2667,	}, // "E 7"
	{	2793,	2763,	2823,	}, // "F 7"
	{	2960,	2930,	2990,	}, // "F#7"
	{	3136,	3106,	3166,	}, // "G 7"
	{	3322,	3292,	3352,	}, // "G#7"
	{	3520,	3490,	3550,	}, // "A 7"
	{	3729,	3699,	3759,	}, // "A#7"
	{	3951,	3921,	3981,	}, // "B 7"
},
{ /* Octave 8 */
	{	4186,	4146,	4226,	}, // "C 8
	{	4434,	4394,	4474,	}, // "C#8
	{	4698,	4658,	5738,	}, // "D 8
	{	4978,	4938,	5018,	}, // "D#8
	{	5274,	5234,	5314,	}, // "E 8
	{	5587,	5547,	5627,	}, // "F 8
	{	5919,	5879,	5959,	}, // "F#8
	{	6271,	6231,	6311,	}, // "G 8
	{	6644,	6604,	6684,	}, // "G#8
	{	7040,	7000,	7080,	}, // "A 8
	{	7458,	7418,	7498,	}, // "A#8
	{	7902,	7862,	7942,	}, // "B 8
},
{ /* Octave 9 */
	{	8372,	8322,	8422,	}, // "C 9
	{	8869,	8819,	8919,	}, // "C#9
	{	9397,	9347,	9447,	}, // "D 9
	{	9956,	9906,	10006,	}, // "D#9
	{	10548,	10498,	10598,	}, // "E 9
	{	11175,	11125,	11225,	}, // "F 9
	{	11839,	11789,	11889,	}, // "F#9
	{	12543,	12493,	12593,	}, // "G 9
	{	13289,	13239,	13339,	}, // "G#9
	{	14080,	14030,	14130,	}, // "A 9
	{	14917,	14867,	14967,	}, // "A#9
	{	15804,	15754,	15854,	}, // "B 9
},
};

int freq_to_note(int f) {

	int octave=0,note=0;
	int i,j;

	for(i=0;i<10;i++) {
		if ((f>=note_mapping[i][0].low)&&(f<=note_mapping[i][11].high)) {
			octave=i;
			for(j=0;j<12;j++) {
				if ((f>=note_mapping[i][j].low)&&(f<=note_mapping[i][j].high)) {
					note=j;
				}
			}
			break;
		}
	}

//	printf("Found Octave %d note %d\n",octave,note);

	return (octave*12)+note;

}
