#include "KinectSkeleton.h"
#include "CatmullRom.h"


// [x, y, w, h] px
float KinectSkeleton::scull_uv[4] = { 236,  15,  91, 124};
float KinectSkeleton::body_uv [4] = {  18,  15, 187, 317};
float KinectSkeleton::lHand_uv[4] = { 205, 140,  75, 330};
float KinectSkeleton::rHand_uv[4] = { 300, 140,  75, 330};
float KinectSkeleton::lLeg_uv [4] = { 385,  15, 100, 460};
float KinectSkeleton::rLeg_uv [4] = { 490,  15, 100, 460};

float KinectSkeleton::lHumerus_uv    [4] = {615,  15, 60, 160};
float KinectSkeleton::rHumerus_uv    [4] = {670,  15, 60, 160};
float KinectSkeleton::lUlnaRadius_uv [4] = {615, 170, 52, 132};
float KinectSkeleton::rUlnaRadius_uv [4] = {675, 170, 52, 132};
float KinectSkeleton::lPalm_uv       [4] = {600, 320, 70,  80};
float KinectSkeleton::rPalm_uv       [4] = {675, 320, 70,  80};

float KinectSkeleton::lFemur_uv      [4] = {745,  10, 75, 235};
float KinectSkeleton::rFemur_uv      [4] = {830,  10, 75, 235};
float KinectSkeleton::lTibiaFibula_uv[4] = {745, 245, 65, 185};
float KinectSkeleton::rTibiaFibula_uv[4] = {840, 245, 65, 185};
float KinectSkeleton::lFoot_uv       [4] = {755, 425, 70, 110};
float KinectSkeleton::rFoot_uv       [4] = {830, 425, 70, 110};

NUI_SKELETON_POSITION_INDEX KinectSkeleton::spine[3] = {
	//NUI_SKELETON_POSITION_HIP_CENTER,
	NUI_SKELETON_POSITION_HEAD,
	NUI_SKELETON_POSITION_SHOULDER_CENTER,
	NUI_SKELETON_POSITION_SPINE,
	
};
	
NUI_SKELETON_POSITION_INDEX KinectSkeleton::leftHand[4] = {
	//NUI_SKELETON_POSITION_SHOULDER_CENTER,
	NUI_SKELETON_POSITION_SHOULDER_LEFT,
	NUI_SKELETON_POSITION_ELBOW_LEFT,
	NUI_SKELETON_POSITION_WRIST_LEFT, 
	NUI_SKELETON_POSITION_HAND_LEFT
};
	
NUI_SKELETON_POSITION_INDEX KinectSkeleton::rightHand[4] = {
	//NUI_SKELETON_POSITION_SHOULDER_CENTER,
	NUI_SKELETON_POSITION_SHOULDER_RIGHT,
	NUI_SKELETON_POSITION_ELBOW_RIGHT,
	NUI_SKELETON_POSITION_WRIST_RIGHT, 
	NUI_SKELETON_POSITION_HAND_RIGHT
};

NUI_SKELETON_POSITION_INDEX KinectSkeleton::leftLeg[4] = {
	//NUI_SKELETON_POSITION_HIP_CENTER,
	NUI_SKELETON_POSITION_HIP_LEFT,
	NUI_SKELETON_POSITION_KNEE_LEFT, 
	NUI_SKELETON_POSITION_ANKLE_LEFT,
	NUI_SKELETON_POSITION_FOOT_LEFT
};
			   
NUI_SKELETON_POSITION_INDEX KinectSkeleton::rightLeg[4] = {
	//NUI_SKELETON_POSITION_HIP_CENTER,
	NUI_SKELETON_POSITION_HIP_RIGHT, 
	NUI_SKELETON_POSITION_KNEE_RIGHT, 
	NUI_SKELETON_POSITION_ANKLE_RIGHT, 
	NUI_SKELETON_POSITION_FOOT_RIGHT
};

