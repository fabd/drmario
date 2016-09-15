// Video.h: interface for the CVideo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEO_H__C7165A22_52B7_11D3_B616_00201834E35C__INCLUDED_)
#define AFX_VIDEO_H__C7165A22_52B7_11D3_B616_00201834E35C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ddraw.h>


// ---------------
// Video constants
// ---------------
typedef unsigned char	PIXEL;


// -----------
// Video Class
// -----------
class CVideo  
{
public:
	CVideo();
	virtual ~CVideo();

public:
	int Width;
	int	Height;
	BOOL Startup( HWND hWnd, int PixelWidth, int iPixelHeight, BOOL bStartFullScreen );
	BOOL ToggleFullscreen();
	void SetWindowZoom( int iZoom );
	void FinishUpdate();

	void TextPrint (int x, int y, char* message);
	void SetPalette (PALETTEENTRY* pal);
	void WaitVbl (void);

	BOOL	bFullScreen;			// wether app is running fullscreen or windowed
	inline PIXEL* pScreen() { return pVirtual; };

	//int					windowPosX;				// current position in windowed mode
	//int					windowPosY;

private:
	BOOL SetWindowedDisplay( HWND hAppWindow );
	void ClearWindowedMode();
	BOOL SetFullScreenDisplay( HWND hWnd, int Width, int Height, int Bpp );
	void ClearFullScreenMode();
	void BlitLinearScreen( PIXEL* srcptr, PIXEL* destptr, int width, int height, int srcrowbytes, int destrowbytes );
	int  iZoomScreen;			// 0 normal, 1 double size (value is used with << operator)
	PIXEL* LockScreen();
	void UnlockScreen(void);

	//void ClearSurface (IDirectDrawSurface* surface, int color);
	/*IDirectDrawSurface* CreateNewSurface( int dwWidth,
										  int dwHeight,
										  int dwSurfaceCaps );*/
	void CreatePalette (PALETTEENTRY* colorTable);

	BOOL SwapScreen();
	HWND	hWndMain;
	
	PIXEL* pVirtual;							// virtual (off-videomemory) screen buffer

	// CODE
	

	// DATA

	IDirectDraw*		DDr;
	IDirectDrawSurface*	DDrPrimary;				// DirectDraw primary surface
	IDirectDrawSurface*	DDrSecondary;			// DirectDraw back surface
	int					DDrModulo;				// offset between 2 lines of DDrSecondary surface
	IDirectDrawPalette*	DDrPalette;				// The primary surface palette
	//IDirectDrawClipper*	windclip;			// clipper for windowed mode

	BITMAPINFO*			bmiMain;
	HDC					hDCMain;
};

#endif // !defined(AFX_VIDEO_H__C7165A22_52B7_11D3_B616_00201834E35C__INCLUDED_)
