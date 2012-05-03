/*
#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>
*/
#include "KinectSkeleton.h"

void KinectSkeleton::Nui_Zero()
{
    m_hNextDepthFrameEvent	= NULL;
    m_hNextVideoFrameEvent	= NULL;
    m_hNextSkeletonEvent	= NULL;
    m_pDepthStreamHandle	= NULL;
    m_pVideoStreamHandle	= NULL;
    m_hThNuiProcess			= NULL;
    m_hEvNuiProcessStop		= NULL;
    
	/*
	ZeroMemory(m_Pen,sizeof(m_Pen));
    m_SkeletonDC			= NULL;
    m_SkeletonBMP			= NULL;
    m_SkeletonOldObj		= NULL;
    m_PensTotal				= 6;
    ZeroMemory(bonePoints,sizeof(bonePoints));
    m_LastSkeletonFoundTime = -1;
    m_bScreenBlanked		= false;
    m_FramesTotal			= 0;
    m_LastFPStime			= -1;
    m_LastFramesTotal		= 0;
	*/
}



HRESULT KinectSkeleton::Nui_Init() {

	HRESULT	hr;
	RECT	rc;

	m_hNextDepthFrameEvent	= CreateEventW( NULL, TRUE, FALSE, NULL );
    m_hNextVideoFrameEvent	= CreateEventW( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent	= CreateEventW( NULL, TRUE, FALSE, NULL );

    hr = NuiInitialize( 
        NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | 
		NUI_INITIALIZE_FLAG_USES_SKELETON | 
		NUI_INITIALIZE_FLAG_USES_COLOR);

    if( FAILED( hr ) ) {
        cout << "INITIALIZE FAILED";
        return hr;
    }

    hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );

    if( FAILED( hr ) ) {
        cout << "SKELETON ENABLE FAILED";
        return hr;
    }
	
	
    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );
	
    if( FAILED( hr ) ) {
        cout << "RGB IMAGE STREAM OPEN FAILED";
        return hr;
    }

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    if( FAILED( hr ) ) {
        cout << "DEPTH STREAM OPEN FAILED";
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hThNuiProcess		= CreateThread(NULL, 0, Nui_ProcessThread,this,0,NULL);

    return hr;
}



void KinectSkeleton::Nui_UnInit( ) {

    // Stop the Nui processing thread
    if(m_hEvNuiProcessStop!=NULL) {

        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);
        // Wait for thread to stop
        if(m_hThNuiProcess!=NULL) {
            WaitForSingleObject(m_hThNuiProcess, INFINITE);
            CloseHandle(m_hThNuiProcess);
        }
        CloseHandle(m_hEvNuiProcessStop);
    }

    NuiShutdown( );

    if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) ) {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }

    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) ) {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }

    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) ) {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }

}


DWORD WINAPI KinectSkeleton::Nui_ProcessThread(LPVOID pParam) {

    KinectSkeleton *pthis=(KinectSkeleton *) pParam;
    HANDLE                hEvents[4];
    int                   nEventIdx,t,dt;

    // Configure events to be listened on
    hEvents[0]=pthis->m_hEvNuiProcessStop;
    hEvents[1]=pthis->m_hNextDepthFrameEvent;
    hEvents[2]=pthis->m_hNextVideoFrameEvent;
    hEvents[3]=pthis->m_hNextSkeletonEvent;

    // Main thread loop
    while(1)
    {
        // Wait for an event to be signalled
        nEventIdx=WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

        // If the stop event, stop looping and exit
        if(nEventIdx==0)
            break;            

        // Perform FPS processing
        t = timeGetTime( );
        if( pthis->m_LastFPStime == -1 )
        {
            pthis->m_LastFPStime = t;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
        }
        dt = t - pthis->m_LastFPStime;
        if( dt > 1000 )
        {
            pthis->m_LastFPStime = t;
            int FrameDelta = pthis->m_FramesTotal - pthis->m_LastFramesTotal;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
			cout << FrameDelta << " ";
        }

	    // Perform skeletal panel blanking
        if( pthis->m_LastSkeletonFoundTime == -1 )
            pthis->m_LastSkeletonFoundTime = t;
        dt = t - pthis->m_LastSkeletonFoundTime;
        if( dt > 250 )
        {
            if( !pthis->m_bScreenBlanked )
            {
                //pthis->Nui_BlankSkeletonScreen( GetDlgItem( pthis->m_hWnd, IDC_SKELETALVIEW ) );
                pthis->m_bScreenBlanked = true;
            }
        }
	
        // Process signal events
        switch(nEventIdx)
        {
            case 1:
                pthis->Nui_GotDepthAlert();
                pthis->m_FramesTotal++;
                break;

            case 2:
                pthis->Nui_GotVideoAlert();
                break;

            case 3:
                pthis->Nui_GotSkeletonAlert( );
                break;
        }
    }

	
    return (0);
}

