// drm_game.h : DrMario game code header file

// this is shared by drm_game main game code,
// and drmario.cpp win32 app code

// this is the reference resolution for the graphics,
// the screen could have an other size
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	200
#define SCREEN_BPP		8

// current palette used in drm_game code, restored when window to fullscreen mode change
extern  PALETTEENTRY	main_pal[256];

// the 3 main functions called by the gui app
boolean Game_Init ( BOOL bSound );
boolean	Game_Main (void);
void	Game_Shutdown (void);

// DrMario.cpp call this in drm_game.h
void	SetSoundEnabled( BOOL bEnabled );
DWORD	GetPlayerVars( int iPlayer );
void	SetDefaultKeys( struct PlayerVars* plV );

void PauseGame();
void UnpauseGame();
