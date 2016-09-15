// Video.cpp: implementation of the CVideo class.
//
//////////////////////////////////////////////////////////////////////


#include "CVideo.h"

#include "drm_game.h"
#include "errors.h"


// This simple Video class will initialize a virtual screen buffer
// to display in a window, and will initialize DirectDraw and a double-buffer
// system for fullscreen mode.

// NOTE FOR WINDOWED MODE SPEED
// Note: windowed mode refresh could be made faster using DirectDraw in windowed mode
//       but then you have to support all desktop bit depths.

// NOTE FOR DOUBLE-BUFFER TECHNIQUE USED
// Primary and Secondary surfaces should reside in video memory and flip with hardware,
// the virtual screen (main memory) is copied to the hidden Secondary Surface in video
// memory and then the Primary and Secondary video memory surfaces are switched.

// Create a CVideo instance,
// Open your main app window,
// call Startup() and pass handler of your window, as well as initial mode (fullscreen or windowed)
// call SetPalette() to change the current palette
// call FinishUpdate() to refresh the physical screen with the virtual screen buffer


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideo::CVideo()
{
	// init windowed mode
	iZoomScreen = 0;	// this must reflect the initial state of the corresponding menu option in menu resource
	hDCMain = NULL;
	bmiMain = NULL;
	
	// init fullscreen mode
	DDr = NULL;
	DDrPrimary = NULL;
	DDrSecondary = NULL;
	DDrPalette = NULL;
}

CVideo::~CVideo()
{
	ClearFullScreenMode();
	ClearWindowedMode();
	FreeMem( pVirtual );
}


// Bit depth is assumed to be 8 (256 colour)
BOOL CVideo::Startup( HWND  hWnd,
					  int   PixelWidth,
					  int   PixelHeight,
					  BOOL  bStartFullScreen )
{
	BOOL result;
	
	// store window handle locally
	hWndMain = hWnd;

	// store requested display size
	Width = PixelWidth;
	Height = PixelHeight;

	// allocate the virtual screen
	pVirtual = (PIXEL*) AllocMem ( Width * Height * sizeof(PIXEL) );
	if ( !pVirtual )
		InitFail("CVideo::CVideo() - AllocMem() FAILED");

    // set initial mode
	bFullScreen = bStartFullScreen;

	if ( !bFullScreen )
		result = SetWindowedDisplay( hWnd );
	else
		result = SetFullScreenDisplay( hWnd, Width, Height, SCREEN_BPP );

	return result;
}


// Release windowed mode resources
void CVideo::ClearWindowedMode()
{
    if ( hDCMain ) {
        ReleaseDC (hWndMain, hDCMain);
	    hDCMain = NULL;
	}
    if (bmiMain) {
        GlobalFree (bmiMain);
		bmiMain = NULL;
	}
}


BOOL CVideo::SetWindowedDisplay( HWND hAppWindow )
{
    //RECT    Rect;

	if ( !bmiMain )
	{
		if ((bmiMain = (BITMAPINFO*)GlobalAlloc (GPTR, sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256)))==NULL)
			return FALSE;	//InitFail("CVideo::SetupWindowedMode() - no mem");

		// setup a BITMAPINFO to allow copying our video buffer to the desktop,
		// with color conversion as needed
		bmiMain->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiMain->bmiHeader.biWidth = Width;
		bmiMain->bmiHeader.biHeight= -(Height);
		bmiMain->bmiHeader.biPlanes = 1;
		bmiMain->bmiHeader.biBitCount = SCREEN_BPP;
		bmiMain->bmiHeader.biCompression = BI_RGB;
	}

	if ( !hDCMain )
	{
		if (!(hDCMain = GetDC( hAppWindow )))
			return FALSE;	//InitFail ("CVideo::SetupWindowedMode() - GetDC FAILED");
	}

	SetWindowZoom( iZoomScreen );
	
	//SetStretchBltMode (hDCMain, COLORONCOLOR);

    return TRUE;
}


