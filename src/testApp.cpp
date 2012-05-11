#include "testApp.h"

#define W 320
#define H 240

//--------------------------------------------------------------
void testApp::setup(){

	kinect.Nui_Init();
	
	depth  = 7;
	doSave = false;
	timer  = 0;
	integralColor = (unsigned long*) malloc(sizeof(unsigned long) * W * H * 3 * 4);
	integralDepth = (unsigned long*) malloc(sizeof(unsigned long) * W * H );

	

	//splitRects(FractalRect(0, 0, image.width-1, image.height-1));


	ofBackground(0xFFFFFF);
	ofSetBackgroundAuto(false);
	ofEnableAlphaBlending();
	ofSetCircleResolution(6);

	
}

void testApp::calcIntegral(){

	// integral depth
	unsigned char* pixels = kinect.depthBuffer;
	integralDepth[0] = pixels[0];
	
	for(unsigned int x=1; x<W; x++) integralDepth[x  ] = integralDepth[(x-1)  ] + pixels[x  *3];
	for(unsigned int y=1; y<H; y++) integralDepth[y*W] = integralDepth[(y-1)*W] + pixels[y*W*3];
	for(unsigned int x=1; x<W; x++)
		for(unsigned int y=1; y<H; y++)
			integralDepth[x+y*W] = integralDepth[(x-1) + y    *W] 
		                         + integralDepth[ x    + (y-1)*W] 
								 - integralDepth[(x-1) + (y-1)*W] 
								 + pixels       [(x    +  y   *W)*3];

	// integral color (per chanel)
	unsigned char* pixels = kinect.colorBuffer;
	integralColor[0] = pixels[0];
	
	for(unsigned int x=1; x<W*2; x++) {
		integralColor[x*3+0] = integralColor[(x-1)*3+0] + pixels[x*3+0];
		integralColor[x*3+1] = integralColor[(x-1)*3+1] + pixels[x*3+1];
		integralColor[x*3+2] = integralColor[(x-1)*3+2] + pixels[x*3+2];
	}
	
	for(unsigned int y=1; y<H*2; y++) {
		integralColor[y*W*3+0] = integralColor[(y-1)*W*3+0] + pixels[y*W*3+0];
		integralColor[y*W*3+1] = integralColor[(y-1)*W*3+1] + pixels[y*W*3+1];
		integralColor[y*W*3+2] = integralColor[(y-1)*W*3+2] + pixels[y*W*3+2];
	}

	for(unsigned int x=1; x<W*2; x++) {
		for(unsigned int y=1; y<H*2; y++) {
			integralColor[(x+y*W)*3+0] = integralColor[((x-1)+y*W)*3+0] + integralColor[(x+(y-1)*W)*3+0] - integralColor[((x-1)+(y-1)*W)*3+0] + pixels[(x+y*W)*3+0];
			integralColor[(x+y*W)*3+1] = integralColor[((x-1)+y*W)*3+1] + integralColor[(x+(y-1)*W)*3+1] - integralColor[((x-1)+(y-1)*W)*3+1] + pixels[(x+y*W)*3+1];
			integralColor[(x+y*W)*3+2] = integralColor[((x-1)+y*W)*3+2] + integralColor[(x+(y-1)*W)*3+2] - integralColor[((x-1)+(y-1)*W)*3+2] + pixels[(x+y*W)*3+2];
		}
	}
}

void testApp::splitTriangles(FractalTriangle& t) {
	
	if(t.l < depth) {
		t.split();
		getIntegralFromTriangle(*t.t1);
		getIntegralFromTriangle(*t.t2);
		getIntegralFromTriangle(*t.t3);
		getIntegralFromTriangle(*t.t4);

		splitTriangles(*t.t1);
		splitTriangles(*t.t2);
		splitTriangles(*t.t3);
		splitTriangles(*t.t4);

	} else {
		if (t.hasIntegral) triangles.push_back(t);
	}
}

