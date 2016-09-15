# vim: syntax=cpp ts=4 sw=4 sts=4 sr noet

// CAudio.cpp: implementation of the CAudio class.
//
//////////////////////////////////////////////////////////////////////

//#include <mmsystem.h>
#include "CAudio.h"
#include "errors.h"

#define DEBUGSOUND	1


// Stack sounds means sounds put on top of each other, since DirectSound can't play
// the same sound buffer at different locations at the same time, we need to dupli-
// cate existing buffers to play multiple instances of the same sound in the same
// time frame. A duplicate sound is freed when it is no more used. The priority that
// comes from the s_sound engine, is kept so that the lowest priority sounds are
// stopped to make place for the new sound, unless the new sound has a lower priority
// than all playing sounds, in which case the sound is not started.
#define MAXSTACKSOUNDS      32          // this is the absolute number of sounds that
                                        // can play simultaneously
typedef struct {
    LPDIRECTSOUNDBUFFER lpSndBuf;
    int                 priority;
    boolean             duplicate;
} StackSound_t;
StackSound_t    StackSounds[MAXSTACKSOUNDS];


#define DSBPAN_RANGE    (DSBPAN_RIGHT-(DSBPAN_LEFT))


// -------------+
// CAudio		: Initialize DirectSound, make CAudio object ready to play
// -------------+
CAudio::CAudio( HWND hWndMain )
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER lpDsb;
    DSBUFFERDESC        dsbdesc;
    WAVEFORMATEX        wfm;
    int                 cooplevel;
    int                 frequency;

	DiSnd = NULL;
	bAudioStarted = false;

    //if (nosound)
    //    return;

    // Secure and configure sound device first.
    LOG->Log("I_StartupSound: \n");
    
    // Create DirectSound, use the default sound device
    hr = DirectSoundCreate( NULL, &DiSnd, NULL);
    if ( FAILED( hr ) ) {
        InitFail (" DirectSoundCreate FAILED\n"
                     " there is no sound device or the sound device is under\n"
                     " the control of another application\n" );
        return;
    }

    // register exit code, now that we have at least DirectSound to close
    // I_AddExitFunc (I_ShutdownSound);

    // Set cooperative level
    // Cooperative sound with other applications can be requested at cmd-line
    //if (M_CheckParm("-coopsound"))
        cooplevel = DSSCL_PRIORITY;
    //else
    //    cooplevel = DSSCL_EXCLUSIVE;

	LOG->Log("SetCooplevel...\n");

    hr = DiSnd->SetCooperativeLevel (hWndMain, cooplevel);
    if ( FAILED( hr ) ) {
        InitFail("CAudio::CAudio() - SetCooperativeLevel() FAILED");
        return;
    }

    // Set up DSBUFFERDESC structure.
    ZeroMemory (&dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof (DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER |
                      DSBCAPS_CTRLVOLUME;
    dsbdesc.dwBufferBytes = 0;      // Must be 0 for primary buffer
    dsbdesc.lpwfxFormat = NULL;     // Must be NULL for primary buffer


    // frequency of primary buffer may be set at cmd-line
    //p = M_CheckParm ("-freq");
    //if (p && p < myargc-1) {
    //    frequency = atoi(myargv[p+1]);
        //cons_printf (" requested frequency of %d hz\n", frequency);
    //}
    //else
        frequency = 22050;
    
    // Set up structure for the desired format
    ZeroMemory (&wfm, sizeof(WAVEFORMATEX));
    wfm.wFormatTag = WAVE_FORMAT_PCM;
    wfm.nChannels = 2;                              //STEREO SOUND!
    wfm.nSamplesPerSec = frequency;
    wfm.wBitsPerSample = 16;
    wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
    wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	LOG->Log("CreateSoundBuffer...\n");

    // Gain access to the primary buffer
    hr = DiSnd->CreateSoundBuffer (&dsbdesc, &lpDsb, NULL);

    // Set the primary buffer to the desired format,
    // but only if we are allowed to do it
    if (cooplevel >= DSSCL_PRIORITY)
    {
        if (SUCCEEDED ( hr ))
        {
            // Set primary buffer to the desired format. If this fails,
            // we'll just ignore and go with the default.
            hr = lpDsb->SetFormat (&wfm);
            if (FAILED(hr))
                InitFail("CAudio::CAudio() - SetFormat() FAILED");
        }
    }

    // move any on-board sound memory into a contiguous block
    // to make the largest portion of free memory available.
    if (cooplevel >= DSSCL_PRIORITY)
    {
        LOG->Log("Compacting onboard sound-memory...\n");
        hr = DiSnd->Compact();
        //cons_printf (" %s\n", SUCCEEDED(hr) ? "done" : "FAILED");
    }

    // set the primary buffer to play continuously, for performance
    // "... this method will ensure that the primary buffer is playing even when no secondary
    // buffers are playing; in that case, silence will be played. This can reduce processing
    // overhead when sounds are started and stopped in sequence, because the primary buffer
    // will be playing continuously rather than stopping and starting between secondary buffers."
	LOG->Log("Play() on primary sound buffer\n");
    hr = lpDsb->Play (0, 0, DSBPLAY_LOOPING);
    if ( FAILED ( hr ) )
        InitFail ("CAudio::CAudio() - Primary buffer continuous play FAILED");

#ifdef DEBUGSOUND

	LOG->Log( "DEBUGSOUND (ddscaps):\n\n" );
    
	DSCAPS              DSCaps;
    DSCaps.dwSize = sizeof(DSCAPS);
    hr = DiSnd->GetCaps (&DSCaps);

	if (SUCCEEDED (hr))
    {
        if (DSCaps.dwFlags & DSCAPS_CERTIFIED)
            LOG->Log ("This driver has been certified by Microsoft\n");
        if (DSCaps.dwFlags & DSCAPS_EMULDRIVER)
            LOG->Log ("No driver with DirectSound support installed (no hardware mixing)\n");
        if (DSCaps.dwFlags & DSCAPS_PRIMARY16BIT)
            LOG->Log ("Supports 16-bit primary buffer\n");
        if (DSCaps.dwFlags & DSCAPS_PRIMARY8BIT)
            LOG->Log ("Supports 8-bit primary buffer\n");
        if (DSCaps.dwFlags & DSCAPS_SECONDARY16BIT)
            LOG->Log ("Supports 16-bit, hardware-mixed secondary buffers\n");
        if (DSCaps.dwFlags & DSCAPS_SECONDARY8BIT)
            LOG->Log ("Supports 8-bit, hardware-mixed secondary buffers\n");

        LOG->Log ("Maximum number of hardware buffers: %d\n", DSCaps.dwMaxHwMixingStaticBuffers);
        LOG->Log ("Size of total hardware memory: %d\n", DSCaps.dwTotalHwMemBytes);
        LOG->Log ("Size of free hardware memory= %d\n", DSCaps.dwFreeHwMemBytes);
        LOG->Log ("Play Cpu Overhead (%% cpu cycles): %d\n", DSCaps.dwPlayCpuOverheadSwBuffers);
    }
    else
        LOG->Log (" couldn't get sound device caps.\n");

#endif

    // save pointer to the primary DirectSound buffer for volume changes
    DiSndPrimary = lpDsb;

    ZeroMemory (StackSounds, sizeof(StackSounds));

	//MessageBox(NULL, "audio started", "hahahaaa!!", MB_OK);

    LOG->Log("sound initialised.\n");
    bAudioStarted = true;
}


// -------------+
// ~CAudio		: Releases CAudio resources, shutdown DirectSound
// -------------+
CAudio::~CAudio()
{
    int i, t;

    LOG->Log("CAudio::~CAudio() ... ");

    // release any temporary 'duplicated' secondary buffers
    for (i=0,t=0; i<MAXSTACKSOUNDS; i++)
        if (StackSounds[i].lpSndBuf) {
            // stops the sound and release it if it is a duplicate
            StopSound (i);
			t++;
		}
    
    LOG->Log("\t\t\treleased %d sound buffers on exit\n", t);

    // this should release all DirectSoundBuffers as well
	if (DiSnd)
        SAFE_RELEASE( DiSnd );
}


// -------------+
// StartSound	:
// Start the given S_sfx[id] sound with given properties (panning, volume..)
// FIXME: if a specific sound Id is already being played, another instance
//        of that sound should be created with DuplicateSound()
// -------------+
STACKSOUNDID CAudio::StartSound( LPDIRECTSOUNDBUFFER dsBuf,
							     int iVol, int iSep, int iPitch, int iPriority)
{
    HRESULT     hr;
    int         lowestpri;
	STACKSOUNDID lowestprihandle;
    STACKSOUNDID handle = -1;
    DWORD       dwStatus;
    int         i;

    if( !bAudioStarted )
        return -1;

	LOG->Log( "CAudio::StartSound()\n");
    
	//CONS_Printf ("I_StartSound:\n\t\tS_sfx[%d]\n", id);

    // DirectSound can't play multiple instances of the same sound buffer
    // unless they are duplicated, so if the sound buffer is in use, make a duplicate
    lowestpri = 256;
    lowestprihandle = 0;
    for (i=0; i<MAXSTACKSOUNDS; i++)
    {
        //remember lowest priority sound
        if (StackSounds[i].lpSndBuf)
            if (StackSounds[i].priority < lowestpri) {
                lowestpri = StackSounds[i].priority;
                lowestprihandle = i;
            }

        // find a free 'playing sound slot' to use
        if (StackSounds[i].lpSndBuf==NULL) {
            LOG->Log ("\t\tfound free slot %d\n", i);
            break;
        }
        else
            // check for sounds that finished playing, and can be freed
            if( !SoundIsPlaying(i) )
            {
                LOG->Log ("\t\tfinished sound in slot %d\n", i);
                //stop sound and free the 'slot'
                StopSound (i);
                // we can use this one since it's now freed
                break;
            }
    }
    
    // the maximum of sounds playing at the same time is reached, if we have at least
    // one sound playing with a lower priority, stop it and replace it with the new one
    if( i>=MAXSTACKSOUNDS )
    {
        LOG->Log ("\t\tall slots occupied..");
        if( iPriority >= lowestpri ) {
            handle = lowestprihandle;
            LOG->Log (" kicking out lowest priority slot: %d pri: %d, my priority: %d\n",
                         handle, lowestpri, iPriority);
        }
        else {
            LOG->Log (" not enough priority mine: %d lowest is: %d\n", iPriority, lowestpri);
            return -1;
        }
    }
    else {
        // we found a free playing sound slot
        handle = i;
    }

    LOG->Log ("\t\tusing handle %d\n", handle);

    // if the original buffer is playing, duplicate it (DirectSound specific)
    // else, use the original buffer

    LPDIRECTSOUNDBUFFER dsNewBuf = dsBuf;

    //if( SoundIsPlaying( id ) ...
	dsBuf->GetStatus( &dwStatus );
    if (dwStatus & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING))
    {
        LOG->Log ("\t\toriginal sound buffer is playing, duplicating.. ");
		hr = DiSnd->DuplicateSoundBuffer( dsBuf, &dsNewBuf );
        if( FAILED(hr) )
        {
            // stop the original buffer then, and re-use it
			for (i=0; i<MAXSTACKSOUNDS; i++)
                if( StackSounds[i].lpSndBuf == dsBuf )
                {
                    StopSound( i );	//StackSounds[i].lpSndBuf = NULL;
                }
        }
    }

    // store information on the playing sound
    StackSounds[handle].lpSndBuf = dsNewBuf;
    StackSounds[handle].priority = iPriority;
    StackSounds[handle].duplicate = (dsNewBuf != dsBuf);

    LOG->Log ("StackSounds[%d].lpSndBuf is %s\n", handle, StackSounds[handle].lpSndBuf==NULL ? "Null":"valid");
    LOG->Log ("StackSounds[%d].priority is %d\n", handle, StackSounds[handle].priority);
    LOG->Log ("StackSounds[%d].duplicate is %s\n", handle, StackSounds[handle].duplicate ? "TRUE":"FALSE");

    UpdateSoundVolume( dsNewBuf, iVol );
    UpdateSoundPanning( dsNewBuf, iSep );

    dsNewBuf->SetCurrentPosition( 0 );

    hr = dsNewBuf->Play( 0, 0, 0 );
    if( hr == DSERR_BUFFERLOST )
    {
        /* restores the buffer memory and all other settings for the buffer
        hr = dsNewBuf->Restore();
        if( SUCCEEDED ( hr ) )
        {
            /*
			// reload sample data here
            lumpnum = S_sfx[id].lumpnum;
            if (lumpnum<0)
                lumpnum = S_GetSfxLumpNum (&S_sfx[id]);
            dsdata = W_CacheLumpNum (lumpnum, PU_CACHE);
            CopySoundData (dsBuf, (byte*)dsdata + 8);
            
            // play
            hr = dsBuf->lpVtbl->Play (dsBuf, 0, 0, 0);
        }
        else*/
            InitFail ("CAudio::StartSound() - Buffer lost");
    }

    // Returns a handle
    return handle;
}


