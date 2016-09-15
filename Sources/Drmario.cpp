
// ========================
// DrMario
// total (99.5%) PC conversion
//  (C)1995-1999 faBBe DeZiGn
// original idea & concept
//  (C) Nintendo
// ========================

#include <windows.h>		// include the standard windows stuff
#include <windowsx.h>		// include the 32 bit stuff
//#include <mmsystem.h>

#include "win_dbg.h"		// exception handler, trap bugs!!

//#include <afxwin.h>

#include "CArgv.h"
#include "CLogFile.h"
#include "CVideo.h"
#include "fabgrafx.h"

//#include <stdlib.h>
//#include <stdio.h>
#include <stdarg.h>

#include "resource.h"

#include "errors.h"
#include "drm_game.h"

#include "dlgpaddle.h"


// -------
// private
// -------

static	HINSTANCE	myInstance;

static	int			myargc;
static	char**		myargv;


// ---------------
// globals go here
// ---------------

	BOOL    bAppHasFocus = FALSE;			// the app is active (false when Alt-tab)

	HWND	myWindow;				// main app window
	HCURSOR	windowCursor;			// main window cursor

	CVideo*	VID = NULL;				// the display object (window surface or fullscreen directdraw surface)

	CLogFile* LOG = NULL;

	ULONG	GameTics;


// the timer we use to flip the pages
//#define TIMER_ID            1
//#define TIMER_RATE          500



// ----------
// Error Exit
// ----------
BOOL InitFail(LPCTSTR lpFmt, ...)
{
    char    str[1999];
    va_list arglist;

    va_start (arglist, lpFmt);
    wvsprintf (str, lpFmt, arglist);
    va_end   (arglist);

	LOG->Log("InitFail() Reason: %s", str);

	// - shutdown -
	Game_Shutdown();

	SAFE_DELETE( VID )
	
	// - shutdown -
    
    DestroyWindow(myWindow);
    
	MessageBox(NULL, str, "Error", MB_OK|MB_ICONSTOP);

	SAFE_DELETE( LOG )	//delete this at the very end!!
    exit (-1);
}


// -------------+
// AboutDlgProc	: the about dialog procedure
// -------------+
LRESULT WINAPI AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch( wParam )
		{
		case IDOK:
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return TRUE;
		}
		break;
	}
	return FALSE;
}


