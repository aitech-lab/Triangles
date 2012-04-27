#include "FractalRect.h"
#include "ofMain.h"

float FractalRect::P =  1.6180339887498948482;

FractalRect::FractalRect(float x, float y, float w, float h, int l) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->l = l;
}

FractalRect::~FractalRect(void) {

}


void FractalRect::split(void) {
	if(w>h) {
		float ww = ofRandom(100.0) > 50.0 ? w/P : w/P/P;
		c1 = new FractalRect(x   , y,   ww, h, l+1);
		c2 = new FractalRect(x+ww, y, w-ww, h, l+1);
	} else {
		float hh = ofRandom(100.0) > 50.0 ? h/P : w/P/P;
		c1 = new FractalRect(x, y   , w,   hh, l+1);
		c2 = new FractalRect(x, y+hh, w, h-hh, l+1);
	}
}