//--------------------------------------------------------------
void KinectSkeleton::setup(){

	Nui_Zero();
    liveTracking = Nui_Init() == S_OK;

	//ofSetBackgroundAuto(false);
    ofSetFrameRate(30);
    ofSetVerticalSync(true);

	rgb.allocate(640, 480, OF_IMAGE_COLOR);
	dph.allocate(320, 240, OF_IMAGE_COLOR);

	ofBackground(0, 0, 0);

	drawDebug     = false;
    recordingData = false;
    playRecord    = false;

    frame = 0;

	udpConnection.Create();
	udpConnection.Bind(11999);
	udpConnection.SetNonBlocking(true);
    message = "NODATA";


	fft = new float[256];
	soundPlayer.loadSound("004.mp3");
	soundPlayer.play();
	//soundPlayer.setVolume(1.0);

}

//--------------------------------------------------------------
void KinectSkeleton::update(){

	ofSoundUpdate();
	float* val = ofSoundGetSpectrum(256);
	
	for (int i = 0;i < 256; i++){
		// let the smoothed calue sink to zero:
		//fft[i] *= 0.96f;
		//fft[i] += (val[i]-fft[i])/10;

		fft[i] = val[i];
	}

	//soundPlayer.play();

    char udpMessage[100000];
	udpConnection.Receive(udpMessage,1);
	string request = udpMessage;
	// if we got not empty UDP request - response it with skeleton message
	if(request != ""){
        //printf("GOT REQEST: %s", request);
        int sent = udpConnection.Send(message.c_str(), message.length());
    }

    if (liveTracking) {
	    rgb.setFromPixels(rgb_buf, 640, 480, OF_IMAGE_COLOR_ALPHA, false);
	    dph.setFromPixels(dph_buf, 320, 240, OF_IMAGE_COLOR_ALPHA, false);
    }
}

void KinectSkeleton::exit() {

	Nui_UnInit();

	delete[] fft;

}

//--------------------------------------------------------------
void KinectSkeleton::draw(){

    if (liveTracking) {
        
        ofSetHexColor(0xFFFFFF);
	    if (rgb.width !=0 && !drawDebug) rgb.draw(0, 0, ofGetWidth(), ofGetHeight());
        if (dph.width !=0 &&  drawDebug) dph.draw(0, 0, ofGetWidth(), ofGetHeight());
        
		message = ""; // empty UDP message
        for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ ) {

            if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ) {
            			
			    glDisable(GL_TEXTURE_2D);
			    float fx=0, fy=0;
                USHORT fd =0;
			    for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++) {
                    Vector4 pos = SkeletonFrame.SkeletonData[i].SkeletonPositions[j];
				    NuiTransformSkeletonToDepthImageF( pos, &fx, &fy, &fd );
				    bonePoints[j].x = (int) ( fx * ofGetWidth()  + 0.5f );
				    bonePoints[j].y = (int) ( fy * ofGetHeight() + 0.5f );
				    ofSetHexColor(0xFFFFFF);
                    ofCircle(bonePoints[j].x, bonePoints[j].y, 5);
                    if(drawDebug) ofDrawBitmapString(ofToString(fd),bonePoints[j].x+7, bonePoints[j].y+5);
                    ofSetHexColor(0x00FF);
                    ofCircle(ofGetWidth()/3 + pos.x*100, ofGetHeight()/2 - pos.y*100, 3); 
                    ofSetHexColor(0xFF00);
                    ofCircle(ofGetWidth()/3 + pos.z*100, ofGetHeight()/2 - pos.y*100, 3);
                    
					// if recording, put data to skeletonData vector
					if (recordingData) {
                        skeletonData.push_back(pos.x);
                        skeletonData.push_back(pos.y);
                        skeletonData.push_back(pos.z);
                    }
					// form the UDP message
					message += ofToString(pos.x)+" "+ofToString(pos.y)+" "+ofToString(pos.z)+" ";

			    } // for (NUI_SKELETON_POSITION_COUNT)
            } // if (NUI_SKELETON_TRACKED)
        } // for(NUI_SKELETON_COUNT)


    } // if (liveTracking)


	if (playRecord) {
		// Create UDP message for recorded data and draw it
		if (skeletonData.size() > NUI_SKELETON_POSITION_COUNT*3) {
			ofSetHexColor(0xFF00);
			message="";
			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++) {
				int p = (frame*NUI_SKELETON_POSITION_COUNT+j)*3;
				float x = skeletonData[p  ];
				float y = skeletonData[p+1];
				float z = skeletonData[p+2];
				ofCircle(ofGetWidth()/2 +x*256, ofGetHeight()/2 - y*256, 5);
				message += ofToString(x)+" "+ofToString(y)+" "+ofToString(z)+" ";
			}
			frame = (frame+1)%(skeletonData.size()/3/NUI_SKELETON_POSITION_COUNT-1);
		}
    }

    if (recordingData) {
        ofSetHexColor(0xFF00000);
        ofCircle(30,30,10);
    }


	// draw the fft resutls:
	ofSetColor(255,255,255,255);
	
	float width = (float)2.0f;
	for (int i = 0;i < 256; i++){
		ofRect(i*(width+1)+5,ofGetHeight()-5,width,-(fft[i] * 200));
	}

}