// -----------------------------------------------------------------------------
// WindowProc : the message handler for my window
// -----------------------------------------------------------------------------
LRESULT WINAPI WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //static HINSTANCE hInstance;
	int		i;
	HMENU	hMenu;
    
	switch (message)
    {
		case WM_CREATE:
			// Initialise window object ?
			// do windows inits here
			//	hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
			return false;

		case WM_PAINT:
			// app came to foreground
			//if (bAppHasFocus)
			// redraw screen here
			//else
			if (!bAppHasFocus && !VID->bFullScreen)
			// app becomes inactive (if windowed 
			{
				// Paint "Game Paused" in the middle of the screen
				PAINTSTRUCT ps;
				RECT		rect;
				HDC hdc = BeginPaint (hWnd, &ps);
				GetClientRect (hWnd, &rect);
				DrawText (hdc, "Game Paused", -1, &rect,
						DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				EndPaint (hWnd, &ps);
				return false;
			}
			break;
        
		case WM_ACTIVATEAPP:		
			// Handle task switching
			bAppHasFocus = wParam;
			InvalidateRect (hWnd, NULL, TRUE);
			break;

		case WM_COMMAND:
			hMenu = GetMenu( hWnd );
			switch (LOWORD(wParam))
			{
				// =========
				// MENU : main
				// =========
				case IDM_ABOUT:
					DialogBox( myInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, (DLGPROC) AboutDlgProc );
					return 0;
				case IDM_QUIT:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;
				// =========
				// MENU : config
				// =========
				case IDM_CONFIGPL1:
				case IDM_CONFIGPL2:
					PauseGame();
					DialogBoxParam( myInstance, MAKEINTRESOURCE(IDD_PADDLE), hWnd,
									(DLGPROC)PaddleDlgProc,
									GetPlayerVars( (LOWORD(wParam)==IDM_CONFIGPL1) ? 1 : 2) );
					UnpauseGame();
					return 0;
				case IDM_SOUNDS:
					i = (GetMenuState( hMenu, IDM_SOUNDS, MF_BYCOMMAND ) & MF_CHECKED) ? 0 : 1;
					SetSoundEnabled( i );
					CheckMenuItem( hMenu, IDM_SOUNDS, i ? MF_CHECKED : MF_UNCHECKED );
					break;
				// =========
				// MENU : display
				// =========
				case IDM_DISPLAY_TOGGLEFULLSCREEN:
					goto toggleviewfullscreen;
				case IDM_DISPLAY_WINDOW_DOUBLE:
					i = (GetMenuState( hMenu, IDM_DISPLAY_WINDOW_DOUBLE, MF_BYCOMMAND ) & MF_CHECKED) ? 0 : 1;
					VID->SetWindowZoom( i );
					CheckMenuItem( hMenu, IDM_DISPLAY_WINDOW_DOUBLE, i ? MF_CHECKED : MF_UNCHECKED );
					break;
			}
			break;

		// handle key commands
		case WM_KEYDOWN:

            switch (wParam)
            {
				//case VK_ESCAPE:
				case VK_F12:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0;
            }
            break;

		// If the app is in windowed mode, the coordinates the client area
        // must be calculated since DDraw does not do this.
        case WM_MOVE:
            
			if (VID->bFullScreen)
            {
				// hmm does this happen ?
                SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                return 0;
            }

        // Hide the cursor
		
		case WM_SETCURSOR:
			if (VID->bFullScreen)
	            SetCursor(NULL);
			else
				SetCursor(windowCursor);
            return TRUE;

        // This is where switching windowed/fullscreen is handled. DirectDraw
        // objects must be destroyed, recreated, and artwork reloaded.
        case WM_SYSKEYUP:
			if (wParam == VK_RETURN)
            {
                //static int  lastPosX, lastPosY;		// last window position
toggleviewfullscreen:

                if (!VID->ToggleFullscreen())
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
				
				//will set the current used palette or create it
				VID->SetPalette( main_pal );

				return 0;
            }
            else
                break;
		
		// Closing the window
		// - exit program, free stuff
		case WM_CLOSE:
		case WM_DESTROY:
		    PostQuitMessage(0);
			break;


			
		//case WM_KEYUP:


		// left mouse button
		
		/*
		case WM_LBUTTONDOWN:
			//x = LOWORD (lParam);	// X-coordinate of mouse click
			//y = HIWORD (lParam);	// Y-coordinate of mouse click
			if (bAppHasFocus)
			{
				UpdateFrame();
				ScreenFlip();
			}
			return 0;
		*/

		//the most significant bit returned tells of key is down _NOW_
		// the least significant bit tells us if key has ever been down
		// since last time we looked.
		// if (!GetASyncKeyState(VK_LEFT)) key_table[KEY_LEFT] = 0;
		//
		// contrary to WM_KEYUP/DOWN events, GetAsyncKeyState method still
		//  handles key when app isnot the active one.. watch out


        // Called by our timer
		// Update and flip surfaces
		/*case WM_TIMER:
            
            if (bAppHasFocus && (TIMER_ID == wParam))
            {
                UpdateFrame(hWnd);
                ScreenFlip ();
            }
            break;
		*/
    }
	return DefWindowProc(hWnd, message, wParam, lParam);
}


