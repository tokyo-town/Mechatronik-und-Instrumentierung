#include "main.h"
#include "coords.h"

#ifndef __LETTERS_H
#define __LETTERS_H

void travel(Coord co);
void draw(Coord co);

void drawLetter(char letter, Coord pos);
void letterRect(char letter, float* x, float* y, float* w, float* h);
void drawText(char* chars, Coord pos);

extern const float LETTER_SCALE;

/* 
    This file defines the coordinate patterns for drawing uppercase and lowercase letters.
    Each letter is represented as a sequence of (x,y) coordinates that the plotter will follow.
    The endPts arrays indicate where to lift the pen for each letter.
*/

extern short int uppercase_letters_coordinates[26][100][2];
extern unsigned char uppercase_letters_endPts[26][3];

/* Lowercase coordinates: fixed max 400 points per letter */
extern short int lowercase_letters_coordinates[26][100][2];
extern unsigned char lowercase_letters_endPts[26][3];

#endif