// -------------+
// UpdateSoundVolume
//				: Update the volume for a secondary buffer, make sure it was
//				: created with DSBCAPS_CTRLVOLUME
// -------------+
void CAudio::UpdateSoundVolume( LPDIRECTSOUNDBUFFER lpSnd, int volume )
{
    HRESULT hr;
	int		dsvol;
    dsvol = (volume * ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4)) / VOL_RANGE +
                        (DSBVOLUME_MAX - ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4));
    LOG->Log ("CAudio::UpdateSoundVolume(vol=%d) dsvol=%d\n", volume, dsvol);
    hr = lpSnd->SetVolume( dsvol );
    //if (FAILED(hr))
    //    CONS_Printf ("SetVolume FAILED\n");
}


// -------------+
// UpdateSoundPanning
//				: Update the panning for a secondary buffer, make sure it was
//				: created with DSBCAPS_CTRLPAN
// -------------+
void CAudio::UpdateSoundPanning( LPDIRECTSOUNDBUFFER lpSnd, int sep )
{
    HRESULT hr;
    LOG->Log ("CAudio::UpdateSoundPanning(sep=%d)\n", sep);
    hr = lpSnd->SetPan( (sep * DSBPAN_RANGE)/SEP_RANGE - DSBPAN_RIGHT );
    //if (FAILED(hr))
    //    CONS_Printf ("SetPan FAILED for sep %d pan %d\n", sep, (sep * DSBPAN_RANGE)/SEP_RANGE - DSBPAN_RIGHT);
}


