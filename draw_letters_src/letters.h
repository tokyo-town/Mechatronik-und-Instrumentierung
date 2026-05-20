#ifndef LETTERS_H
#define LETTERS_H

/* 
    This file defines the coordinate patterns for drawing uppercase and lowercase letters.
    Each letter is represented as a sequence of (x,y) coordinates that the plotter will follow.
    The endPts arrays indicate where to lift the pen for each letter.
*/
int uppercase_letters_coordinates[26][400][2];
int uppercase_letters_endPts[26][3];

int lowercase_letters_coordinates[26][400][2];
int lowercase_letters_endPts[26][3];

#endif