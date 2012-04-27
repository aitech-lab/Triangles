#include "FractalTriangle.h"
#include "ofMain.h"
#include <math.h>


FractalTriangle::FractalTriangle(float x1, float y1, float x2, float y2, float x3, float y3, int l) {
	
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
	this->x3 = x3;
	this->y3 = y3;

	this->l = l;
	hasIntegral = false;

	t1 = 0;
	t2 = 0;
	t3 = 0;
	t4 = 0;
}

FractalTriangle::~FractalTriangle(void) {

}

void FractalTriangle::split(void) {

	//float P = ofRandom(50.0) > 50.0 ? 1.6180339887498948482 : 2.6180339887498948482;
	float P = 2.0;

	float dx12 = (x2-x1); float dx23 = (x3-x2); float dx31 = (x1-x3);
	float dy12 = (y2-y1); float dy23 = (y3-y2); float dy31 = (y1-y3);
	float l12 = sqrt(dx12*dx12+dy12*dy12);
	float l23 = sqrt(dx23*dx12+dy23*dy23);
	float l31 = sqrt(dx31*dx12+dy31*dy31);

	x12 = x1+dx12 / P; x23 = x2+dx23 / P; x31 = x3+dx31 / P;
	y12 = y1+dy12 / P; y23 = y2+dy23 / P; y31 = y3+dy31 / P;

	t1 = new FractalTriangle(x1 , y1 , x12, y12, x31, y31, l+1);
	t2 = new FractalTriangle(x2 , y2 , x23, y23, x12, y12, l+1);
	t3 = new FractalTriangle(x3 , y3 , x31, y31, x23, y23, l+1);
	t4 = new FractalTriangle(x12, y12, x23, y23, x31, y31, l+1);

}