// -------------+
// SoundIsPlaying
// Returns TRUE of FALSE indicating if the sound is currently playing
// -------------+
BOOL CAudio::SoundIsPlaying( STACKSOUNDID id )
{
    LPDIRECTSOUNDBUFFER dsbuffer;
    DWORD   dwStatus;
    
    if( !bAudioStarted || id == -1 )
        return FALSE;

    dsbuffer = StackSounds[id].lpSndBuf;
    if( dsbuffer ) {
        dsbuffer->GetStatus( &dwStatus );
        if (dwStatus & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING))
            return TRUE;
    }
    
    return FALSE;
}


// -------------+
// StopSound	: Stop a sound if it is playing,
//				: free the corresponding 'playing sound slot' in StackSounds[]
//				: release the buffer if it was duplicated from a given buffer at StartSound()
// -------------+
void CAudio::StopSound( STACKSOUNDID id )
{
    LPDIRECTSOUNDBUFFER dsbuffer;
    HRESULT hr;

    if( !bAudioStarted || id<0 )
        return;

    LOG->Log ("CAudio::StopSound(%d)\n", id);
    
    dsbuffer = StackSounds[id].lpSndBuf;
    hr = dsbuffer->Stop();

    // free duplicates of original sound buffer (DirectSound hassles)
    if( StackSounds[id].duplicate ) {
        LOG->Log ("\t\trelease a duplicate..\n");
        dsbuffer->Release();
    }
    StackSounds[id].lpSndBuf = NULL;
}