//--------------------------------------------------------------
void KinectSkeleton::keyPressed(int key){

    if(key == 'r') {
		recordingData = !recordingData;
        if( recordingData) skeletonData.clear();
        if(!recordingData) {
            FILE * pFile;
            pFile = fopen ("data\\skeletonAnimation.dat","wb");
            if (pFile!=NULL) {
                fwrite (&skeletonData[0] , sizeof(float), skeletonData.size() , pFile);
                fclose (pFile);
            }
            skeletonData.clear();
        }
	}

    if(key == 'l') {
        if(recordingData) {
            recordingData = false;
            skeletonData.clear();
        }

        FILE * pFile;
        pFile = fopen ("data\\skeletonAnimation.dat","rb");
        if (pFile!=NULL) {
            // obtain file size:
            fseek (pFile , 0 , SEEK_END);
            long lSize = ftell (pFile)/sizeof(float);
            rewind (pFile);

            skeletonData.resize(lSize);
            fread (&skeletonData[0], sizeof(float), lSize, pFile);

            fclose (pFile);
        }
	}

    if(key == 'p') {
        playRecord = !playRecord;
    }

	if(key == 'd') {
		drawDebug = !drawDebug;
	}

    if(key == 'q') {
		LONG angle = 0;
		NuiCameraElevationGetAngle( &angle );
		NuiCameraElevationSetAngle( angle+5);
	}

	if(key == 'a') {
		LONG angle = 0;
		NuiCameraElevationGetAngle( &angle );
		NuiCameraElevationSetAngle( angle-5);
	}
}

//--------------------------------------------------------------
void KinectSkeleton::keyReleased(int key){

}

//--------------------------------------------------------------
void KinectSkeleton::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void KinectSkeleton::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void KinectSkeleton::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void KinectSkeleton::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void KinectSkeleton::windowResized(int w, int h){

}
/*
inline void ofPoint KinectSkeleton::CatmulSpline2(float t, ofPoint p0, ofPoint p1, ofPoint p2, ofPoint p3) {
	
	ofPoint out;

	float t2 = t * t;
	float t3 = t2 * t;
	
	out.x = 0.5f * ( ( 2.0f * p1.x ) +
		( -p0.x + p2.x ) * t +
		( 2.0f * p0.x - 5.0f * p1.x + 4 * p2.x - p3.x ) * t2 +
		( -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x ) * t3 );

	out.y = 0.5f * ( ( 2.0f * p1.y ) +
		( -p0.y + p2.y ) * t +
		( 2.0f * p0.y - 5.0f * p1.y + 4 * p2.y - p3.y ) * t2 +
		( -p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y ) * t3 );
}
*/