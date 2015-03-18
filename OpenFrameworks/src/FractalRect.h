#pragma once
class FractalRect {

public:
	FractalRect(float x, float y, float w, float h, int l=0);
	~FractalRect(void);

	static float P;

	void split(void);

	float x, y, w, h;
	int l;
	float r, g, b;

	FractalRect* c1;
	FractalRect* c2;

};

