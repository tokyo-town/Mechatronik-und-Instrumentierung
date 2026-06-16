#include "letters.h"


#ifndef __DRAW_PATTERNS
#define __DRAW_PATTERNS

enum Pattern {
	RECTANGLE,
	DOUBLE_SQUARE,
	LETTER_A,
	ZIG_ZAG,
	STAR
};


void draw_pattern_offset(enum Pattern pattern, Coord offset);
void draw_pattern(enum Pattern pattern);

#endif