bool testApp::getIntegralFromTriangle(FractalTriangle& t) {
	
	unsigned char* pixels = kinect.colorBuffer;
	int minx = (int)min(t.x1, min(t.x2, t.x3));
	int miny = (int)min(t.y1, min(t.y2, t.y3));
	int maxx = (int)max(t.x1, max(t.x2, t.x3));
	int maxy = (int)max(t.y1, max(t.y2, t.y3));
	
	if (minx>=0 && maxx<W && miny>0 && maxy<H) {
		t.r = (integralColor[(minx+miny*W)*3+0] + integralColor[(maxx+maxy*W)*3+0] - integralColor[(minx+maxy*W)*3+0] - integralColor[(maxx+miny*W)*3+0])/((float)(maxx-minx)*(maxy-miny));
		t.g = (integralColor[(minx+miny*W)*3+1] + integralColor[(maxx+maxy*W)*3+1] - integralColor[(minx+maxy*W)*3+1] - integralColor[(maxx+miny*W)*3+1])/((float)(maxx-minx)*(maxy-miny));
		t.b = (integralColor[(minx+miny*W)*3+2] + integralColor[(maxx+maxy*W)*3+2] - integralColor[(minx+maxy*W)*3+2] - integralColor[(maxx+miny*W)*3+2])/((float)(maxx-minx)*(maxy-miny));
		
		t.r1 = pixels[((int)t.x1*2+(int)t.y1*2*W)*3+0]; t.g1 = pixels[((int)t.x1*2+(int)t.y1*2*W)*3+1]; t.b1 = pixels[((int)t.x1*2+(int)t.y1*2*W)*3+2];
		t.r2 = pixels[((int)t.x2*2+(int)t.y2*2*W)*3+0];	t.g2 = pixels[((int)t.x2*2+(int)t.y2*2*W)*3+1];	t.b2 = pixels[((int)t.x2*2+(int)t.y2*2*W)*3+2];
		t.r3 = pixels[((int)t.x3*2+(int)t.y3*2*W)*3+0];	t.g3 = pixels[((int)t.x3*2+(int)t.y3*2*W)*3+1];	t.b3 = pixels[((int)t.x3*2+(int)t.y3*2*W)*3+2];
		
		float a = 0.8;
		t.r1 = t.r1*(1-a)+t.r*a; t.g1 = t.g1*(1-a)+t.g*a; t.b1 = t.b1*(1-a)+t.b*a;
		t.r2 = t.r2*(1-a)+t.r*a; t.g2 = t.g2*(1-a)+t.g*a; t.b2 = t.b2*(1-a)+t.b*a;
		t.r3 = t.r3*(1-a)+t.r*a; t.g3 = t.g3*(1-a)+t.g*a; t.b3 = t.b3*(1-a)+t.b*a;

		t.hasIntegral = true;

		return true;
	}
	return false;
}

void testApp::splitRects(FractalRect r) {

	if(r.l < 11) {
		r.split();
		splitRects(*r.c1);
		splitRects(*r.c2);
	}  else {
		r.r = (integral[((int)r.x+(int)r.y*W)*3+0] + integral[((int)(r.x+r.w)+(int)(r.y+r.h)*W)*3+0] - integral[((int)(r.x+r.w)+(int)r.y*W)*3+0] - integral[((int)r.x+(int)(r.y+r.h)*W)*3+0])/(r.w*r.h);
		r.g = (integral[((int)r.x+(int)r.y*W)*3+1] + integral[((int)(r.x+r.w)+(int)(r.y+r.h)*W)*3+1] - integral[((int)(r.x+r.w)+(int)r.y*W)*3+1] - integral[((int)r.x+(int)(r.y+r.h)*W)*3+1])/(r.w*r.h);
		r.b = (integral[((int)r.x+(int)r.y*W)*3+2] + integral[((int)(r.x+r.w)+(int)(r.y+r.h)*W)*3+2] - integral[((int)(r.x+r.w)+(int)r.y*W)*3+2] - integral[((int)r.x+(int)(r.y+r.h)*W)*3+2])/(r.w*r.h);
		rects.push_back(r);
	}
}

void testApp::exit() {
	free(integralColor);
	free(integralDepth);

	kinect.Nui_UnInit();
}