// ----------------------
// Create app main window
// ----------------------
HWND	OpenWindow (HINSTANCE hInstance, char* appTitle, char* windowTitle, int nCmdShow)
{
    HWND       hWnd;
    WNDCLASS   wc;

	// Set up and register window class
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW /*| CS_DBLCLKS*/;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    //wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRGICON));
	windowCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
    wc.hCursor = windowCursor; /*LoadCursor (NULL, IDC_ARROW);*/
    wc.hbrBackground = (HBRUSH )GetStockObject(NULL_BRUSH);	//(HBRUSH)( COLOR_WINDOW+1 );
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = appTitle;
    RegisterClass(&wc);

    // Create a window
	// CreateWindowEx - seems to create just the interior, not the borders
	
	hWnd = CreateWindowEx(WS_EX_TOPMOST,		//ExStyle
 						  appTitle,				//Classname
                          windowTitle,			//Windowname
                          WS_CAPTION|WS_POPUP,	//dwStyle	//WS_VISIBLE|WS_POPUP for fullScreen
                          0,
                          0,
                          320,	//GetSystemMetrics(SM_CXSCREEN),
                          200,	//GetSystemMetrics(SM_CYSCREEN),
                          NULL,					//hWnd Parent
                          NULL,					//hMenu Menu
                          hInstance,
                          NULL);
	

	/*CreateWindow - create window with borders
	hWnd = CreateWindow(appTitle,
						windowTitle,
						WS_OVERLAPPEDWINDOW,	//WS_EX_TOPMOST for fullScreen
						CW_USEDEFAULT, CW_USEDEFAULT,	// top left
						CW_USEDEFAULT, CW_USEDEFAULT,	// size
						HWND_DESKTOP,					// parent
						0,								// menu
						hInstance,
						NULL );
	*/
    
	if (hWnd)
	{
		// show the Window
		SetFocus(hWnd);
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}
	
	return hWnd;
}


// -----------------------------------------------------------------------------
// HandledWinMain : called by exception handler
// -----------------------------------------------------------------------------
int WINAPI HandledWinMain(HINSTANCE hInstance,
                          HINSTANCE hPrevInstance,
                          LPSTR     lpCmdLine,
                          int       nCmdShow)
{
	CArgv*	ARG;
    MSG     msg;

	LOG = new CLogFile( "drmario.log" );	//pass NULL to disable logging without removing calls to LOG
	
    // parse command line
	ARG = new CArgv( lpCmdLine );

	//if( hPrevInstance )
	//{
	//	MessageBox( NULL, "DrMario is already running!", "Message", MB_OK );
	//	return FALSE;
	//}

	myInstance = hInstance;

    // initialize display BEFORE OpenWindow !!! (because window proc uses VID)
	VID = new CVideo;

	// open main app window
	if (!(myWindow = OpenWindow(hInstance, "DrMarioWindow", "DrMario", nCmdShow)))
		return FALSE;
    
	if (!VID->Startup( myWindow, SCREEN_WIDTH, SCREEN_HEIGHT, FALSE ))
		return FALSE;

	// initialize game
	if (!Game_Init( !ARG->CheckParm("-nosound") ) )
		goto quit;	

	// we don't need it anymore
	SAFE_DELETE( ARG );
    
	//
	// da main Windoze loop
	//
	while (1)
    {
        if (PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessage( &msg, NULL, 0, 0))
            { 
                break;
            }
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
        else if (bAppHasFocus)
        {
			// call main logic module
			// if it returns false, it means the user wants to quit the program
			if (!Game_Main())
				break;
        }
        else
        {
            WaitMessage();
        }
    }


quit:
	// shutdown game
	Game_Shutdown();

	SAFE_DELETE( VID )

	SAFE_DELETE( LOG )

	// back to Windoze
	return msg.wParam;
}


// -----------------------------------------------------------------------------
// Exception handler calls WinMain for catching exceptions
// -----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR        lpCmdLine,
                    int          nCmdShow)
{
    int Result = -1;
    __try
    {
        Result = HandledWinMain (hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    }

    __except ( RecordExceptionInfo( GetExceptionInformation(), "main thread", lpCmdLine) )
    {
        //Do nothing here.
    }

    /*__finally
    {
    if (logstream != INVALID_HANDLE_VALUE)
        CloseHandle (logstream);
	}*/

    return Result;
}
