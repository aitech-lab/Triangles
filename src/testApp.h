#pragma once

#include "ofMain.h"
#include "FractalRect.h"
#include "FractalTriangle.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void splitRects(FractalRect r);
		void splitTriangles(FractalTriangle& t);
		void drawTriangles(FractalTriangle& t);

		void calcIntegral(ofImage bitmap);
		
		ofImage image;
		vector<FractalRect>     rects;
		vector<FractalTriangle> triangles;

		unsigned long* integral;
		FractalTriangle* triangle;
		unsigned int depth;

		bool doSave;
		float timer;


private:
		bool getIntegralFromTriangle(FractalTriangle& t);
};
