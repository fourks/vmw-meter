/* Makes a clock */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "i2c_lib.h"
#include "graphics_lib.h"


#define DIGIT_WIDTH 4
#define DIGIT_HEIGHT 5

struct digit_type {
  unsigned char line[5];
} digits[11] = {
{{
  0xe, // ###
  0xa, // # #
  0xa, // # #
  0xa, // # #
  0xe, // ###
}}, {{
  0x4, //  #
  0x4, //  #
  0x4, //  #
  0x4, //  #
  0x4, //  #
}}, {{
  0xe, // ###
  0x2, //   #
  0xe, // ###
  0x8, // #
  0xe, // ###
}}, {{
  0xe, // ###
  0x2, //   #
  0x6, //  ##
  0x2, //   #
  0xe, // ###
}}, {{
  0xa, // # #
  0xa, // # #
  0xe, // ###
  0x2, //   #
  0x2, //   #
}}, {{
  0xe, // ###
  0x8, // #
  0xe, // ###
  0x2, //   #
  0xe, // ###
}}, {{
  0xe, // ###
  0x8, // #
  0xe, // ###
  0xa, // # #
  0xa, // ###
}}, {{
  0xe, // ###
  0x2, //   #
  0x2, //   #
  0x2, //   #
  0x2, //   #
}}, {{
  0xe, // ###
  0xa, // # #
  0xe, // ###
  0xa, // # #
  0xe, // ###
}}, {{
  0xe, // ###
  0xa, // # #
  0xe, // ###
  0x2, //   #
  0x2, //   #
}},{{
  0x0, //
  0x4, //  #
  0x0, //
  0x4, //  #
  0x0, //
}},
  // ###
  // # #
  // ###
  // # #
  // # #
  //
  // ###
  // # #
  // ##
  // # #
  // ###
  //
  // ###
  // #
  // #
  // #
  // ###
  //
  // ##
  // # #
  // # #
  // # #
  // ##
  //
  // ###
  // #
  // ##
  // #
  // ###
  //
  // ###
  // #
  // ##
  // #
  // #
  //
  // ###
  // #
  // # #
  // # #
  // ###
  //
  // # #
  // # #
  // ###
  // # #
  // # #
  //
  // ###
  //  #
  //  #
  //  #
  // ###
  //
  //   #
  //   #
  //   #
  // # #
  //  #
  //
  // # #
  // # #
  // ##
  // # #
  // # #
  //
  // #
  // #
  // #
  // #
  // ###
  //
  // # #
  // ###
  // # #
  // # #
  // # #
  //
  //
  // #
  // ###
  // # #
  // # #
  //
  // ###
  // # #
  // # #
  // # #
  // ###
  //
  // ###
  // # #
  // ###
  // #
  // #
  //
  // ###
  // # #
  // # #
  // ###
  //   #
  //
  // ###
  // # #
  // ###
  // ##
  // # #
  //
  // ###
  // #
  // ###
  //   #
  // ###
  //
  // ###
  //  #
  //  #
  //  #
  //  #
  //
  // # #
  // # #
  // # #
  // # #
  // ###
  //
  // # #
  // # #
  // # #
  // # #
  //  #
  //
  // # #
  // # #
  // # #
  // ###
  // # #
  //
  // # #
  // # #
  //  #
  // # #
  // # #
  //
  // # #
  // # #
  //  #
  //  #
  //  #
  //
  // ###
  //   #
  //  #
  // #
  // ###
};


#define XSIZE 32
#define YSIZE 8

int put_digit(int c, int x, int y, int scroll_buffer[XSIZE][YSIZE]) {

    int h,w;

    for(h=0;h<DIGIT_HEIGHT;h++) {
       for(w=0;w<DIGIT_WIDTH;w++) {
          if (digits[c].line[h] & (1<<(DIGIT_WIDTH-1-w))) {
             scroll_buffer[x+w][y+h]=1;
          }
          else {
             scroll_buffer[x+w][y+h]=0;
          }
       }
    }

    return 0;
}

int main(int argc, char **argv) {

  int result,x,y;
  int x_scroll=0;

  int scroll_buffer[XSIZE][YSIZE];
  unsigned char display_buffer[8];


  /* init scroll buffer */
  for(x=0;x<XSIZE;x++) {
     for(y=0;y<YSIZE;y++) {
        scroll_buffer[x][y]=0;
     }
  }

  result=init_display();

  put_digit(2,0,0,scroll_buffer);

  /* Put scroll buffer into output buffer */
  for(y=0;y<YSIZE;y++) {
     /* clear the line before drawing to it */
     display_buffer[y]=0;
     for(x=0;x<XSIZE;x++) {
        if (scroll_buffer[x+x_scroll][y]) plotxy(display_buffer,x,y);
     }
  }
  update_display(display_buffer);

  return result;
}