// -------------+
// SetSfxVolume : set the global volume for sound effects
// -------------+
void CAudio::SetSfxVolume( int volume )
{
    int     vol;
    HRESULT hr;

    if( !bAudioStarted )
        return;
        
	LOG->Log( "CAudio::SetSfxVolume(%d)\n", volume);

    // use the last quarter of volume range
    if( volume )
        vol = (iSfxVolume * ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4)) / (VOL_RANGE-1) +
              (DSBVOLUME_MAX - ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4));
    else
        vol = DSBVOLUME_MIN;    // make sure 0 is silence

    hr = DiSndPrimary->SetVolume( vol );
	if( SUCCEEDED( hr ) )
		iSfxVolume = volume;
    //if (FAILED(hr))
    //    CONS_Printf ("setvolumne failed\n");
}


// -------------+
// UpdateSoundParams
//				: Update properties of a sound currently playing
// -------------+
void CAudio::UpdateSoundParams( STACKSOUNDID handle,
                                int        vol,
                                int        sep,
                                int        pitch )
{
    LPDIRECTSOUNDBUFFER     dsbuffer;

    if( !bAudioStarted )
        return;

    LOG->Log ("CAudio::UpdateSoundParams(vol=%d,sep=%d,pitch=%d)\n", vol,sep,pitch);

    dsbuffer = StackSounds[handle].lpSndBuf;
    if( dsbuffer ) {
        UpdateSoundVolume( dsbuffer, vol );
        UpdateSoundPanning (dsbuffer, sep );
    }
}


