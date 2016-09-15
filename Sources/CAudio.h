// CAudio.h: interface for the CAudio class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAUDIO_H__6BDB34E0_55A1_11D3_B616_00201834E35C__INCLUDED_)
#define AFX_CAUDIO_H__6BDB34E0_55A1_11D3_B616_00201834E35C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dsound.h>


typedef int		STACKSOUNDID;	// sound handle returned by StartSound()

#define SEP_RANGE       256     // CAudio stereo separation range is 0-255 (128 is centre)
#define VOL_RANGE		32		// CAudio volume range
#define MAX_VOLUME		(VOL_RANGE-1)


class CAudio  
{
public:
	void StopAllSounds();
	LPDIRECTSOUNDBUFFER LoadWave( HANDLE hFile );
	void SetSfxVolume( int volume );
	void UpdateSoundPanning( LPDIRECTSOUNDBUFFER lpSnd, int sep );
	void UpdateSoundVolume( LPDIRECTSOUNDBUFFER lpSnd, int volume );
	void StopSound( STACKSOUNDID id );
	void UpdateSoundParams( STACKSOUNDID handle, int vol, int sep, int pitch );
	inline BOOL AudioStarted() { return bAudioStarted; };
	STACKSOUNDID StartSound( LPDIRECTSOUNDBUFFER dsBuf, int iVol, int iSep, int iPitch, int iPriority );
	CAudio( HWND hWndMain );
	virtual ~CAudio();

private:
	int iSfxVolume;
	BOOL SoundIsPlaying( STACKSOUNDID id );
	BOOL bAudioStarted;
	LPDIRECTSOUND DiSnd;
	LPDIRECTSOUNDBUFFER DiSndPrimary;
};

#endif // !defined(AFX_CAUDIO_H__6BDB34E0_55A1_11D3_B616_00201834E35C__INCLUDED_)