void KinectSkeleton::Nui_GotVideoAlert( ) {

	//cout << "RGB ";
	
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        &pImageFrame );
    if( FAILED( hr ) )
    {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 ) {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		memcpy(rgb_buf, pBuffer,640*480*4);
		for (int i = 0; i<640*480; i++) rgb_buf[i*4+3] = 255;
    } else {
        cout << "Buffer length of received texture is bogus\r\n";
    }

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}


void KinectSkeleton::Nui_GotDepthAlert( )
{
    
	//cout << "DPH ";
	
	const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &pImageFrame );

    if( FAILED( hr ) ) {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 ) {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

        // draw the bits to the bitmap
        RGBQUAD * rgbrun = m_rgbWk;
        USHORT * pBufferRun = (USHORT*) pBuffer;
        for( int y = 0 ; y < 240 ; y++ ) {
            for( int x = 0 ; x < 320 ; x++ ) {
                RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
                pBufferRun++;
                *rgbrun = quad;
                rgbrun++;
            }
        }

		memcpy(dph_buf, m_rgbWk, 320*240*4);

    } else {
        cout << "\nBuffer length of received texture is bogus\n";
    }

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
}


void KinectSkeleton::Nui_GotSkeletonAlert( ) {

    //cout << "SKL ";
	
    HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );

    bool bFoundSkeleton = true;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ ) {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ) {
            bFoundSkeleton = false;
        }
    }

    // no skeletons!
    if( bFoundSkeleton ) {
        return;
    }

    // smooth out the skeleton data
    NuiTransformSmooth(&SkeletonFrame, NULL);

    // we found a skeleton, re-start the timer
    m_bScreenBlanked = false;
    m_LastSkeletonFoundTime = -1;

    // draw each skeleton color according to the slot within they are found.
    //
    bool bBlank = true;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ ) {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ) {
            //Nui_DrawSkeleton( bBlank, &SkeletonFrame.SkeletonData[i], GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), i );
            // Draw the joints in a different color
			bBlank = false;
        }
    }

    //Nui_DoDoubleBuffer(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), m_SkeletonDC);

}



RGBQUAD KinectSkeleton::Nui_ShortToQuad_Depth( USHORT s ) {
    USHORT RealDepth = (s & 0xfff8) >> 3;
    USHORT Player = s & 7;

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

    switch( Player )
    {
    case 0:
        q.rgbRed = l / 2;
        q.rgbBlue = l / 2;
        q.rgbGreen = l / 2;
        break;
    case 1:
        q.rgbRed = l;
        break;
    case 2:
        q.rgbGreen = l;
        break;
    case 3:
        q.rgbRed = l / 4;
        q.rgbGreen = l;
        q.rgbBlue = l;
        break;
    case 4:
        q.rgbRed = l;
        q.rgbGreen = l;
        q.rgbBlue = l / 4;
        break;
    case 5:
        q.rgbRed = l;
        q.rgbGreen = l / 4;
        q.rgbBlue = l;
        break;
    case 6:
        q.rgbRed = l / 2;
        q.rgbGreen = l / 2;
        q.rgbBlue = l;
        break;
    case 7:
        q.rgbRed = 255 - ( l / 2 );
        q.rgbGreen = 255 - ( l / 2 );
        q.rgbBlue = 255 - ( l / 2 );
    }

    return q;
}
