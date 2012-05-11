#pragma once

#include "ofMain.h"
#include "FractalRect.h"
#include "FractalTriangle.h"
#include "Kinect.h"

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
		void splitTriangles(FractalTriangle& t);
		void drawTriangles(FractalTriangle& t);

		void calcIntegral(ofImage bitmap);
		
		vector<FractalTriangle> triangles;

		unsigned long* integralColor;
		unsigned long* integralDepth;

		unsigned int depth;

		bool doSave;
		float timer;
		Kinect kinect;
		ofImage depthImage;
		ofImage colorImage;


private:
		bool getIntegralFromTriangle(FractalTriangle& t);
		void calcIntegral();
};
