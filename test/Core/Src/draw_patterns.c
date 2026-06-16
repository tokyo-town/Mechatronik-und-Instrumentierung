#include "draw_patterns.h"

void move_pattern(enum Pattern pattern) {
	switch(pattern) {
		case SQUARE:
			move((Coord) {100,0});
			move((Coord) {100,100});
			move((Coord) {0,100});
			move((Coord) {0,0});
			
		case DOUBLE_SQUARE:
			/* schräges Quadrat im normalen Quadrat*/
			move((Coord) {100,100});
			move((Coord) {200,0});
			move((Coord) {100,-100});
			move((Coord) {0,0});
			move((Coord) {100,-100});
			move((Coord) {200,0});
			move((Coord) {100,100});
			move((Coord) {0,0});
			move((Coord) {100,0});
			move((Coord) {100,100});
			move((Coord) {0,100});
			move((Coord) {0,0});
			
			
			move((Coord) {100,10});
			move((Coord) {0,0});
			move((Coord) {100,10});
			move((Coord) {0,0});
			move((Coord) {100,10});
			move((Coord) {0,0});
			move((Coord) {100,10});
			move((Coord) {0,0});
			move((Coord) {100,10});
			move((Coord) {0,0});
			
		case LETTER_A:
			/* A */
			move((Coord){100,200});
			move((Coord){200,0});
			move((Coord){150,100});
			move((Coord){100,100});
			move((Coord) {0,0});
			
		case ZIG_ZAG:
			/* Zickzack */
			move( (Coord) {200,10});
			move( (Coord) {0,20});
			move( (Coord) {200,30});
			move( (Coord) {0,40});
			move( (Coord) {0,0}); 
			move( (Coord) {200,-10});
			move( (Coord) {0,-20});
			move( (Coord) {200,-30});
			move( (Coord) {0,-40});
			move( (Coord) {0,0});
			
		case STAR:
			/* Stern */
//			move((Coord){ -45, -10});
//			move((Coord){-100,  30});
//			move((Coord){ -30,  30});
//			move((Coord){   0, 100});		
//			move((Coord){  30,  30});
//			move((Coord){ 100,  30});
//			move((Coord){  45, -10});
//			move((Coord){  70,-100});
//			move((Coord){   0, -40});
//			move((Coord){ -70,-100});	
			
			move((Coord){ 25,  90});
			move((Coord){-30, 130});
			move((Coord){ 40, 130});
			move((Coord){ 70, 200});		
			move((Coord){100, 130});
			move((Coord){170, 130});
			move((Coord){115,  90});
			move((Coord){140,   0});
			move((Coord){ 70,  60});
			move((Coord){  0,   0});	
	}
}


void draw_pattern_offset(enum Pattern pattern, Coord offset) {
	stift(0);
	move(offset);
	
	draw_pattern(pattern);
}
	
	
void draw_pattern(enum Pattern pattern) {
	stift(1);
	move_pattern(pattern);
	stift(0);
 
  //zeichne(rechteck);
}