#pragma once
class FractalTriangle
{
public:
	FractalTriangle(float x1, float y1, float x2, float y2, float x3, float y3, int l=0);
	~FractalTriangle(void);

	void split(void);

	float x1 , x2 , x3 , y1 , y2 , y3 ;
	float x12, x23, x31, y12, y23, y31;
	FractalTriangle *t1, *t2, *t3, *t4;

	float r1, r2, r3;
	float g1, g2, g3;
	float b1, b2, b3;
	float r, g, b;
	bool hasIntegral;


	int l;
};