// -------------+
// Fill the DirectSoundBuffer with data from a sample
// -------------+
/*BOOL CAudio::CopySoundData( LPDIRECTSOUNDBUFFER dsbuffer, byte* data )
{
    LPVOID  lpvAudio1;              // receives address of lock start
    DWORD   dwBytes1;               // receives number of bytes locked
    HRESULT hr;

    // Obtain memory address of write block.
    hr = dsbuffer->lpVtbl->Lock (dsbuffer, 0, 0, &lpvAudio1, &dwBytes1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
    
    // If DSERR_BUFFERLOST is returned, restore and retry lock. 
    if (hr == DSERR_BUFFERLOST) 
    { 
        dsbuffer->lpVtbl->Restore (dsbuffer);
        hr = dsbuffer->lpVtbl->Lock (dsbuffer, 0, 0, &lpvAudio1, &dwBytes1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
    } 
    
    if ( SUCCEEDED (hr) )
    {
        // copy wave data into the buffer (note: dwBytes1 should equal to dsbdesc->dwBufferBytes ...)
        CopyMemory (lpvAudio1, data, dwBytes1);
    
        // finally, unlock the buffer
        hr = dsbuffer->lpVtbl->Unlock (dsbuffer, lpvAudio1, dwBytes1, NULL, 0);

        if ( SUCCEEDED (hr) )
            return true;
    }

    I_Error ("CopySoundData() : Lock, Unlock, or Restore FAILED");
    return false;
}*/