// Toggle windowed mode between normal size and double size
void CVideo::SetWindowZoom( int iZoom )
{
	iZoomScreen = iZoom;

    int     iScrWidth, iScrHeight;
    int     iWinWidth, iWinHeight;
    int     iWinLeftBorder, iWinTopBorder;

	// center window on the desktop
    iScrWidth = GetSystemMetrics(SM_CXFULLSCREEN);
    iScrHeight = GetSystemMetrics(SM_CYFULLSCREEN);

	DWORD		dwStyle;
    dwStyle = WS_CAPTION | WS_POPUP | WS_VISIBLE;
    SetWindowLong( hWndMain, GWL_STYLE, dwStyle);

    iWinWidth = SCREEN_WIDTH << iZoomScreen;
    iWinLeftBorder = GetSystemMetrics(SM_CXFIXEDFRAME);
    iWinWidth += (iWinLeftBorder * 2);
    iWinHeight = SCREEN_HEIGHT << iZoomScreen;
    iWinTopBorder = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
    iWinHeight += iWinTopBorder;
    iWinHeight += GetSystemMetrics(SM_CYFIXEDFRAME);

    MoveWindow (hWndMain, (iScrWidth - iWinWidth)>>1, (iScrHeight - iWinHeight)>>1, iWinWidth, iWinHeight, TRUE);
    SetFocus(hWndMain);
    ShowWindow(hWndMain, SW_SHOW);
}


// -------------+
// ClearFullScreenMode
//				+ Release DirectDraw resources
// -------------+
void CVideo::ClearFullScreenMode()
{
    if (DDr != NULL)
    {
        SAFE_RELEASE( DDrPalette );
        
		// If the app is fullscreen, the back buffer is attached to the
        // primary. Releasing the primary buffer will also release any
        // attached buffers, so explicitly releasing the back buffer is not
        // necessary.
		
		SAFE_RELEASE (DDrPrimary);			// front and any attached backbuffers
        DDr->Release();
        DDr = NULL;
    }
}


// -------------+
// SetFullScreenDisplay
//				: Initialize DirectDraw and 2 surfaces for doublebuffer,
//				: set cooperative level, and go fullscreen
// -------------+
BOOL CVideo::SetFullScreenDisplay(HWND hWnd, int Width, int Height, int Bpp)
{
	DDSURFACEDESC	ddsd;				// DirectDraw surface description for allocating
    DDSCAPS         ddscaps;
    HRESULT         ddrval;
    
	DWORD		dwStyle;


    // create an instance of DirectDraw object
    //
    ddrval = DirectDrawCreate(NULL, &DDr, NULL);
    if (ddrval != DD_OK)
        return InitFail("DirectDrawCreate FAILED");


    // Change window attributes
    dwStyle = WS_POPUP | WS_VISIBLE;
    SetWindowLong (hWnd, GWL_STYLE, dwStyle);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE |
							SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

	// Get exclusive mode
	ddrval = DDr->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE |
											DDSCL_FULLSCREEN |
											DDSCL_ALLOWREBOOT);
	if (ddrval != DD_OK)
		return InitFail("SetCooperativeLevel() FAILED");

    // Switch from windows desktop to fullscreen

	ddrval = DDr->SetDisplayMode(Width, Height, Bpp);
	if (ddrval != DD_OK)
		return InitFail("SetDisplayMode() FAILED");

    // This is not really needed, except in certain cases. One case
    // is while using MFC. When the desktop is initally at 16bpp, a mode
    // switch to 8bpp somehow causes the MFC window to not fully initialize
    // and a CreateSurface will fail with DDERR_NOEXCLUSIVEMODE. This will
    // ensure that the window is initialized properly after a mode switch.

    ShowWindow(hWnd, SW_SHOW);
	
    // Create the primary surface with 1 back buffer. Always zero the
    // DDSURFACEDESC structure and set the dwSize member!

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
							| DDSCAPS_VIDEOMEMORY;

	// for fullscreen we use page flipping, for windowed mode, we blit the hidden surface to
	// the visible surface, in both cases we have a visible (or 'real') surface, and a hidden
	// (or 'virtual', or 'backbuffer') surface.
	ddsd.dwBackBufferCount = 1;

	ddrval = DDr->CreateSurface(&ddsd, &DDrPrimary, NULL);
	if (ddrval != DD_OK)
		return InitFail("CreateSurface() Primary FAILED");

    // Get a pointer to the back buffer

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_VIDEOMEMORY;
	ddrval = DDrPrimary->GetAttachedSurface(&ddscaps, &DDrSecondary);
	if (ddrval != DD_OK)
		return InitFail("GetAttachedSurface() FAILED");

	// use most up-to-date version of the interface
	/*
    ddrval = DDr->QueryInterface(IID_IDirectDraw4, (LPVOID *) &myDD4);
    if (ddrval != DD_OK)
        return InitFail(ddrval, "QueryInterface FAILED");

	SAFE_RELEASE (DDr);
	*/

	return true;	
}


