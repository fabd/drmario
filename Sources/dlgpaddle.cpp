// dlgpaddle.cpp : implementation of the 'setup player controls' dialog

#include "dlgpaddle.h"
#include "resource.h"

#include "drmario.h"
#include "drm_game.h"

//#include "strings.h"

// local copy of ptr to player struct whose keys are being setup, from WM_INITDIALOG parm
static struct PlayerVars*	pl;
static	int	txtFocusID = 0;

// order in struct : UBYTE up,down,left,right,buttona,buttonb

// convert Virtual Key code to string description of the key
static LPSTR VKT( UBYTE vkey )
{
static char keyname[2];

	switch( vkey ){
	case VK_BACK:	return "BACK";	
	case VK_TAB:	return "TAB";	
	case VK_CLEAR:	return "CLEAR";
	case VK_RETURN:	return "RETURN";	
	case VK_SHIFT:	return "SHIFT";
	case VK_CONTROL:return "CONTROL";
    case VK_SPACE:	return "SPACE";
    case VK_PRIOR:	return "PRIOR";
    case VK_NEXT:	return "NEXT";
    case VK_END:	return "END";
    case VK_HOME   :return "HOME";
	case VK_UP:		return "UP";
	case VK_DOWN:	return "DOWN";
	case VK_LEFT:	return "LEFT";
	case VK_RIGHT:	return "RIGHT";	
    case VK_SELECT :return "SELECT";
    case VK_PRINT  :return "PRINT";
    case VK_EXECUTE:return "EXECUTE";
    case VK_SNAPSHOT:return "SNAPSHOT";
    case VK_INSERT :return "INSERT";
    case VK_DELETE :return "DELETE";
    case VK_HELP   :return "HELP";

    case VK_LWIN   :return "LWIN";
    case VK_RWIN   :return "RWIN";
    case VK_APPS   :return "APPS";

    case VK_NUMPAD0:return "NUMPAD0";
    case VK_NUMPAD1:return "NUMPAD1";
    case VK_NUMPAD2:return "NUMPAD2";        //0x62
    case VK_NUMPAD3:return "NUMPAD3";        //0x63
    case VK_NUMPAD4:return "NUMPAD4";        //0x64
    case VK_NUMPAD5:return "NUMPAD5";        //0x65
    case VK_NUMPAD6:return "NUMPAD6";        //0x66
    case VK_NUMPAD7:return "NUMPAD7";        //0x67
    case VK_NUMPAD8:return "NUMPAD8";        //0x68
    case VK_NUMPAD9  :return "NUMPAD9";      //0x69
    case VK_MULTIPLY :return "MULTIPLY";     //0x6A
    case VK_ADD      :return "ADD";			 //0x6B
    case VK_SEPARATOR:return "SEPARATOR";    //0x6C
    case VK_SUBTRACT :return "SUBTRACT";     //0x6D
    case VK_DECIMAL  :return "DECIMAL";      //0x6E
    case VK_DIVIDE   :return "DIVIDE";       //0x6F
    case VK_F1     :return "F1";        //0x70
    case VK_F2     :return "F2";        //0x71
    case VK_F3     :return "F3";        //0x72
    case VK_F4     :return "F4";        //0x73
    case VK_F5     :return "F5";        //0x74
    case VK_F6     :return "F6";        //0x75
    case VK_F7     :return "F7";        //0x76
    case VK_F8     :return "F8";        //0x77
    case VK_F9     :return "F9";        //0x78
    case VK_F10    :return "F10";       //0x79
    case VK_F11    :return "F11";       //0x7A
    case VK_F12    :return "F12";       //0x7B
    case VK_NUMLOCK:return "NUMLOCK";        //0x90
    case VK_SCROLL :return "SCROLL";         //0x91

	// VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
	// Used only as parameters to GetAsyncKeyState() and GetKeyState().
	// No other API or message will distinguish left and right keys in this way.
    case VK_LSHIFT   :return "LSHIFT";      //0xA0
    case VK_RSHIFT   :return "RSHIFT";      //0xA1
    case VK_LCONTROL :return "LCONTROL";    //0xA2
    case VK_RCONTROL :return "RCONTROL";    //0xA3
    case VK_LMENU    :return "LMENU";		//0xA4
    case VK_RMENU    :return "RMENU";		//0xA5

	/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (//0x30 - //0x39) */
	/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (//0x41 - //0x5A) */

	default:
		keyname[0] = vkey;
		keyname[1] = '\0';
		return keyname;
	}
}

static void SetText (HWND hDlg, int nID, char* szCaption)
{
	HWND	hCtl;
	hCtl = GetDlgItem( hDlg, nID );
	if( hCtl ) {
		SetWindowText( hCtl, szCaption );
	}
}

static void ShowPlayerKeys( HWND hDlg, struct PlayerVars* plv )
{
	SetText( hDlg, TXT_UP, VKT(plv->P_Up));
	SetText( hDlg, TXT_DOWN, VKT(plv->P_Down));
	SetText( hDlg, TXT_LEFT, VKT(plv->P_Left));
	SetText( hDlg, TXT_RIGHT, VKT(plv->P_Right));
	SetText( hDlg, TXT_BUTTONA, VKT(plv->P_ButtonA));
	SetText( hDlg, TXT_BUTTONB, VKT(plv->P_ButtonB));
}



// -------------+
// PaddleDlgProc: the setup controls Dialog window handler
// -------------+
LRESULT WINAPI PaddleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#define NUMTXTS	6
	int	txtID[6] = { TXT_LEFT,TXT_RIGHT,TXT_UP,TXT_DOWN,TXT_BUTTONA,TXT_BUTTONB };
	int	i;
	HWND hwndOwner;
	RECT rc, rcDlg, rcOwner; 
	UBYTE vkey = LOBYTE(wParam);

	switch( msg )
	{
	
	case WM_INITDIALOG:
		// lParam is player vars struct
		pl = (struct PlayerVars*) lParam;

		// CENTER ON PARENT WINDOW
		// -------------------------------------------------
        if( (hwndOwner = GetParent(hDlg)) == NULL )
            hwndOwner = GetDesktopWindow();
        GetWindowRect( hwndOwner, &rcOwner );
        GetWindowRect( hDlg, &rcDlg );
		CopyRect( &rc, &rcOwner );
        // Offset the owner and dialog box rectangles so that 
        // right and bottom values represent the width and 
        // height, and then offset the owner again to discard 
        // space taken up by the dialog box.  
        OffsetRect( &rcDlg, -rcDlg.left, -rcDlg.top );
        OffsetRect( &rc, -rc.left, -rc.top ); 
        OffsetRect( &rc, -rcDlg.right, -rcDlg.bottom );  
        // The new position is the sum of half the remaining 
        // space and the owner's original position.  
        SetWindowPos( hDlg, HWND_TOP,
						rcOwner.left + (rc.right / 2), 
						rcOwner.top + (rc.bottom / 2), 
						0, 0,     // ignores size arguments
						SWP_NOSIZE ); 		
		// -------------------------------------------------
		
		
		ShowPlayerKeys( hDlg, pl );
		return TRUE;

	case WM_COMMAND:
		//  low-order word of wParam  = control identifier
		// high-order word contrains the notification code
		// lParam contains the control window handle
		if( HIWORD(wParam)==EN_KILLFOCUS ) {
			txtFocusID = 0;
			SetWindowText( hDlg, "killfocus");
			break;
		}
		
		switch( LOWORD(wParam) )
		{
		case IDOK:
		case IDCANCEL:	//sysmenu close
			EndDialog( hDlg, TRUE );
			return TRUE;
		case CMD_DEFAULT:
			SetDefaultKeys( pl );
			ShowPlayerKeys( hDlg, pl );
			return TRUE;
		default:
			if( HIWORD(wParam)==EN_SETFOCUS )
			{
				for(i=0; i<NUMTXTS; i++)
					if( LOWORD(wParam)==txtID[i] ) {
						txtFocusID = txtID[i];
						//SetWindowText( hDlg, va("txtfocusid %d",txtFocusID) );
						break;
					}
				/*if(i<NUMTXTS)
					SetWindowText( hDlg, "setfocus txt_xxx" );
				else
					SetWindowText( hDlg, "setfocus no txt" );*/
			}
			break;
		}
		break;

	case WM_CHAR:
		SetWindowText( hDlg, "char!!!" );
		return TRUE;
	case WM_KEYDOWN:

        // (int)wParam  = virtual key code
		// lParam = key data
		SetWindowText( hDlg, "touche!!!" );
		return TRUE;

		switch( txtFocusID )
		{
		case TXT_UP:		pl->P_Up = vkey;	break;
		case TXT_DOWN:		pl->P_Down = vkey;	break;
		case TXT_LEFT:		pl->P_Left = vkey;	break;
		case TXT_RIGHT:		pl->P_Right = vkey;	break;
		case TXT_BUTTONA:	pl->P_ButtonA = vkey;	break;
		case TXT_BUTTONB:	pl->P_ButtonB = vkey;	break;
		default:	vkey=0;
				SetWindowText( hDlg, "key but no txt focus" );
			break;
		}
		if( vkey ) {
				SetWindowText( hDlg, "key on txt focus" );
			ShowPlayerKeys( hDlg, pl );
		}
		return 0; //break;
	}
	return FALSE;
}