//--------------------------------------------------------------
void testApp::update(){
	
	timer += 1e-3;

	colorImage.setFromPixels((unsigned char *) kinect.colorBuffer, W*2, H*2, OF_IMAGE_COLOR_ALPHA, false);
	colorImage.update();
	depthImage.setFromPixels((unsigned char *) kinect.depthBuffer,   W,   H, OF_IMAGE_COLOR_ALPHA);
	depthImage.update();

	calcIntegral();

	float cx = W/2;
	float cy = H/2;
	float r = max(cx,cy)*4;
	float a = 3.141592654/6;
	float x1 = cx+r*sin(a+1*3.1415926*2/3); float y1 = cx+r*cos(a+1*3.1415926*2/3);
	float x2 = cx+r*sin(a+2*3.1415926*2/3); float y2 = cx+r*cos(a+2*3.1415926*2/3);
	float x3 = cx+r*sin(a+3*3.1415926*2/3); float y3 = cx+r*cos(a+3*3.1415926*2/3);
	
	triangle = new FractalTriangle(x1, y1, x2, y2, x3, y3);
	
	triangles.clear();
	splitTriangles(*triangle);

	
}

void testApp::drawTriangles(FractalTriangle& t) {

	float cx = (t.x1+t.x2+t.x3)/3.0;
	float cy = (t.y1+t.y2+t.y3)/3.0;
	float d = sqrt((mouseX-cx)*(mouseX-cx) + (mouseY-cy)*(mouseY-cy));
	d /= W/2.0;
	if (d > 1) d = 1;
	float noize = ofNoise(cx/1200-timer, cy/1200-timer/10.0);
	float r = sqrt((t.x1-t.x2)*(t.x1-t.x2)+(t.y1-t.y2)*(t.y1-t.y2))*sqrt(3.0)/6;

	// if (t.l>2 && t.l>(1-d)*depth || t.l >= depth) {
	// if (t.l>2 && ofRandom(100)>75 || t.l >= depth) {
	if (t.l > 2 && t.l >= 2+noize*(depth-2) || t.l >= depth) {
		
		if (t.hasIntegral) {
			ofFill();

			//if(t.l == depth && ofRandom(100)>99.5) 
			//	ofEnableBlendMode(OF_BLENDMODE_SCREEN);
			//else 
			//	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

			if(doSave) {
				ofSetColor(t.r, t.g, t.b, 400);
				ofTriangle(t.x1, t.y1, t.x2, t. y2, t.x3, t.y3);
			} else {
				glBegin(GL_TRIANGLES);
				glColor4f(t.r1/255.0, t.g1/255.0, t.b1/255.0, 0.1); glVertex2d(t.x1, t.y1);
				glColor4f(t.r2/255.0, t.g2/255.0, t.b2/255.0, 0.1); glVertex2d(t.x2, t.y2);
				glColor4f(t.r3/255.0, t.g3/255.0, t.b3/255.0, 0.1); glVertex2d(t.x3, t.y3);
				glEnd();
			}		

			if(t.l<depth-1) { 
			    //ofSetColor(0xff, 0xff, 0xff, 16);
			 	//ofSetColor(t.r, t.g, t.b, 32);
			 	//ofCircle(cx, cy, r);
			 	ofSetColor(t.r, t.g, t.b, 16);
			 	ofTriangle(t.x12, t.y12, t.x23, t.y23, t.x31, t.y31);
			 }

			
			if (t.l<depth-2) {
				ofNoFill();
				ofSetColor(0xFF, 0xFF, 0xFF, 32);
				ofCircle(cx, cy, r*2);
			}
		}

	} else {
		drawTriangles(*t.t1);
		drawTriangles(*t.t2);
		drawTriangles(*t.t3);
		drawTriangles(*t.t4);
	}



}

//--------------------------------------------------------------
void testApp::draw(){
	unsigned static int counter=0;

	int k = 100;
	// image.draw(0,0, w/2.0, h/2.0);

	// for (int i = 0; i<rects.size(); i++) {
	// 	FractalRect r = rects[i];
	// 	ofSetColor(r.r, r.g, r.b);
	//	ofRect(r.x, r.y, r.w-1, r.h-1);
	// }

	//drawTriangles(*triangle);
	
	ofDisableAlphaBlending();

	colorImage.draw( 0, 0, W, H);
	depthImage.draw( W, 0, W, H);


}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