// -------------+
// LoadWave		: create a DirectSound buffer from RIFF Wave file data,
//				: pass the file handle with the file pointer set to the beginning
//				: of the wave file
// NOTE			: Remember to ->Release() the sound buffer when you have finished with it !
// -------------+
LPDIRECTSOUNDBUFFER CAudio::LoadWave( HANDLE hFile )
{
	HMMIO	 file = NULL;
	MMIOINFO mmioInfo;

	LOG->Log( "CAudio::LoadWave()\n" );

	// DirectSound buffer allocation
	LPDIRECTSOUNDBUFFER dsBuf = NULL;
    DSBUFFERDESC        dsbdesc;
    HRESULT             hr;
		// Writing the buffer
		void* write1 = 0; unsigned long length1 = 0;
		void* write2 = 0; unsigned long length2 = 0;

	// Open from standard file handle
	ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
	mmioInfo.adwInfo[0] = (DWORD) hFile;
	file = mmioOpen( NULL, &mmioInfo, 0 );
	if( file==NULL ) {
		InitFail( "CAudio::LoadWave() - mmioOpen() FAILED");
		goto errExit;
	}

	// Set up the specifer to find the wave data.
	MMCKINFO parent;
	ZeroMemory( &parent, sizeof(parent) );
	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if( mmioDescend( file, &parent, NULL, MMIO_FINDRIFF ) != MMSYSERR_NOERROR )
	{
		InitFail("CAudio::LoadWave() - mmioDescend() on 'WAVE' FAILED");
		goto errExit;
	}

	// Set up the specifer to find the fmt data.
	MMCKINFO child;
	ZeroMemory( &child, sizeof(child) );
	child.fccType = mmioFOURCC('f', 'm', 't', ' ');
	if( mmioDescend( file, &child, &parent, MMIO_FINDCHUNK ) != MMSYSERR_NOERROR )
	{
		InitFail("CAudio::LoadWave() - mmioDescend() on 'fmt ' FAILED");
		goto errExit;
	}

	PCMWAVEFORMAT	pcmw;
	WAVEFORMATEX	wfmtx;

	// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>
    // if there are extra parameters at the end, we'll ignore them
    if( child.cksize < (long) sizeof(PCMWAVEFORMAT) )
	{
		InitFail("CAudio::LoadWave() - 'fmt' chunk does not match PCMWAVEFORMAT size");
		goto errExit;
	}

	// Read the format.
	if( mmioRead( file, (HPSTR)&pcmw, (long) sizeof(pcmw) ) != (long) sizeof(pcmw) )
	{
		InitFail("CAudio::LoadWave() - can not read 'fmt' chunk");
		goto errExit;
	}

	// Make sure the wave data is the right format.
	if( pcmw.wf.wFormatTag != WAVE_FORMAT_PCM )
	{
		InitFail("CAudio::LoadWave() - 'fmt' chunk not PCM format");
		goto errExit;
	}

	// WE MUST READ cbSize extra bytes after PCMWAVEFORMAT and allocate a buffer
	// of size (PCMWAVEFORMATEX + cbSize) bytes, which will contain the attributes
	// needed by the format specified in wFormatTag
	// Right now, I don't need anything else than PCMWAVEFORMAT

	// Copy the bytes from the pcm structure to the waveformatex structure
	CopyMemory( &wfmtx, &pcmw, sizeof(pcmw) );
	wfmtx.cbSize = 0;

#ifdef DEBUGSOUND
	LOG->Log( "DEBUGSOUND (wavefmt):\n\n" );
	LOG->Log( "WAVEFORMATEX\n");
	LOG->Log( "\tcbSize\t%d\n", wfmtx.cbSize );
	LOG->Log( "\tnAvgBytesPerSec\t%d\n", wfmtx.nAvgBytesPerSec );
	LOG->Log( "\tnBlockAlign\t%d\n", wfmtx.nBlockAlign );
	LOG->Log( "\tnSamplesPerSec\t%d\n", wfmtx.nSamplesPerSec );
	LOG->Log( "\tnChannels\t%d\n", wfmtx.nChannels );
	LOG->Log( "\twBitsPerSample\t%d\n", wfmtx.wBitsPerSample );
#endif

	// Ascend the input file out of the 'fmt ' chunk
	if( mmioAscend( file, &child, 0 ) != MMSYSERR_NOERROR )
	{
		InitFail("CAudio::LoadWave() - mmioAscend() FAILED");
		goto errExit;
	}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');

	// And down to the data chunk.
	if( mmioDescend( file, &child, &parent, MMIO_FINDCHUNK ) )
	{
		InitFail("CAudio::LoadWave() - mmioDescend() to 'data' chunk FAILED");
		goto errExit;
	}

    // Set up DSBUFFERDESC structure.
    ZeroMemory (&dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof (DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRLPAN |
                      DSBCAPS_CTRLVOLUME |
                      DSBCAPS_STICKYFOCUS |
                      //DSBCAPS_LOCSOFTWARE |
                      DSBCAPS_STATIC;
    dsbdesc.dwBufferBytes = child.cksize;
    dsbdesc.lpwfxFormat = &wfmtx;				// pointer to WAVEFORMATEX structure

    // Create the sound buffer
    hr = DiSnd->CreateSoundBuffer( &dsbdesc, &dsBuf, NULL );
    if( FAILED( hr ) ) {
        InitFail("CAudio::LoadWave() - CreateSoundBuffer() FAILED");	//DXErrorToString() ...
		goto errExit;
	}
 
    // fill the DirectSoundBuffer waveform data -------------------------------------

        // Lock the buffer.
        hr = dsBuf->Lock( 0, child.cksize, &write1, &length1, &write2, &length2, 0 );

		if( FAILED( hr ) ) {
			InitFail("CAudio::LoadWave() - Lock() FAILED");
			goto errExit;
		}
		
        // read two parts for looping samples ?
		if( write1 > 0 ) {
            LOG->Log( "write1 %0x length %d\n", (DWORD)write1, length1 );
			
			// Copy the first bit of memory.
            if( mmioRead( file, (HPSTR)write1, (long) length1 ) != (long) length1 ) {
				InitFail("CAudio::LoadWave() - can not read wave data (part 1)");
				goto errExit;
			}
        }
        if( write2 > 0 ) {
            LOG->Log( "write2 %0x length %d\n", (DWORD)write2, length2 );
            // Copy the second bit of memory.
            if( mmioRead( file, (HPSTR)write2, (long) length2 ) != (long) length2 ) {
				InitFail("CAudio::LoadWave() - can not read wave data (part 2)");
				goto errExit;
            }
        }

        // Unlock the buffer.
        hr = dsBuf->Unlock( write1, length1, write2, length2 );
        if( FAILED( hr ) ) {
			InitFail("CAudio::LoadWave() - Unlock() FAILED");
			goto errExit;
		}
			
	// ------------------------------------------------------------------------------
   
	// Close the multimedia handle, NOT the file handle we passed to it!!
	mmioClose( file, MMIO_FHOPEN );

	LOG->Log( "CAudio::LoadWave SUCCESFUL\n\n");

	// cool exit - return DirectSound buffer pointer
	return dsBuf;

// error exit - free whatever was allocated - return NULL
errExit:
	if( file!=NULL )
		mmioClose( file, 0 );
	SAFE_RELEASE( dsBuf );
	return NULL;
}


// -------------+
// StopAllSounds: stop all playing sounds immediately
// -------------+
void CAudio::StopAllSounds()
{
    int i;

    for (i=0; i<MAXSTACKSOUNDS; i++)
        if (StackSounds[i].lpSndBuf) {
            // stops the sound and release it if it is a duplicate
            StopSound (i);
		}
}

