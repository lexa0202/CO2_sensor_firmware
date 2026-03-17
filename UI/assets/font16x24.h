#ifndef FONT16X24_H
#define FONT16X24_H

#include <stdint.h>

#define FONT_WIDTH        		16
#define FONT_HEIGHT       		24
#define FONT_GLYPH_WIDTH      	12
#define FONT_SIDE_PADDING    	 2
#define FONT_ADVANCE         	13
#define FONT_BASELINE_OFFSET     4
#define FONT_FIRST_CHAR   		32
#define FONT_LAST_CHAR    	   126

/* Каждый символ:
   24 строки
   по 16 бит (2 байта) на строку
*/
extern const uint8_t font16x24[][FONT_HEIGHT][2];

#endif