// -------------+
// FinishUpdate : copy the virtual (off-screen) buffer to the Secondary surface,
//              : then switch Primary and Secondary surfaces.
//              : For windowed mode : copy the virtual buffer to the window Bitmap.
// -------------+
void CVideo::FinishUpdate()
{
    // Not FullScreen = Windowed Mode
	// (could run fullscreen without DirectDraw.. not really useful)
	if ( !bFullScreen )
    {
        // paranoia
		if ( !hDCMain || !bmiMain || !pVirtual )
			return;
            //InitFail("CVideo::FinishUpdate() - Windowed mode not set");

        // Copy screen to window
		if ( iZoomScreen==0 )
		{
			if ( !SetDIBitsToDevice (hDCMain,
                           0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           0, 0,
						   0, SCREEN_HEIGHT,
                           pVirtual, bmiMain, DIB_RGB_COLORS) )
				InitFail("CVideo::FinishUpdate() - SetDIBitsToDevice() FAILED");
		}
		else if (iZoomScreen==1)
		{
			if ( StretchDIBits( hDCMain,
								 0, 0, SCREEN_WIDTH<<1, SCREEN_HEIGHT<<1,
								 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
								 pVirtual, bmiMain, DIB_RGB_COLORS, SRCCOPY ) == GDI_ERROR )
				InitFail("CVideo::FinishUpdate() - StretchDIBits() FAILED");
		}
    }
	else if ( DDr )
	{
		PIXEL* pScr = LockScreen();

		// copy virtual screen to real screen

		//CopyBlock (VID->pVirtual, 0, 0, 320, 200, SCREEN_WIDTH, ScreenPtr, 0, 0, ScreenPitch);
        
		//faB: TODO: use directX blit here!!? a blit might use hardware with access
        //     to main memory on recent hardware, and software blit of directX may be
        //  optimized for p2 or mmx??
        BlitLinearScreen (pVirtual, pScr,
					      Width, Height,
						  Width, DDrModulo);

		UnlockScreen();

		SwapScreen();
	}

}


// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// srcPitch, destPitch : width of source and destination bitmaps
// --------------------------------------------------------------------------
void CVideo::BlitLinearScreen(PIXEL *srcptr, 
							  PIXEL *destptr, 
							  int	width, 
							  int	height, 
							  int	srcrowpixels, 
							  int	destrowpixels)
{
    if (srcrowpixels==destrowpixels)
    {
        CopyMemory (destptr, srcptr, srcrowpixels * sizeof(PIXEL) * height);
    }
    else
    {
		int srcrowbytes = srcrowpixels * sizeof(PIXEL);
		int	destrowbytes = destrowpixels * sizeof(PIXEL);
        while (height--)
        {
            CopyMemory (destptr, srcptr, width * sizeof(PIXEL));
            destptr += destrowbytes;
            srcptr += srcrowbytes;
        }
    }
}



//
// Clear the surface to color
/*
void CVideo::ClearSurface (IDirectDrawSurface* surface, int color)
{
    DDBLTFX		ddbltfx;

    // Use the blter to do a color fill to clear the back buffer
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = color;
    surface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

}*/


// -------------+
// SwapScreen	: switch Primary and Secondary surfaces
// -------------+
BOOL CVideo::SwapScreen()
{
	HRESULT hr;
	//RECT	rect;

	// In full-screen exclusive mode, do a hardware flip.
	hr = DDrPrimary->Flip(NULL, DDFLIP_WAIT);
	if (hr == DD_OK)
		return TRUE;
	
	// If the surface was lost, restore it.
	if (hr == DDERR_SURFACELOST)
		DDrPrimary->Restore();
	
	// The restore worked, so try the flip again.
	hr = DDrPrimary->Flip(NULL, DDFLIP_WAIT);
	if (hr == DD_OK)
		return TRUE;

	/* HMMM ... maybe use a off-videomemory surface and blit it to secondary surface ?
	{
		rect.left = windowPosX;
		rect.top = windowPosY;
		rect.right = windowPosX + ScreenWidth - 1;
		rect.bottom = windowPosY + ScreenHeight - 1;

		// Copy the back buffer to front.
		hr = DDrPrimary->Blt(&rect, DDrSecondary, 0, DDBLT_WAIT, 0);
		if (hr == DD_OK)
			return true;		
		
		// If the surfaces were lost, restore them.
		if (DDrPrimary->IsLost() == DDERR_SURFACELOST)			
			DDrPrimary->Restore();
		
		if (DDrSecondary->IsLost() == DDERR_SURFACELOST)
			DDrSecondary->Restore();
		
		// Retry the copy.
		hr = DDrPrimary->Blt(&rect, DDrSecondary, 0, DDBLT_WAIT, 0);
		if (hr == DD_OK)
			return true;		
	}
	*/
	return FALSE;
}


