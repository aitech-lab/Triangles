#ifndef _KINECT_SKELETON_
#define _KINECT_SKELETON_

#define _WINSOCKAPI_ 
#include <Windows.h>
#include <MSR_NuiApi.h>
#include "ofMain.h"
#include "ofxNetwork.h"


#define ERROR_CHECK( ret )  \
    if ( ret != S_OK ) {    \
        std::cout << "failed " #ret " " << ret << std::endl;    \
        exit();          \
    }


class KinectSkeleton : public ofBaseApp{

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
		
		void SendUDP();

		ofImage rgb;
		ofImage dph;
		
		ofImage boneImg;
		GLuint  boneTex;

		unsigned char rgb_buf[640*480*4];
		unsigned char dph_buf[320*240*4];

		bool drawDebug;
        bool liveTracking;
        bool recordingData;
        bool playRecord;
        unsigned  int frame;
        string message;
        vector<float> skeletonData;

		ofxUDPManager udpConnection;

//		SOUND

		ofSoundPlayer soundPlayer;
		float*        fft;


//		KINECT
		
		HRESULT					Nui_Init();
		void                    Nui_UnInit( );
		void                    Nui_GotDepthAlert( );
		void                    Nui_GotVideoAlert( );
		void                    Nui_GotSkeletonAlert( );
		void                    Nui_Zero();
		RGBQUAD                 Nui_ShortToQuad_Depth( USHORT s );
		void					drawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... );

	private:
		static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);
		
		// thread handling
		HANDLE        m_hThNuiProcess;
		HANDLE        m_hEvNuiProcessStop;

		HANDLE        m_hNextDepthFrameEvent;
		HANDLE        m_hNextVideoFrameEvent;
		HANDLE        m_hNextSkeletonEvent;
		HANDLE        m_pDepthStreamHandle;
		HANDLE        m_pVideoStreamHandle;
		/*
		HFONT         m_hFontFPS;
		HPEN          m_Pen[6];
		HDC           m_SkeletonDC;
		HBITMAP       m_SkeletonBMP;
		HGDIOBJ       m_SkeletonOldObj;
		int           m_PensTotal;
		*/
		
		NUI_SKELETON_FRAME SkeletonFrame;


		POINT         bonePoints[NUI_SKELETON_POSITION_COUNT];
		RGBQUAD       m_rgbWk[640*480];
	
		int           m_LastSkeletonFoundTime;
		bool          m_bScreenBlanked;
		int           m_FramesTotal;
		int           m_LastFPStime;
		int           m_LastFramesTotal;

		static NUI_SKELETON_POSITION_INDEX spine    [3];
		static NUI_SKELETON_POSITION_INDEX leftHand [4];
		static NUI_SKELETON_POSITION_INDEX rightHand[4];
		static NUI_SKELETON_POSITION_INDEX leftLeg  [4];
		static NUI_SKELETON_POSITION_INDEX rightLeg [4];
		
		static float scull_uv[4];
		static float body_uv [4];
		static float lHand_uv[4];
		static float rHand_uv[4];
		static float lLeg_uv [4];
		static float rLeg_uv [4];

		static float lHumerus_uv[4];
		static float rHumerus_uv[4];
		static float lUlnaRadius_uv[4];
		static float rUlnaRadius_uv[4];
		static float lPalm_uv[4];
		static float rPalm_uv[4];

		static float lFemur_uv[4];
		static float rFemur_uv[4];
		static float lTibiaFibula_uv[4];
		static float rTibiaFibula_uv[4];
		static float lFoot_uv[4];
		static float rFoot_uv[4];


};

#endif