//
// Print a text to the surface
//
void CVideo::TextPrint (int x, int y, char* message)
{
	HRESULT hr;
	HDC		hdc = NULL;

	// Get the device context handle.
	hr = DDrSecondary->GetDC(&hdc);
	if (hr != DD_OK)
		return;

	// Write the message.
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	TextOut(hdc, x, y, message, strlen(message));

	// Release the device context.
	hr = DDrSecondary->ReleaseDC(hdc);
}


// -------------+
// LockScreen	: lock the secondary surface, return a PIXEL* you can use
//				: to write directly to the surface, also sets DDrModulo
//				; to the screen pitch (it may be larger than Width).
// -------------+
PIXEL* CVideo::LockScreen()
{
	DDSURFACEDESC ddsd;
	HRESULT		ddrval;
	PIXEL*		screenPtr;

	ZeroMemory( &ddsd, sizeof( ddsd ));
	ddsd.dwSize = sizeof( ddsd );
	
	// attempt to Lock the surface
	ddrval = DDrSecondary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	
	// Always, always check for errors with DirectX!
	if( ddrval == DD_OK )
	{
		screenPtr = (PIXEL*) ddsd.lpSurface;
		DDrModulo = ddsd.lPitch;
	}
	else
	{
		screenPtr = NULL;
		InitFail ("CVideo::LockScreen() FAILED");
	}

	return screenPtr;
}


// -------------+
// UnlockScreen : Release the lock on the secondary surface when you have finished with it
// -------------+
void CVideo::UnlockScreen(void)
{
	if (DD_OK != DDrSecondary->Unlock(NULL))
		InitFail ("CVideo::UnlockScreen() FAILED");
}


// -------------+
// CreatePalette: create a DirectDraw palette object from the PALETTEENTRY array
// -------------+
void CVideo::CreatePalette( PALETTEENTRY* colorTable )
{
	HRESULT	 ddrval;
	ddrval = DDr->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256, colorTable, &DDrPalette, NULL);
	if (ddrval != DD_OK)
		InitFail ("CVideo::CreatePalette() FAILED");
};



// -------------+
// SetPalette	: set the current palette for the screen
//				: works for fullscreen as well as windowed mode 
// -------------+
void CVideo::SetPalette (PALETTEENTRY* pal)
{
	int	i;

    if( !bFullScreen )
    {
        // set palette in RGBQUAD format, NOT THE SAME ORDER as PALETTEENTRY, grmpf!
        RGBQUAD*    pColors;
        pColors = (RGBQUAD*) ((char*)bmiMain + bmiMain->bmiHeader.biSize);
        ZeroMemory (pColors, sizeof(RGBQUAD)*256);
        for (i=0; i<256; i++, pColors++)
        {
            pColors->rgbRed = pal->peRed;
            pColors->rgbGreen = pal->peGreen;
            pColors->rgbBlue = pal->peBlue;
			pal++;
        }
		// the palette of RGB entries is then used for color conversion
		// in SetDIBitsToDevice()
		return;
    }
	else if( DDr )
	{
		// create palette first time
		if( DDrPalette==NULL )
			CreatePalette( pal );
		else
			DDrPalette->SetEntries( 0, 0, 256, pal );
		
		// setting the same palette to the same surface again does not increase
		// the reference count
		DDrPrimary->SetPalette( DDrPalette );
	}
}


//
// Wait for vsync, gross
//
void CVideo::WaitVbl (void)
{
	if ( !bFullScreen )
	{
		
		return;
	}
	else if (DDr)
	{
		DDr->WaitForVerticalBlank (DDWAITVB_BLOCKBEGIN, NULL);
	}
}


// -------------+
// ToggleFullscreen
//				: switch to fullscreen DirectDraw mode, or back to windowed mode
// -------------+
BOOL CVideo::ToggleFullscreen()
{
	BOOL	result;
	
	if ( bFullScreen )
		ClearFullScreenMode();
	else
		ClearWindowedMode();

    bFullScreen = !bFullScreen;

	if ( !bFullScreen )
		result = SetWindowedDisplay( hWndMain );
	else
		result = SetFullScreenDisplay( hWndMain, Width, Height, SCREEN_BPP );

	return result;
}



//
// this function sets a palette entry with the sent color
/* UNTESTED
int SetPaletteEntry (int index, int red, int green, int blue)
{
	PALETTEENTRY color;		// used to build up color
	
	// set RGB value in structure
	color.peRed = (BYTE)red;
	color.peGreen = (BYTE)green;
	color.peBlue = (BYTE)blue;
	color.peFlags = PC_NOCOLLAPSE;
	
	// set the color palette entry
	lpddpal->SetEntries(0,index,1,&color);
	
	// make copy in shadow palette
	memcpy(&color_palette[index],
		&color,
		sizeof(PALETTEENTRY));
	
	// return success
	return(1);
	
} // end Set_Pal_Entry*/

/*--------------------------------------------------------------------------------
  get a primary surface

DDSURFACEDESC ddsd; 
ddsd.dwSize = sizeof(ddsd); 
 
// Tell DirectDraw which members are valid. 
ddsd.dwFlags = DDSD_CAPS; 
 
// Request a primary surface. 
ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; 

  After creating the primary surface, you can retrieve information about its
  dimensions and pixel format by calling it
  IDirectDrawSurface3::GetSurfaceDesc
  method.

--------------------------------------------------------------------------------
The following example shows how to prepare for creating a simple off-screen surface.

DDSURFACEDESC ddsd; 
ddsd.dwSize = sizeof(ddsd); 
 
// Tell DirectDraw which members are valid. 
ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH; 
 
// Request a simple off-screen surface, sized 
// 100 by 100 pixels. 
//
// (This assumes that the offscreen surface we are about
// to create will match the pixel format of the 
// primary surface.)
ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN; 
ddsd.dwHeight = 100; 
ddsd.dwWidth = 100; 

--------------------------------------------------------------------------------
    The following code fragment shows how to prepare the DDSURFACEDESC structure
	members in order to create an 8-bit palettized surface (assuming that the
	current display mode is something other than 8-bits per pixel).

ZeroMemory(&ddsd, sizeof(ddsd));
ddsd.dwSize  = sizeof(ddsd);
ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
ddsd.dwHeight = 100;
ddsd.dwWidth  = 100;
ddsd.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;

// Set the bit depth for an 8-bit surface, but DO NOT 
// specify any RGB mask values. The masks must be zero

// for a palettized surface.
ddsd.ddpfPixelFormat.dwRGBBitCount = 8;

--------------------------------------------------------------------------------
The following example shows how to prepare for creating a primary surface flipping chain.

DDSURFACEDESC ddsd; 
ddsd.dwSize = sizeof(ddsd); 
 
// Tell DirectDraw which members are valid. 
ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT; 
 
// Request a primary surface with a single 
// back buffer 
ddsd.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE; 
ddsd.dwBackBufferCount = 1; 

The previous example constructs a double-buffered flipping environment
a single call to the IDirectDrawSurface3::Flip method exchanges the surface memory
of the primary surface and the back buffer.

*/
//
// CreateNewSurface
/*
IDirectDrawSurface* CVideo::CreateNewSurface(int dwWidth,
									 int dwHeight,
								     int dwSurfaceCaps)
{
    DDSURFACEDESC       ddsd;
    HRESULT             hr;
//    DDCOLORKEY          ddck;
    LPDIRECTDRAWSURFACE psurf;
    
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;

    ddsd.ddsCaps.dwCaps = dwSurfaceCaps;

    ddsd.dwHeight = dwHeight;
    ddsd.dwWidth = dwWidth;

    hr = DDr->CreateSurface (&ddsd, &psurf, NULL);

    if( hr == DD_OK )
    {
        psurf->Restore();

        //hr = DDrPrimary->GetColorKey(DDCKEY_SRCBLT, &ddck);
        //psurf->SetColorKey(DDCKEY_SRCBLT, &ddck);
    }
    else
        psurf = NULL;

    return psurf;
}*/
