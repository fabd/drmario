// drm_game.cpp : DrMario main game code

#include <time.h>			// for randomize functions

// ------------------------
// DrMario game definitions
// ------------------------
#include "DrMario.h"
#include "errors.h"

#include "CAudio.h"			// isn't it beautiful ? :D
#include "CVideo.h"
#include "CWad.h"

#include "fabgrafx.h"
#include "drm_game.h"


// ---------------
// Drmario globals
// ---------------
	PALETTEENTRY	main_pal[256];
	// for fade in fade out
	PALETTEENTRY	src_pal[256];
	PALETTEENTRY	dest_pal[256];

	// this tells the main game loop to exit
	static boolean  QuitGame;
	static boolean	bGamePaused;

	static CAudio*	AUD = NULL;
	static BOOL bSoundEnabled;

// -----------------------------------------------------------------------------
// Protos
// -----------------------------------------------------------------------------

static  UBYTE  *ReserveBackground(UWORD nbytes);
static  void    FreeBackground(UBYTE *p_buf);

void    Fade_to( PALETTEENTRY* cmap_a, PALETTEENTRY* cmap_b, PALETTEENTRY* cmap_c, int level );

static  boolean DisplayPic(UBYTE *picdat,SWORD x0,SWORD y0);
static  void    PasteIcon(SLONG xpos,SLONG ypos,UWORD blocknum,UBYTE mode);
static  void    PasteVirus(UWORD xpos,UWORD ypos,UWORD frame );
static  void    Aff_Zone(UWORD xpos,UWORD ypos,UBYTE *zonemap);
static  void    Aff_Tuile(UWORD xpos,UWORD ypos,UWORD casenum);
static  void    Aff_Tuile_Transp(UWORD xpos,UWORD ypos,UWORD casenum);
static  void    FillRect(UWORD x1,UWORD y1,UWORD x2,UWORD y2,UBYTE coul);
static  void    Copy_block(UBYTE *p_src,UBYTE *p_dest,UWORD width,UWORD height,UWORD sourcew,UWORD destw);

static  void    AnimIcon(struct Icon_s *icon,UBYTE *animseq,UWORD firstframe,UWORD framerate,SWORD xpos,SWORD ypos,UWORD mode);
static  void    RefreshIcon(struct Icon_s *icon);
static  void    ClearBob(struct Icon_s *icon);

static  boolean Alloc_all(void);
static  void    Restore_all(void);

static  int     DrMario_TitlePage(void);
static  void    DrMario_OptionsPage(void);
static  void    DrMario_OnePlayerGame(void);
static  void    DrMario_TwoPlayerGame(void);

static  void    Aff_Digits( UBYTE ncar, ULONG value, UWORD xpos, UWORD ypos );

// Routines globales du jeu (1player ET 2player) dans l'ordre chronologique

static  void  Init_Level(void);             //boucle niveau suivant...      
static  void  Remplit_Bouteille(void);      // remplissage avec des virus   
static  void  Next_Gellule(void);           // boucle gellule suivante...   
static  void   Lance_Gellule(void);         // Mario lance la gellule       
static  void  Pose_Gellule_Main_Loop(void); //  boucle deplace et pose gell 
static  void  Boucle_Destruction(void);     //  boucle eliminations en serie
static  void   Attente_Explosion(void);
static  void   Les_gellules_tombent(void);
static  void   Attente_chutes(void);

static  void  WinGame_1pl(void);
static  void   Win1pl_clearup(void);
static  void   Win1pl_afterclearup(void);
static  void   Win_boucle_1pl(void);
static  void  GameOver_1pl(void);
static  void   GameOver1pl_clearup(void);
static  void   GameOver1pl_afterclearup(void);
static  void   GameOver_boucle_1pl(void);
static  void  WinGame_2pl(void);
static  void   Win2pl_clearup(void);
static  void   Win2pl_afterclearup(void);
static  void   Win_boucle_2pl(void);
static  void  GameOver_2pl(void);
static  void   GameOver2pl_clearup(void);
static  void   GameOver2pl_afterclearup(void);
static  void   GameOver_boucle_2pl(void);
static  void  Attends_et_fermela(void);
static  void  Efface_les_cases_detruites();

// Routines appell?es par les routines globales

static  void  Move_Viruses_ala_loupe(void); // mode 1player (animation)
static  void  Deplace_Gellule(void);        // deplace la gellule (interaction)


// Routines appell?es par Deplace_Gellule()
static  void    Deplace_Gellule(void);
static  void    A_Droite(void);
static  void    A_Gauche(void);
static  void    Turn_Clockwise(SWORD dxjoy);
static  void    Turn_AntiClockwise(void);
static  void    Final_Draw_Gellule(void);
static  void    Draw_Gellule_Horiz(void);
static  void    Push_Down(void);
static  void    Gellule_Go_Down(void);

static  void    Gellule_Bloquee(void);

static  void    Aff_Case(UWORD xcase,UWORD ycase,UWORD casenum);
static  void    Destroy_Gellules(SWORD x,SWORD y,SWORD nb,UBYTE coul,SWORD mx,SWORD my);
static  boolean Seek_And_Destroy(void);

static  void    Clear_Bottle(void);
static  void    Eff_Case(UWORD x,UWORD y);

static  void    PlaySound( GameSound_t soundid );

static  void    Anime_virus(void);

static  void    Make_transp_table();
static  void    Transp_effect();


// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------


static  UWORD   VirusMsg[3];
static  boolean VirusAvance;   /* mis ? ON quand les TROIS virus tournent*/

static  UWORD   DigitMode = dmWHITEONBLACK;


//static  UBYTE   *p_cmap_a,*p_cmap_b,*p_cmap_c;
                                    // 3 Colormaps for various effects.
// oldchtuff from DOS version
//static  BOOLboolean  Music,Sound,Stereo,Reverse,SeizeBits,Interpolate;
//static  SLONG    MixFreq,DmaBuf;
//static  UNIMOD  *p_titlemus;
//static  UBYTE   First_Voice;
//static  UBYTE   Last_Voice;

// updated 19/08/1999 - use CAudio class
// (to remember the old code did +1 with a NULL pointer ..??)
static  LPDIRECTSOUNDBUFFER Sound_ptr[MAX_SAMPLES];

static  SWORD    NPlayers;      /* number of players should be 1 or 2       */
static  boolean  EscapeKey;     /* mis ? 1 pendant le jeu signale la touche */

static  SWORD    VirusLevel1P;  /* entre 0 et 20 soit de 4 ? 84 virus!      */
static  SWORD    VirusLevel2P;
static  SWORD    Speed1P;       /* speed LOW MED HI soit 1,2,3              */
static  SWORD    Speed2P;
static  SWORD    MusicType;     /* 1= FEVER 2= CHILL 3= OFF                 */



// variables pour l'animation gellule lanc?e par Mario en 1 player
static  UBYTE   *p_lance_bg;       // pour sauver/restorer le fond
static  struct  Icon_s  Bras_lance;
static  SWORD    Anim_lance_cpt;
static  SWORD    Anim_lance_pos;


// variables TEMPORAIRES pour ne pas toujours passer par un pointeur
// sur structure ( Pl->xxx ).                                        

static  UBYTE   GE[5];

static  SWORD    GX,GY;      /* GelluleX/Y sert aux deux joueurs, pour
                               pas passer ? travers une structure (pl->)
                               car GX et GY sont TRES SOUVENT utilises  */


static  UBYTE   Bottle1[8+1][16+1];
static  UBYTE   Bottle2[8+1][16+1];

static  UBYTE   (*Bottle)[8+1][16+1];

// one for each player
static	struct	PlayerVars	Pl1,Pl2;

// the game code uses these :
//   Pl			is the current player being processed
//   AutrePl	is the opposite player	(otherPl)
static  struct  PlayerVars  *Pl,*AutrePl;

static  ULONG   Score;
static  ULONG   TopScore;      /* SEULEMENT en mode 1 player */

static  UBYTE   *p_titlepic;   /* points to LBM titlepicture */
static  UBYTE   *p_optionpg;   /* pointe la page d'options format LBM */
static  UBYTE   *p_oneplayr;   /* pointe la page one player format LBM */
static  UBYTE   *p_twoplayr;   /* pointe la page Two players format LBM */

static  UBYTE   *p_blokpage;   /* pointe la page des blocs avec la fonte */
static  UBYTE   *p_bottlbg1;   /* fond de bouteille numero 1 (80x112)    */
static  UBYTE   *p_viruspag;   /* page raw avec 27 frames de virus       */

static  SBYTE    *p_circle;     /* table de 120 positions en cercle       */
                               /* 120 paires de x,y (coords relatives)   */
static  SBYTE    *p_transptable;/* table pour effet de transp. a la loupe */
                               /* construit par 'Make_transp_table'      */
static  UBYTE   *p_main_bg;    /* buffer background principal, appeller  */
                               /* la routine ReserveBackground() pour en */
                               /* r?server des petits bouts (cf. routine)*/
static  UBYTE   *p_current_bg; /* position en cours dans p_main_bg       */
static  UBYTE   *block[MAXBLOCKS];
                               /* each entry points a loaded block into
                                  memory */

static  void    *p_blocks;     /* points to block buffer (to free) */


static  UBYTE   *p_marioinzebox; /* on y sauve mario dans la petite fen?tre   */
                                 /* (en mode 1pl) pour le remettre de temps   */
                                 /* en temps quand on l'a effac? (WinGame_1pl)*/

static  SWORD    i,j,k;         /* general usage */


//
//
//
boolean  Alloc_all(void)
{
    // ALLOCATE SPACE FOR THREE COLORMAPS - FOR VARIOUS FADE EFFECTS...
	/*
    if( (p_cmap_a = (UBYTE *) AllocMem( COLORMAPSIZE )) == NULL )
        InitFail("Err:Memory Alloc for Colormap.");

    if( (p_cmap_b = (UBYTE *) AllocMem( COLORMAPSIZE )) == NULL )
        InitFail("Err:Memory Alloc for Colormap.");

    if( (p_cmap_c = (UBYTE *) AllocMem( COLORMAPSIZE )) == NULL )
        InitFail("Err:Memory Alloc for Colormap.");
	*/

    // SAVEBACKGROUND BUFFER POUR LA GELLULE LANCEE PAR MARIO EN 1 PLAYER
    if( (p_lance_bg = (UBYTE *) AllocMem( TUILEW*2*TUILEH*2 )) == NULL )
        InitFail("Err:Memory Alloc.");

    // MAIN BACKGROUND BUFFER UTILISE A TRAVERS ReserveBackground()
    if( (p_main_bg = (UBYTE *) AllocMem(MAINBGBUFSIZE)) == NULL )
        InitFail("Err:mem alloc main background buffer.");

    // Buffer pour l'effet de transparence ? la loupe
    if( (p_transptable = (SBYTE *) AllocMem( TRANSPTABLEMAX )) == NULL )
        InitFail("Err:mem alloc transparent table.");
	
	return true;
}


static  void Restore_all(void)
{
    FreeMem( p_main_bg );
    FreeMem( p_transptable );
    FreeMem( p_lance_bg );       /* background buffer */

    //if( p_cmap_c != NULL )  FreeMem( p_cmap_c );
    //if( p_cmap_b != NULL )  FreeMem( p_cmap_b );
    //if( p_cmap_a != NULL )  FreeMem( p_cmap_a );       /* free COLORMAPS */
}


/*******************************************************************\
*   Fade_To                                                         *
*       Fades cmap_a from cmap_b to cmap_c with given percentage.   *
*       Only cmap_a change! cmap_b and cmap_c are unchanged.        *
*       Look at this:                                               *
*       0% ........ 50% ........ 100%                               *
*      cmap_b      between        cmap_c                            *
*       So with 0% to 100% you fade from b to c (modifying a)       *
*       But with 100% to 0% you fade from c to b ! (simple ,eh ?)   *
*   Output:                                                         *
*       cmap_a is faded. cmap_b & cmap_c doesn't change.            *
\*******************************************************************/

void    Fade_to( PALETTEENTRY* cmap_a, PALETTEENTRY* cmap_b, PALETTEENTRY* cmap_c, int level )
{

	int a_val,b_val,diff,i;

    for( i=0; i < 256; i++ )
    {
		a_val =  cmap_b->peRed;	// source value
		b_val =  cmap_c->peRed;	// destination value
        diff = (( b_val - a_val ) * level) / 100;
        cmap_a->peRed = (UBYTE)(( a_val + diff ) & 255);

		a_val =  cmap_b->peGreen;
		b_val =  cmap_c->peGreen;
        diff = (( b_val - a_val ) * level) / 100;
        cmap_a->peGreen = (UBYTE)(( a_val + diff ) & 255);
		
		a_val =  cmap_b->peBlue;
		b_val =  cmap_c->peBlue;
        diff = (( b_val - a_val ) * level) / 100;
        cmap_a->peBlue = (UBYTE)(( a_val + diff ) & 255);

		cmap_a++;
		cmap_b++;
		cmap_c++;
	}
}


/*** ReserveBackground()
**
**   Cette routine ?hont?e r?serve simplement un morceau du buffer
**   de sauvegarde de fond principal p_main_bg.
**
**   L'adresse renvoy?e peut alors servir ? sauver le fond d'une
**   animation RefreshIcon().
**
**   Utilisez les macros BOBHEIGHT,BOBWIDTH pour calculer la taille
**   n?cessaire ? r?server.
**
**   Quand vous avez fini, LIBEREZ les buffers avec FreeBackground,
**   dans l'ordre EXACTEMENT INVERSE des allocations, car le syst?me
**   archi-simple utilis? fontionne comme une pile.
**
**   Pas question d'utiliser AllocMem() ? chaque fois, ce serait trop LOURD.
**
*****************************************************************************/
static UBYTE *ReserveBackground( UWORD nbytes )
{
static UWORD *duckptr;

    if( nbytes & 3 )
        nbytes = (nbytes & ~3)+4;        /* aligner sur DOUBLE WORD */

    duckptr = (UWORD *) p_current_bg;

    p_current_bg += nbytes+2;

    if( (p_current_bg-p_main_bg) > MAINBGBUFSIZE )
        InitFail("MAINBGBUFFER too small!");

    *( duckptr++ ) = nbytes;             /* enregistre taille du buffer */

    return( (UBYTE *)duckptr );          /* nul! nul! nul! */
}

/*** FreeBackground()
**
**   Comme la taille des buffers allou?s par ReserveBackground() est
**   enregistr?e au d?but de chaque buffer, il suffit de donner l'adresse
**   du buffer et rien d'autre.
**
***************************************************************************/
static void FreeBackground( UBYTE *p_buf )
{
    p_buf -= 2;
    p_current_bg -= ( *( (UWORD *) p_buf ) +2 );
}



/*** Aff_Tuile -v1.0- Dim 20/10/1996.
**
**  Affiche une tuile (de BLOKPAGE) aux coords ECRAN ( x:0->319, y:0->195 ).
**
****************************************************************************/
static  void    Aff_Tuile( UWORD xpos, UWORD ypos, UWORD casenum )
{
	UBYTE   *p_dest;
	UBYTE   *p_src;
	UWORD   offs;

    /* trouve la tuile dans BLOKPAGE et affiche */
    /* 20 tuiles par rangee                     */
    /* 20 * TUILEW octets par ligne de BLOKPAGE */

    offs  = ( casenum / 20 ) * TUILEH * 20 * TUILEW;
    offs += ( casenum % 20 ) * TUILEW;
    p_src = p_blokpage + offs;

    p_dest = VID->pScreen() + xpos + ( ypos * SCREEN_WIDTH );

    /* copie SANS CLIPPING de la tuile sur l'?cran virtuel */

    for( i=0; i<TUILEH; i++ )
    {
        for( j=0; j<TUILEW; j++ )
            *( p_dest++ ) = *( p_src++ );
        p_src  += 19 * TUILEW;
        p_dest += SCREEN_WIDTH - TUILEW;
    }

}


/*** Aff_Zone -v1.0- Dim 20/10/1996.
**
**  Affiche une zone de cases de BLOKPAGE ? partir des coordonn?es
**  de d?part xpos, ypos (coord ECRAN).
**
**  Les caract?res sp?ciaux suivants sont reconnus:
**
**  \0  fin de la zone,
**  \1  passe une case( case vide ),
**  \2  passe une ligne, retour ? xpos, ligne suivante,
**
************************************************************************/
static  void    Aff_Zone( UWORD xpos, UWORD ypos, char* zonemap )
{

	int   code;
	int   x,y;

    x = xpos;
    y = ypos;

    while( (code = *( zonemap++ )) != '\0' )
    {
        switch( code )
        {
            case 1:
                x += TUILEW;
                break;
            case 2:
                x  = xpos;
                y += TUILEH;
                break;
            default:
                Aff_Tuile( x, y, code );
                x += TUILEW;
        }
    }
}

#define INVIS_FRAME  0xFFFF


/**** AnimIcon -v1.0- Jeu 17/10/1996.
**
**  Initialise une structure Ic?ne, on peut ensuite appeller RefreshIcon()
**  ? chaque VBL pour animer l'Icon.
**
**  Format de la s?quence animseq:
**
**   un caract?re (UBYTE) par commande.
**   \0      termine la s?quence, celle-ci repart au d?but.
**   \1      stoppe la s?quence.
**           Copier icon->animseq dans icon->p_anim pour red?marrer.
**   'A'-'Z' chaque lettre majuscule correspond ? un offset (0 ? 25) addition?
**           au num?ro de block firstframe, pour trouver l'image ? afficher.
**   '-'     correspond ? une image vide, pour faire clignoter un objet.
**
**   ex: 'ABCB' avec firstframe 4 donne: 4,5,6,5, 4,5,6,5, ...
**
**
*********************************************************************/
static  void    AnimIcon( struct Icon_s *icon, char* animseq, UWORD firstframe, UWORD framerate, SWORD xpos, SWORD ypos, UWORD gmode )
{

    icon->currentframe = INVIS_FRAME;       /* image vide (au cas o?...) */
    icon->initframe = firstframe;
    icon->framerate = framerate;
    icon->animseq   = animseq;    /* pointe TOUJOURS le d?but de la seq. */
    icon->p_anim    = animseq;    /* pointe pos. en cours dans animseq   */
    icon->counter   = 0;          /* ceci initialisera la premi?re frame */
    icon->xpos      = xpos;
    icon->ypos      = ypos;
    icon->mode      = gmode;      /* mode d'affichage COPYPUT,TRANSPUT,..*/
    icon->p_bgbuf   = NULL;       /* pointeur sur buffer de fond         */
    icon->xold      = -1;         /* derni?re position de sauvegarde du  */
    icon->yold      = 0;          /* fond, xold -1 signifie que le fond  */
                                  /* n'est pas encore sauv?              */
}


// called when menu item 'sound' is toggled
void SetSoundEnabled( BOOL bEnabled )
{
	if( !(bSoundEnabled = bEnabled) && AUD )
		AUD->StopAllSounds();
}

// 
static  void    PlaySound( GameSound_t soundid )
{
	// ---------------------------
	// NEW CODE USING CAudio class
	// ---------------------------
	LPDIRECTSOUNDBUFFER lpSfx;
	int	pan;

	if( !AUD || !AUD->AudioStarted() || !bSoundEnabled )
		return;

	if( soundid<0 || soundid >= MAX_SAMPLES )
		InitFail( "PlaySound() - invalid sound id (%d)", soundid );

    lpSfx = Sound_ptr[soundid];    /* pointeur sur structure sample MIKMOD */

    pan = Pl->Panning;
	
	// note: priority always 128, not used, pitch is not used
	AUD->StartSound( lpSfx, MAX_VOLUME, Pl->Panning, 13000, 128 );

#if 0
	// -------------------------------
	// OLD CODE using MIKMOD under DOS
	// -------------------------------
static  UBYTE   voice=46;       /* force l'initialisation de voice au 1er */
static  UBYTE   pan;           /* appel de la routine                    */

  if( Sound )
  {
static  MIKMOD_SAMPLE *s;

    s = Sound_ptr[soundnum];    /* pointeur sur structure sample MIKMOD */

    if( ++voice > Last_Voice )
        voice = First_Voice;    /* changer de voix pour entendre plusieurs */
                                /* samples en m?me temps */

    MD_VoiceSetVolume( voice, 64 );

    if( Reverse )
        pan = 255 - Pl->Panning;
    else
        pan = Pl->Panning;

    MD_VoiceSetPanning( voice, pan );

    MD_VoiceSetFrequency( voice, 13000 );

    MD_VoicePlay( voice,s->handle,0,s->length,0,0,s->flags );
  }
#endif
}


#define CLIP_LEFT       0
#define CLIP_TOP        0
#define CLIP_RIGHT      SCREEN_WIDTH - 1
#define CLIP_BOTTOM     SCREEN_HEIGHT - 1      /* INCLUSIVE coords */

/* ==================================================================== */
/* DisplayPic -- displays a packed picture at (x0,y0) coords.           */
/* (this is Doom packed picture format)                                 */
/*                                                                      */
/* Coords are clipped to screen boundaries (see defines below)          */
/*                                                                      */

//put a return value just for the err_exit routine
static  boolean    DisplayPic( UBYTE *picdat, SWORD x0, SWORD y0 )
{
    UWORD   xsize, ysize;
    SWORD    xofs, yofs;
    UWORD   nColumns, nCurrentColumn;

    SWORD    *p_word;                // pour lire le d?but
    ULONG  *lpNeededOffsets;       // pointe la table des offsets_colonnes
    UBYTE   *lpColumn;              // pointe dans une colonne
    UBYTE   *p_dest;                // screen destination

    SWORD    x, y;
    UWORD   i, n;
    UBYTE   bRowStart, bColored;

#define TEX_COLUMNSIZE      512L

    p_dest = VID->pScreen();

   /* get picture size & offsets */

    p_word = (SWORD *) picdat;

    xsize = *( p_word++ );
    ysize = *( p_word++ );
    xofs  = *( p_word++ );          /* negative values means ABSOLUTE coords */
    yofs  = *( p_word++ );

   /* ignore the picture offsets */
    xofs = 0;
    yofs = 0;

   /* HORIZONTAL clipping */

    nColumns = xsize;
    nCurrentColumn = 0;

    if( x0 < CLIP_LEFT )
    {
        if( ( nCurrentColumn = CLIP_LEFT - x0 ) >= xsize )
            return true;                                     /* SPRITE OUT LEFT  */
        x0 = 0;
    }
    else if( (x0+xsize) > CLIP_RIGHT+1 )
    {
        if( ( nColumns -= ( x0 + xsize - (CLIP_RIGHT+1)) ) <= 0 )
            return true;                                     /* SPRITE OUT RIGHT */
    }

    lpNeededOffsets = (ULONG *) (picdat + 8);

    while( nCurrentColumn < nColumns )
    {
       /* calcule ptr au d?but de la colonne */
        lpColumn = picdat + *( lpNeededOffsets + nCurrentColumn );

       /* we now have the needed column data, one way or another, so write it */
        n = 1;
        bRowStart = *( lpColumn );

        while( bRowStart != 255 && n < TEX_COLUMNSIZE )
        {
            bColored = *( lpColumn + n );   /* number of pixels to draw       */
            n += 2;                         /* skip over 'null' pixel in data */

            x = x0 + xofs + nCurrentColumn;
            y = y0 + yofs + bRowStart;
            p_dest = VID->pScreen() + x + (y * SCREEN_WIDTH);
            for( i = 0; i < bColored; i++ )
            {
                if( ( y >= CLIP_TOP ) && ( y <= CLIP_BOTTOM ) )
                    *( p_dest ) = *( lpColumn + i + n );

                p_dest += SCREEN_WIDTH;
            }

            n += bColored + 1;  /* skip over written pixels, and the 'null' one */
            bRowStart = lpColumn[ n++ ];
        }
        if( bRowStart != 255 )
            InitFail("BUG: DisplayPic: bRowStart != 255.");

	    nCurrentColumn++;
    }
	return true;
}


/*** PasteIcon -1.0- Jeu 17/10/1996.
**
**   Le block est au format BLK.
**
**   Deux modes possibles actuellement:
**
**    COPYPUT, TRANSPUT.
**   Avec TRANSPUT, la couleur TRANSP_COL indique les zones transparentes.
**
*************************************************************************/
static  void    PasteIcon( LONG xpos, LONG ypos, UWORD blocknum, UBYTE mode )
{

auto    UWORD   xsize, ysize;
auto    SWORD   xofs,  yofs;

auto    SWORD   *p_word;                // pour lire le d?but
auto    UBYTE   *p_dest;                // screen destination
auto    UBYTE   *p_src;                 // source Block

auto    int    xdeb,ydeb;
auto    int    nColumns, nLines, modulox;
auto    int   i, j;

auto    UBYTE   col;


   /* get picture size & offsets */

    p_word = (SWORD *) block[ blocknum ];

    xsize = *( p_word++ );
    ysize = *( p_word++ );
    xofs  = *( p_word++ );
    yofs  = *( p_word++ );

   /* ignore the picture offsets */
    xofs = 0;
    yofs = 0;

   /* horizontal clipping */

    modulox = 0;

    p_src = (UBYTE *) p_word;

    nColumns = xsize;

    if( xpos < CLIP_LEFT )
    {
        if( (xdeb = CLIP_LEFT - xpos) >= xsize )
            return;
        xpos = 0;
        p_src += xdeb;
        modulox += xdeb;
        nColumns -= xdeb;
    }
    else if( (xpos+xsize) > CLIP_RIGHT+1 )
    {
        if( ( nColumns -= ( xpos + xsize - (CLIP_RIGHT+1)) ) <= 0 )
            return;
    }

   /* vertical clipping */

    nLines = ysize;

    if( ypos < CLIP_TOP )
    {
        if( (ydeb = CLIP_TOP - ypos) >= ysize )
            return;
        ypos = 0;
        p_src += ( ydeb * xsize );
        nLines -= ydeb;
    }
    else if( (ypos+ysize) > CLIP_BOTTOM+1 )
    {
        if( ( nLines -= ( ypos + ysize - (CLIP_BOTTOM+1)) ) <= 0 )
            return;
    }


    p_dest = VID->pScreen() + xpos + ( ypos * SCREEN_WIDTH );

    if( mode == COPYPUT )
    {
        i = nLines;
        while( i-- > 0 )
        {
            j = nColumns;
            while( j-- > 0 )
            {
                *( p_dest++ ) = *( p_src++ );
            }
            p_src  += modulox;
            p_dest += SCREEN_WIDTH - nColumns;
        }
    }
    else if( mode == TRANSPUT )
    {
        i = nLines;
        while( i-- > 0 )
        {
            j = nColumns;
            while( j-- > 0 )
            {
                if( (col = *( p_src++ )) != TRANSP_COL )
                    *( p_dest ) = col;
                p_dest++;
            }
            p_src  += modulox;
            p_dest += SCREEN_WIDTH - nColumns;
        }
    }
}


/**** PasteVirus - affiche un virus ? l'?cran - NO CLIPPING -.
**
** On donne:
**
**  frame     -> num?ro d'image du virus de 0 ? 26.
**  xpos,ypos -> destination ? l'?cran ... NON CLIPPE!!!
**
** Il y a 27 sprites de virus rassembl?s dans une page RAW en grille
** de 9 virus par rang?e, sur 3 rang?es.
** Taille de chaque virus:
**     32 x 21 pixels.
** Dans cette page, la couleur 255 est transparente.
**
************************************************************************/
#define VIRUSPAGEW  VIRUSW*9
static  void    PasteVirus( UWORD xpos, UWORD ypos, UWORD frame )
{
static  UBYTE   *p_dest;
static  UBYTE   *p_src;
static  UBYTE   c;
static  UWORD   srcmod,destmod,x,y;

    srcmod  = VIRUSPAGEW - VIRUSW;
    destmod = SCREEN_WIDTH - VIRUSW;

    p_src  = p_viruspag + ((frame % 9)*VIRUSW);
    p_src += (frame/9) * VIRUSH * VIRUSPAGEW;

    p_dest = VID->pScreen() + xpos + (ypos * SCREEN_WIDTH);

    y = VIRUSH;
    while( y-- > 0 )
    {
        x = VIRUSW;
        while( x-- > 0 )
        {
            if( (c = *(p_src++)) != 255 )
                *(p_dest++) = c;
            else
                p_dest++;
        }
        p_src  += srcmod;
        p_dest += destmod;
    }
}


/**** utilis? quand DrMario lance une gellule en mode 1 player ****/
static  void    Aff_Tuile_Transp(UWORD xpos,UWORD ypos,UWORD casenum)
{
static  UBYTE   *p_dest;
static  UBYTE   *p_src;
static  UWORD   offs;
static  UBYTE   col;
    offs  = ( casenum / 20 ) * TUILEH * 20 * TUILEW;
    offs += ( casenum % 20 ) * TUILEW;
    p_src = p_blokpage + offs;
    p_dest = VID->pScreen() + xpos + ( ypos * SCREEN_WIDTH );
    for( i=0; i<TUILEH; i++ )
    {
        for( j=0; j<TUILEW; j++ )
        {
            if( (col=*(p_src++)) != 255 )
                *( p_dest++ ) = col;
            else
                p_dest++;
        }
        p_src  += 19 * TUILEW;
        p_dest += SCREEN_WIDTH - TUILEW;
    }
}

/*** FillRect -v1.0- Lun 21/10/1996.
**
**  Remplit un rectangle avec UNE COULEUR, coords ECRAN (inclusive).
**
**  NO duckIN' CLIPPING!!!!
**
****************************************************************************/
static  void    FillRect( UWORD x1, UWORD y1, UWORD x2, UWORD y2, UBYTE coul )
{
static  UBYTE   *p_dest;
static  UWORD   xsize,ysize,xcount;

    p_dest = VID->pScreen() + x1 + ( y1 * SCREEN_WIDTH );

    xsize = x2 - x1 + 1;
    ysize = y2 - y1 + 1;

    while( ysize > 0 )
    {
        xcount = xsize;
        while( xcount-- > 0 )
            *( p_dest++ ) = coul;

        p_dest += SCREEN_WIDTH - xsize;
        ysize--;
    }
}


/**** RefreshIcon -v1.0- Jeu 17/10/1996.
**
**  Un 'Icon' est comme un sprite, sauf qu'il n'est pas transparent,
**  et que le fond d'?cran n'est pas restaur?. Un Icon est affich? en
**  format BLK, et sert plut?t ? animer des parties d'?cran.
**
*********************************************************************/
static  void    RefreshIcon( struct Icon_s *icon )
{
static  UBYTE  code;
static  SWORD   *p_block;
static  SWORD   bobwidth,bobheight;

    if( icon->p_anim == NULL )
        return;                 /* anim sequence stopped */

    if( --(icon->counter) <= 0 )
    {
        code = *(icon->p_anim++);

        if( code == '\0' )              /* RESTART anim sequence */
        {
            code = *(icon->animseq);          /* read next anim command      */
            icon->p_anim = (icon->animseq) + 1;
        }
        else if( code == '\1' )         /* STOP anim sequence */
        {
             icon->p_anim = NULL;
             return;
        }

        if( code == '-' )
            icon->currentframe = INVIS_FRAME;     /* image vide */
        else
            icon->currentframe = icon->initframe + code - 'A';

        icon->counter = icon->framerate;
    }

    /* Affichage Icon                                             */
    /* 1) sauver/restaurer le fond.                               */
    /*    UNIQUEMENT si icon->p_bgbuf contient une adresse valide */
    /*    pour le buffer de sauvegarde du fond.                   */
    if( (icon->p_bgbuf != NULL) )
    {
        /* restaurer le fond (s'il a d?j? ?t? sauv?) */
        if( icon->xold != -1 )
            CopyBlock(icon->p_bgbuf, 0,0, icon->oldw,icon->oldh, icon->oldw,
					  VID->pScreen(), icon->xold, icon->yold, SCREEN_WIDTH);

        /* sauver le nouveau fond , SAUF SI C'EST L'IMAGE VIDE */
        if( icon->currentframe != INVIS_FRAME )
        {
            /* si c'est un BLOCK on peut y lire la taille du bob */
            if( icon->mode != VIRUSPUT )
            {
               p_block   = (SWORD *) block[icon->currentframe];
               bobwidth  = *(p_block++);
               bobheight = *(p_block);
            }
            else
            {
               bobwidth  = 32;
               bobheight = 21;   /* pas propre! -!- AMELIORER -!- */
            }

            icon->xold = icon->xpos;
            icon->yold = icon->ypos;
            icon->oldw = bobwidth;
            icon->oldh = bobheight;
            CopyBlock(VID->pScreen(),icon->xpos,icon->ypos,bobwidth,bobheight,SCREEN_WIDTH,
				      icon->p_bgbuf,0,0,bobwidth);
        }
    }

    /* 2) affichage du bob, sauf si c'est l'image vide! */
    if( icon->currentframe == INVIS_FRAME )
        return;

    switch( icon->mode )
    {
     case COPYPUT:
         PasteIcon( icon->xpos, icon->ypos, icon->currentframe, COPYPUT );
          break;
     case TRANSPUT:
         PasteIcon( icon->xpos, icon->ypos, icon->currentframe, TRANSPUT );
          break;
     case VIRUSPUT:
         PasteVirus( icon->xpos, icon->ypos, icon->currentframe );
          break;
    }
}

/**** ClearBob - EFFACE UN BOB (Icon anim?) de l'?cran
**
**   Quand on arr?te une animation, les derniers bobs affich?s restent
**   sur l'?cran. Cette routine sert ? nettoyer l'?cran des derniers
**   bobs affich?s.
**
**   Fonctionne UNIQUEMENT pour les BOBS, c'est ? dire les Icons avec
**   un buffer de fond (icon->p_bgbuf ).
**
**   Faire ca AVANT de lib?rer le buffer de fond associ? au Bob !!!
**
************************************************************************/
static  void    ClearBob( struct Icon_s *icon )
{
    /* restaurer le fond d'?cran */
    if( (icon->p_bgbuf != NULL) && (icon->xold != -1) )
        //CopyBlock(icon->p_bgbuf,VID->pScreen()+icon->xold+(icon->yold*SCR_WIDTH),icon->oldw,icon->oldh,icon->oldw,SCR_WIDTH);
        CopyBlock(icon->p_bgbuf,0,0,icon->oldw,icon->oldh,icon->oldw, VID->pScreen(),icon->xold,icon->yold,SCREEN_WIDTH);
}


// =============================================================================
//
//	main game code here
//
// =============================================================================

#define GAMETICTIME		(1000/60)		// time between each game frame and input, milliseconds
static	ULONG	lastTime;


// because of the window loop I have to break the original Dos routine in
// several states so that these routines always return , and don't wait in there
typedef enum {
	gs_titlepage,
	gs_titlepage_loop,
	gs_options,
	gs_options_loop,
	gs_oneplayer,
	gs_oneplayer_fadein,
	gs_oneplayer_loop,
	gs_oneplayer_end,
	gs_twoplayers,
	gs_twoplayers_fadein,
	gs_twoplayers_loop,
	gs_twoplayers_fadeout,
} gamestate_t;

static	gamestate_t	GameState;



static  void    Dummie(void)
{
    Aff_Tuile( 0,0, '!' );
}

/*
static  void    Waitkey()
{
    //Scr_put();
    //kb_clear();
    while( !kb_keydown( KEY_ENTER ) )
        Vbl_wait();
    while( kb_keydown( KEY_ENTER ))
        Vbl_wait();
}*/


/*
**  Affiche NEXT gellule( given coordinates )
**
*************************************************/
static  void    Aff_Next(void)
{
static  UWORD   tuile,xpos,ypos;
   //Shared NX1,NX2

    xpos = Pl->XNext;
    ypos = Pl->YNext;

    tuile = (Pl->Nx1 - 1) * 4 + GELLULES + 3;
    Aff_Tuile( xpos, ypos, tuile );
    tuile = (Pl->Nx2 - 1) * 4 + GELLULES + 4;
    xpos += TUILEW;
    Aff_Tuile( xpos, ypos, tuile );
}



/*
**  Affiche SPEED (soit "LOW", "MED" ou "HI")
**
*************************************************/
static  void    Aff_Speed( UWORD xpos, UWORD ypos )
{
static  char   *speedo[4] = { NULL, "low", "med", "[hi" };

    Aff_Zone( xpos, ypos, speedo[ Pl->Speed ] );
}



/**
**   LEVEL SUIVANT : Initialisations, affichage Level, Speed, ...
**
*****************************************************************/

#define SPEED_LOW   48
#define SPEED_MED   24
#define SPEED_HI    12

static  void    Init_Level(void)
{
    /* initialise la ronde des virus */
    VirusMsg[0] = VIRUSMARCHE;
    VirusMsg[1] = VIRUSMARCHE;
    VirusMsg[2] = VIRUSMARCHE;

    /* r?initialise la vitesse de chute des gellules */

    Pl->SpeedTim[1] = SPEED_LOW;
    Pl->SpeedTim[2] = SPEED_MED;
    Pl->SpeedTim[3] = SPEED_HI;

    /* D?termine la 1?re gellule
       (on la voit pendant le remplissage de la bouteille. */

    Pl->Nx1 = (rand() % 3) +1;
    Pl->Nx2 = (rand() % 3) +1;      // couleurs de 1 ? 3

    /* Affiche la 1?re fois Niveau, Speed, Next Gellule ... */

    Aff_Next();
    Aff_Digits( 2, Pl->Level, Pl->XLevel, Pl->YLevel );
    Aff_Speed( Pl->XSpeed, Pl->YSpeed );

    if( NPlayers == 1 ){
       Aff_Digits( 7, Score, 20, 63);
       Aff_Digits( 7, TopScore, 20, 42);
    }

    /* Vider le tableau pour le prochain niveau (qui va se jouer) */

    memset( (*Bottle), 0, 9*17 );

    /* Pr?parer le Remplissage de la Bouteille */

    Pl->NVirus = 0;
    Pl->TotVirus[0] = 0;
    Pl->TotVirus[1] = 0;
    Pl->TotVirus[2] = 0;        /* nb de Virus de chaque couleur      */

    Pl->Coul = 0;               /* couleur initiale remplissage virus */

    Clear_Bottle();             /* vide la bouteille A L'ECRAN        */

    Pl->NCrasses = 0;

    Pl->Step = Remplit_Bouteille;
}

/**
**   REMPLISSAGE DE LA BOUTEILLE AVEC DES VIRUS
**
**   -> un appel ajoute un Virus dans la bouteille, jusque... cf cond.
**
*****************************************************************/
static  void    Remplit_Bouteille(void)
{
static  SWORD    dy,x,y;

    do{
        x = (rand() & 7)+1;
        y = 16-(rand() % 11);
    }while( (*Bottle)[x][y] != 0 );

        Pl->NVirus++;

        /* couleur du virus */
        if( ++Pl->Coul > 3 )
            Pl->Coul = 1;

        Pl->TotVirus[Pl->Coul-1]++;    /* tot de virus pour chaque couleur */

        (*Bottle)[x][y] = Pl->Coul + VIRUS;

        /* afficher le virus dans ce buffer */
        x *= TUILEW;
        x += Pl->BottleX1;
        y *= TUILEH;
        y += Pl->BottleY1 - TUILEH;

        /* affiche le virus dans la bouteille et le total actuel */
        Aff_Tuile( x, y, Pl->Coul + GELLULES + 21 );

        Aff_Digits( 2, Pl->NVirus, Pl->XVirus, Pl->YVirus );

    if( Pl->NVirus >= (Pl->Level+1)*4 )
    {
        /* pr?pare NEXT STEP. */

        Pl->Nth  = 0;               //compteur de gellules pour acc?l?rer
                                   // la vitesse apr?s 10
        Pl->Step = Next_Gellule;
    }
}

/**
**   GELLULE SUIVANTE
**
**
*****************************************************************/
static  void    Next_Gellule(void)
{
/* BOUCLE PRINCIPALE D'UN NIVEAU : SE REPETE JUSQU'A CE QUE LE GOULOT SOIT  */
/* OCCUPE  (GAME OVER) ,... OU NVIRUS=0 (TOUS LES VIRUS SONT DETRUITS)      */

    /* Acc?l?re la vitesse de chute des gellules apr?s 10 gellules */
    if( Pl->Nth != -1 )
    {
        if( ++(Pl->Nth) >= 10 )
        {
            Pl->SpeedTim[ Pl->Speed ] -= (4-(Pl->Speed));
            Pl->Nth = -1;
        }
    }
    Aff_Digits(2,Pl->NVirus,Pl->XVirus,Pl->YVirus);

    /* si le goulot est occup?... GAME OVER! */
    if( (*Bottle)[4][1] || (*Bottle)[5][1] )
    {
        if( NPlayers == 1 )
            Pl->Step = GameOver_1pl;
        else
            Pl->Step = GameOver_2pl;
        return;
    }

    /* si BLOK = 0, la gellule n'est pas encore bloqu?e (pos?e) */
    Pl->Bloque = false;

    /* nombre de colonnes/rang?es d?truites avec cette gellule */
    Pl->Super = 0;

    /* gellule start pos. pour la chute */
    GX = 4;
    GY = 0;

    /* mini-tableau de la gellule
    *  ge1?ge2
    *   * * *
    *   **?**
    *   * * *
    *  ge3?ge4   <- au d?part la gellule est couch?e.
    */

    GE[1] = 0;
    GE[2] = 0;
    GE[3] = Pl->Nx1;
    GE[4] = Pl->Nx2;

    Pl->Nx1 = (rand() % 3) +1;
    Pl->Nx2 = (rand() % 3) +1;      //couleurs de la procha?ne gellule

    /* compteur pour la chute (nb de vbls entre chaque descente d'une case)*/
    Pl->Count = Pl->SpeedTim[ Pl->Speed ];
    Pl->YWait = 0;           /* compteur de chute acc?l?r?e */

    /* flags indiquant si on reste appuy? dans une direction.*/
    Pl->YPress = false;
    Pl->XPress = false;

    if( NPlayers == 1 )
    {
        /* Pr?pare Lance_Gellule */
        AnimIcon( &Bras_lance, "ABC\1", BRASLANCE, 4, 234, 47, COPYPUT );
        Anim_lance_cpt = 0;
        Anim_lance_pos = -1;
        /* efface Next gellule pr?c?dent */
        FillRect( 240,40, 240+(TUILEW*2)-1, 40+TUILEH-1, 0 );
        Lance_Gellule();
        Pl->Step = Lance_Gellule;
        return;
    }
    else
        Pl->Step = Pose_Gellule_Main_Loop;

    /* affiche la procha?ne gellule */
    Aff_Next();

    /* place la gellule horizontalement ? sa position de d?part (goulot)*/
    Draw_Gellule_Horiz();
}

/* DrMario lance la gellule dans le goulot de la bouteille.           */
/* Animation de son bras(3 images), et de la gellule qui tourne sur   */
/* elle-m?me en une s?rie de 26 images.                               */
/* Initialiser Anim_lance_cpt = 0;                                    */
/*             Anim_lance_pos = -1; pour lancer l'animation           */

static  void   Lance_Gellule()
{
/* coords de la gellule lanc?e vers le goulot de la bouteille */
static  UBYTE   coords[2*26]={ 240,40, 240,27, 240,27, 240,20, 230,20, 230,13,
 220,13, 220,6, 210,7, 210,0, 200,7, 200,0, 190,7, 190,0, 180,7, 180,0, 170,7,
 170,7, 160,14, 160,14, 160,21, 150,28, 150,35, 150,42, 150,49, 150,56 };

/* 1 indique que la gellule tourne(AVANT affichage) , 0 elle tourne pas */
static  UBYTE   ori[26]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0};

static  UWORD   pos,xpos,ypos;
static  UWORD   oldx,oldy;      /* last pos. of gellule on-screen */

    if( Anim_lance_pos >= 0 )  /* si on a d?j? sauv? le fond... */
        /* restaurer le fond */
        //Copy_block(p_lance_bg,(VID->pScreen()+oldx+(oldy * SCR_WIDTH)),TUILEW*2,TUILEH*2,TUILEW*2,SCR_WIDTH);
		CopyBlock(p_lance_bg,0,0,TUILEW*2,TUILEH*2,TUILEW*2, VID->pScreen(),oldx,oldy,SCREEN_WIDTH);

    if( --Anim_lance_cpt < 0 )
    {
        Anim_lance_cpt = 0;
        if( ++Anim_lance_pos > 25 )
        {
            Pl->Step = Pose_Gellule_Main_Loop;
            /* maintenant Mario rel?ve le Bras et tient la procha?ne gellule */
            PasteIcon( 234,47, BRASLANCE, COPYPUT );
            Aff_Next();
            Draw_Gellule_Horiz();
            return;
        }
        else   /*la gellule tourne sur elle-m?me ? la plupart des positions*/
        {
            if( ori[Anim_lance_pos] == 1 )   /* la gellule doit tourner */
            {
                if( GE[1] == 0 )
                {
                    GE[1] = GE[4];
                    GE[4] = 0;
                }
                else
                {
                    GE[4] = GE[3];
                    GE[3] = GE[1];
                    GE[1] = 0;
                }
            }
        }
    }

    pos  = Anim_lance_pos<<1;
    xpos = coords[pos];
    ypos = coords[pos+1];

    /* sauver le fond */
    //Copy_block((VID->pScreen()+xpos+(ypos*SCR_WIDTH)),p_lance_bg,TUILEW*2,TUILEH*2,SCR_WIDTH,TUILEW*2);
	CopyBlock(VID->pScreen(),xpos,ypos,TUILEW*2,TUILEH*2,SCREEN_WIDTH, p_lance_bg,0,0,TUILEW*2);
    oldx = xpos;
    oldy = ypos;

    if( GE[1] == 0 )       /* gellule couch?e */
    {
        Aff_Tuile_Transp( xpos, ypos, ((GE[3]-1)<<2)+3+TRGELLULES );
        xpos += TUILEW;
        Aff_Tuile_Transp( xpos, ypos, ((GE[4]-1)<<2)+4+TRGELLULES );
    }
    else                    /* gellule debout  */
    {
        Aff_Tuile_Transp( xpos, ypos, ((GE[1]-1)<<2)+1+TRGELLULES );
        ypos += TUILEH;
        Aff_Tuile_Transp( xpos, ypos, ((GE[3]-1)<<2)+2+TRGELLULES );
    }

    RefreshIcon( &Bras_lance );

    Anime_virus();
}

static  void    Pose_Gellule_Main_Loop()
{
/*                                                                         */
/*=========================< MAIN_LOOP: POSE_GELLULE >=====================*/
/*                                                                         */
	SWORD    n,o,x;
	UBYTE   c;

    Deplace_Gellule();
    Anime_virus();

    /* jusque quand la gellule est POSEE ou que ESCAPE est appuy? */
    if( Pl->Bloque )
    {
        Pl->Step  = Boucle_Destruction;

        /* CRASSES! La gellule est pos?e, AVANT que la procha?ne ne vienne */
        /* s'il y a des crasses de la part du concurrent, on les place en  */
        /* haut de la bouteille, juste avant les chutes en s?rie           */

        if( Pl->NCrasses > 1 )
        {
            n = Pl->NCrasses;
            o = 0;
            for( x=1; x<9; x++ )
            {
                if( (*Bottle)[x][1] == 0 )   /* v?rifie qu'il y a assez de place */
                    o++;                     /* pour toutes les crasses, sinon   */
            }                                /* place ce qu'on peut.             */
            if( o<n )   n = o;

            o = 0;
            while( n-- > 0 )
            {
                do{
                    x = (rand() & 7)+1;
                }while( (*Bottle)[x][1] != 0 );   /* cherche place libre */

                (*Bottle)[x][1] = c = Pl->Crasses[o++];
                Aff_Case( x, 1, GELLULES+11+(c<<1) );
            }

            Pl->NCrasses = 0;
            Pl->ChuteCrasse = true;   /* signale de faire tomber les crasses */
        }
        else
            Pl->ChuteCrasse = false;    /* pas de crasses ? laisser descendre */

        /* FIN PLACE CRASSES ***********************************************/

        return;
    }
}

/*==========================================================================*/
/*                                                                          */
/* Ici On Detruit Les Cases Si La Gellule A Aligne Plus De 3 Cases De Meme  */
/* Couleur Dans La Bouteille. Si Des Morceaux D'anciennes Gellules Tombent  */
/* Suite Aux Espaces Nouvellement Crees, On Continue A Tester Si D'autres   */
/* Cases Se Sont Alignees, Ainsi De Suite Jusqu'a Ce Que Plus Rien Ne Tombe,*/
/* Et Plus Aucune Suite De Couleurs N'est Possible.                         */
/*                                                                          */
/*==========================================================================*/
static  void    Boucle_Destruction()
{
auto    UWORD   points,n;

/************* Boucle de chute des morceaux dans la bouteille ***************/

    /* virus kill?s dans ce cycle de chute */
    Pl->VirusKill = 0;

    /* 1. comptage des couleurs align?es et explosions    */
    /*    si renvoye NO, aucune case n'a explos? (ni VIRUS, ni GELLULES) */
    if( !Seek_And_Destroy() && !Pl->ChuteCrasse )
    {
        /* LES CHUTES SUCCESSIVES SONT TERMINEES *************************/

        /* Son sp?cial quand on ?limine 2 rang?es/colonnes ou plus */
        if( Pl->Super > 1 )
        {
            PlaySound( SUPER_FX );

            /* copier les codes crasses et nbcrasses(=super) chez l'autre */
            if( NPlayers == 2 )
            {
               AutrePl = Pl->LAutre;
               AutrePl->NCrasses = Pl->Super;
               for( n=0; n < 4; n++ )
                  AutrePl->Crasses[n] = Pl->BetaCrasses[n];
            }
        }

        Pl->Step = Next_Gellule;
        return;
    }

    /* si il y a des crasses, elles vont tomber, c'est ok. */
    Pl->ChuteCrasse = false;

    /* si Pl->VirusKill > 0 des VIRUS ont explos?,
       sinon ce sont des GELLULES qui sont explos?es (de m?me couleur) */

    /* ---pas plus de 6virus ? la fois pour le score... */
    /* si VirusKill = 0 pas de score duschnok!          */
    if( Pl->VirusKill > 0 )
    {
       /*---soustraire les virus tu?s au total de virus et afficher */
       Pl->NVirus -= Pl->VirusKill;
       Aff_Digits( 2, Pl->NVirus, Pl->XVirus, Pl->YVirus );

       /* THIS PART ONLY FOR ONE PLAYER ====================================== */
       if( NPlayers == 1 )
       {
        /* la prime monte jusqu'? SIX virus, t'en fais plus, t'es lourd! */
        if( Pl->VirusKill > 6 )
            Pl->VirusKill = 6;

      //calcul donnant les valeurs de ce tableau (du manuel du jeu sur NES8bit):
      //  VIRUS DETRUITS  Vitesse: LOW  MED  HI
      //        1                  100  200  300
      //        2                  200  400  600
      //        3                  400  800 1200
      //        4                  800 1600 2400
      //        5                 1600 3200 4800
      //        6                 3200 6400 9600
      //---------------------------------------------(pointage officiel!)
        points = 50 * Pl->Speed;
        do{
            Pl->VirusKill--;
            points <<= 1;
        }while( Pl->VirusKill > 0 );

        Score += points;
        Aff_Digits( 7, Score, 20, 63 );

        if( Score > TopScore )
        {
            TopScore = Score;
            Aff_Digits( 7, TopScore, 20, 42);
        }
       }
      /* END OF ONE PLAYER PART FOR SCORES ================================== */

    }//Endif

  /* PARTOUT OU DES CASES ONT ETE DETRUITES ON A AFFICHE UNE PETITE IMAGE   */
  /* D'EXPLOSION. CELLE-CI DISPARAIT APRES 1/2 SECONDE ET LAISSE UN ESPACE  */
  /* VIDE.                                                                  */

    Pl->Explose = 29;
    Pl->Step =  Attente_Explosion;
}

  /* 2) attendre un moment, puis effacer les 'explosions' */
static  void    Attente_Explosion()
{
    if( --Pl->Explose <= 0 )
    {
        Efface_les_cases_detruites();

        if( Pl->NVirus < 1 )
        {
           if( NPlayers == 1 )
               Pl->Step = WinGame_1pl;    /* Plus de virus? T'as gagn?! */
           else
               Pl->Step = WinGame_2pl;
           return;
        }

        Pl->Step = Les_gellules_tombent;
    }
}

/*cette routine place des codes %1100 l? o? les cases devront ?tre effac?es */
/* cel? ?vite d'effacer une case plusieurs fois (en raison des tests)       */

  /*3) TESTER LA BOUTEILLE A LA RECHERCHE DES ANCIENNES GELLULES POUVANT */
  /*   TOMBER. (LES VIRUS NE TOMBENT JAMAIS!)                            */
  /* NB: cette routine s'?tale sur plusieurs cycles VBLs, le temps que   */
  /* tout ce qui tombe soit tomb? et bloqu? au fond de la bouteille.     */
static  void    Les_gellules_tombent()
{
static  SWORD   x,y;
static	UBYTE	n;
static  boolean falling;
static  UBYTE   code;

    falling = false;
    for( y=15; y>=1; y-- )
    {
        for( x=8; x>=1; x-- )
        {
            code = (*Bottle)[x][y];

            //si case vide passer ? la suivante
            if( code == 0  )  continue;
            if( code == 12 )  continue;

            //si demi-gellule...
            if( (code & 0x0F) < 4 )
            {
                //N = case du dessous (les bords de la bouteille  =  -1)
                n = (*Bottle)[x][y+1];
                if( (n != 0) && (n != 12) )
                    //en-dessous c'est pas VIDE : on passe ? la case suivante
                    continue;

                //place la case une en dessous (tombe)
                (*Bottle)[x][y+1] = code;
                n = code & 3;

                //affiche la 1/2 gellule ? sa nouvelle position (y+1)
                Aff_Case( x, y+1, GELLULES+(n<<1)+11 );
                falling = true;
                (*Bottle)[x][y] = 12;
                continue;
            }//Endif

            //si gellule compl?te...
            if( (code & COMPLETE) != 0 )
            {
                n = (*Bottle)[x][y+1];
                //si la case dessous n'est pas vide, elle ne peut pas tomber
                if( (n != 0) && (n != 12) )
                {
                    if( (code & 16) != 0 )
                        x--;
                    continue;
                }//Endif

                //si elle est horizontale ...
                if( (code & 16) != 0 )
                {
                    //---si en-dessous de l'autre moiti? n'est pas vide, elle tombe pas
                    n = (*Bottle)[x-1][y+1];
                    if( (n != 0) && (n != 12) )
                    {
                        x--;
                        continue;
                    }//Endif

                    //---horizontale, et elle tombe...
                    falling = true;
                    (*Bottle)[x][y+1] = code;
                    (*Bottle)[x][y]   = 12;
                    n = code & 3;
                    Aff_Case( x, y+1, GELLULES+(n<<2) );
                    x--;
                    n = (*Bottle)[x][y];
                    (*Bottle)[x][y+1] = n;
                    n &= 3;
                    Aff_Case( x, y+1, GELLULES+(n<<2)-1 );
                    (*Bottle)[x][y] = 12;
                    continue;
                }//Endif

                //si elle est verticale...
                if( (code & 16) == 0 )
                {
                //---si c'est une verticale au fond, elle tombe pas
                    if( y == 15 )
                    {
                       if( (code & 32) != 0 )
                           continue;
                    }
                    if( (code & 32) != 0 )
                        continue;
                    falling = true;
                    (*Bottle)[x][y+1] = code;
                    n = code & 3;
                    Aff_Case( x, y+1, GELLULES+(n<<2)-2 );
                    (*Bottle)[x][y] = (*Bottle)[x][y-1];
                    n = (*Bottle)[x][y-1] & 3;
                    (*Bottle)[x][y-1] = 12;
                    Aff_Case( x, y, GELLULES+(n<<2)-3 );
                    continue;
                }//Endif
            }//Endif
        }//Next X
    }//Next Y

    //--effacer les cases lib?r?es (%1100)
    Efface_les_cases_detruites();

    //rien n'a pu tomber, c'est fini.
    if( !falling )
    {
        Pl->Step = Boucle_Destruction;
        return;
    }
    else
        PlaySound( CHUTE_FX );

    /*--attendre un moment, puis boucler en haut pour tester si des cases
        peuvent encore descendre.                                        */
    Pl->Chute = 15;
    Pl->Step  = Attente_chutes;
}
/* attendre quelques vbls entre les chutes en s?ries dues aux explosions */
static  void    Attente_chutes()
{
    if( --Pl->Chute <= 0 )
    {
        Pl->Step = Les_gellules_tombent;
    }

    Anime_virus();
}

/*  **********************************************************************  */
/*  *           VARIABLES  POUR ANIMATIONS WIN/GAMEOVER 1PL & 2PL        *  */
/*  **********************************************************************  */
static struct Icon_s mariowins;     /* Mario fait 'gagn?!'                */
static struct Icon_s viruswins;     /* Virus qui tient le panneau 'PERDU!'*/
static struct Icon_s starty;        /* START clignotant                   */
static SWORD   yclearup,clrup_tim;   /* pr effacage ralenti du contenu de la bouteille */

/**** WIN GAME - STAGE TERMINE - MODE 1 PLAYER.
**
**
**
**
********************************************************************/

static void   WinGame_1pl()
{
    yclearup = 16;
    clrup_tim = 0;
    Pl->Step = Win1pl_clearup;
}
static void   Win1pl_clearup()
{
   UWORD  x;

    if( --clrup_tim > 0 )
        return;

    clrup_tim = 2;
    for( x=1; x<9; x++)
       Eff_Case( x, yclearup );       /* nettoye les gellules sur une ligne */

    if( --yclearup < 1 )
        Pl->Step = Win1pl_afterclearup;
}
static void   Win1pl_afterclearup()
{
    PlaySound( WIN_FX );

    if( ++Pl->Level >= 21 )
    {
        Aff_Zone( 20, 147, "TAS FINI LE JEU!!!\2MEGA COOOL EUU!" );
        Pl->Step = Dummie;
        return;
    }

    /* efface DrMario dans la bo?te en haut ? droite */
    FillRect( 226, 41, 282, 81, 0 );

    /* affiche le panneau 'STAGE CLEAR   TRY NEXT'   */
    PasteIcon( Pl->BottleX1+12, Pl->BottleY1+2, WINPANEL, TRANSPUT );

    AnimIcon( &mariowins, "AB", MARIOWI1, 30, 144, 117, TRANSPUT );
    mariowins.p_bgbuf = ReserveBackground( BOBSIZE(MARIOWI1) );

    AnimIcon( &starty, "A-", START, 30, Pl->BottleX1+27, Pl->BottleY1+103, TRANSPUT );
    starty.p_bgbuf = ReserveBackground( BOBSIZE(START) );

    Pl->Step = Win_boucle_1pl;
}
static  void    Win_boucle_1pl()
{
    /* anime le bras de la victoire de Mario */
    RefreshIcon( &mariowins );

    /* fait clignoter START! au bas de la bouteille */
    RefreshIcon( &starty );

    if( KEY_DOWN(Pl->P_ButtonA) )
    {
        Clear_Bottle();
		CopyBlock (p_marioinzebox,0,0,69,51,69, VID->pScreen(),220,36,SCREEN_WIDTH);
        //Copy_block(p_marioinzebox,VID->pScreen()+220+(36*SCR_WIDTH),69,51,69,SCR_WIDTH);
        Pl->Step = Init_Level;
        FreeBackground( starty.p_bgbuf );
        FreeBackground( mariowins.p_bgbuf );
    }
}

/****** GAME OVER - 1 PLAYER - NUL-NUL-NUUUUL!
**
**
**
********************************************************************/
static  void    GameOver_1pl(void)
{
    yclearup = 16;
    clrup_tim = 0;
    Pl->Step = GameOver1pl_clearup;
}
static void   GameOver1pl_clearup()
{
auto   UWORD  x;

    if( --clrup_tim > 0 )
        return;

    clrup_tim = 2;
    for( x=1; x<9; x++)
       Eff_Case( x, yclearup );       /* nettoye les gellules sur une ligne */

    if( --yclearup < 1 )
        Pl->Step = GameOver1pl_afterclearup;
}
static void   GameOver1pl_afterclearup()
{
    PlaySound( GAMEOVER_FX );

    if( ++Pl->Level >= 21 )
    {
        Aff_Zone( 20, 147, "TAS FINI LE JEU!!!\2MEGA COOOL EUU!" );
        Pl->Step = Dummie;
        return;
    }

    /* les virus sous la loupe font des grimaces */
    VirusMsg[0] = VIRUSNIARK;
    VirusMsg[1] = VIRUSNIARK;
    VirusMsg[2] = VIRUSNIARK;   /* aucun effet en 2 players */

    /* efface DrMario dans la bo?te en haut ? droite */
    FillRect( 226, 41, 282, 81, 0 );

    /* affiche le panneau 'GAME OVER'   */
    PasteIcon( Pl->BottleX1+12, Pl->BottleY1+2, LOSPANEL, TRANSPUT );

    /* affiche Mario qui a l'air d?cu */
    PasteIcon( 139, 117, MARIOLOS, TRANSPUT );

    AnimIcon( &starty, "A-", START, 30, Pl->BottleX1+27, Pl->BottleY1+103, TRANSPUT );
    starty.p_bgbuf = ReserveBackground( BOBSIZE(START) );

    Pl->Step = GameOver_boucle_1pl;
}
static  void    GameOver_boucle_1pl()
{
    /* fait clignoter START! au bas de la bouteille */
    RefreshIcon( &starty );

    if( KEY_DOWN(Pl->P_ButtonA) )
    {
        Clear_Bottle();
        //Copy_block(p_marioinzebox,VID->pScreen()+220+(36*SCR_WIDTH),69,51,69,SCR_WIDTH);
		CopyBlock (p_marioinzebox,0,0,69,51,69, VID->pScreen(),220,36,SCREEN_WIDTH);

        /* force retour ? TitlePage */
        EscapeKey = true;
        FreeBackground( starty.p_bgbuf );
    }
}


/***** YOU WIN! - STAGE TERMINE - UNE COURONNE! - MODE 2 PLAYERS.
**
**   Le premier joueur qui passe ici redirige l'autre sur l'attente.
**
**   Le joueur qui arrive ici est le gagnant: il recoit une couronne,
**   on voit DrMario qui l?ve le bras de la victoire, et on attends
**   que notre gagnant appuie une touche pour faire passer les deux
**   concurrents au level suivant.
**
********************************************************************/
static  void    WinGame_2pl()
{
    /* trouve les donn?es de l'autre joueur, et nettoye les explosions */
    /* ?ventuellement en cours chez l'autre joueur.                    */
    AutrePl = Pl->LAutre;

    if( Pl->PlayerNum == 1 )
    {
        Pl = AutrePl;
        Bottle = &Bottle2;
        Efface_les_cases_detruites();   /* c'est CON mais c'est SIMPLE */
        Bottle = &Bottle1;
        Pl = &Pl1;
    }
    else
    {
        Pl = AutrePl;
        Bottle = &Bottle1;
        Efface_les_cases_detruites();
        Bottle = &Bottle2;
        Pl = &Pl2;
    }

    /* efface les NEXT gellules */
    FillRect( Pl->XNext, Pl->YNext, Pl->XNext+(2*TUILEW)-1, Pl->YNext+TUILEH-1, BOTTLEBGCOL );
    FillRect( AutrePl->XNext, AutrePl->YNext, AutrePl->XNext+(2*TUILEW)-1, AutrePl->YNext+TUILEH-1, BOTTLEBGCOL );

    /* et une couronne, une! (affichage plus loin) */
    if( ++Pl->Wins > 2 )
    {
        /* animation du gagnant des 3 matches */
        //Vbl_wait();
    }

    yclearup = 16;
    clrup_tim = 0;

    /* redirige l'autre sur l'attente */
    AutrePl->Step = Attends_et_fermela;

    Pl->Step = Win2pl_clearup;
}
static void   Win2pl_clearup()
{
   UWORD  x;
   UBYTE  code;

    if( --clrup_tim > 0 )
        return;

    clrup_tim = 2;
    for( x=1; x<9; x++)
    {
       code = (*Bottle)[x][yclearup];

       (*Bottle)[x][yclearup] = 0;     /*ne pas animer les virus qui ont   */
                                       /* ?t? effac?s de la bouteille      */
       Eff_Case( x, yclearup );        /*nettoye les gellules sur une ligne*/

       /* si on coupe une gellule VERTICALE en 2, afficher une demi-gellule */
       if( (code & (COMPLETE+48)) == COMPLETE )
       {
           code = (*Bottle)[x][yclearup-1] & 3;
           (*Bottle)[x][yclearup-1] = code;
           Aff_Case( x, yclearup-1, GELLULES+11+(code<<1) );
       }
    }

    if( --yclearup < 9 )
        Pl->Step = Win2pl_afterclearup;
}
static void   Win2pl_afterclearup()
{
    PlaySound( WIN_FX );

    /* affiche la couronne du gagnant */
    PasteIcon( Pl->XWin, Pl->YWin+(3-Pl->Wins)*14, COURONNE, COPYPUT );

    /* mario l?ve le bras de la victoire */
    AnimIcon( &mariowins, "AB", MARIOWI1, 30, Pl->BottleX1+35, Pl->BottleY1+75, TRANSPUT );
    mariowins.p_bgbuf = ReserveBackground( BOBSIZE(MARIOWI1) );

    /* start clignote en bas entre les deux bouteilles */
    AnimIcon( &starty, "A-", START, 30, 136, 184, TRANSPUT );
    starty.p_bgbuf = ReserveBackground( BOBSIZE(START) );

    Pl->Step = Win_boucle_2pl;
}
static  void    Win_boucle_2pl()
{
    /* anime le bras de la victoire de Mario */
    RefreshIcon( &mariowins );

    /* fait clignoter START */
    RefreshIcon( &starty );

    if( !KEY_DOWN(Pl->P_ButtonA) )    /* si pas appuy?, return */
        return;

    /* APPUYE LA TOUCHE! on passe au niveau suivant, match suivant */

    ClearBob( &starty );
    ClearBob( &mariowins );

    FreeBackground( starty.p_bgbuf );
    FreeBackground( mariowins.p_bgbuf );

    if( ++Pl->Level >= 21 )
        Pl->Level = 21;

    Pl->Step = Init_Level;

    /* match suivant, niveau suivant, pour l'AUTRE JOUEUR */
    if( ++AutrePl->Level >= 21 )
        AutrePl->Level = 21;

    AutrePl->Step = Init_Level;
}

#define PANNEAUX1 30     /* coord du panneau de la d?faite par rapport */
#define PANNEAUY1 62     /* ? Pl->BottleX1-Y1                          */

/****** GAME OVER - 2 PLAYERS - L'AUT'CHANCEUX GAGNE UNE COURONNE.
**
**   Le premier joueur qui passe ici redirige l'autre sur l'attente.
**
**   Le joueur qui arrive ici est le perdant: l'autre gagne une couronne,
**   un virus fait des grimaces et tiens un panneau avec une croix
**   rouge (perdu!), le perdant doit appuyer sur une touche pour
**   faire avancer les deux concurrents au match suivant, level
**   suivant (apr?s level21 on continue sur level21..)
**
********************************************************************/
static  void  GameOver_2pl(void)
{
    AutrePl = Pl->LAutre;

    if( Pl->PlayerNum == 1 )
    {
        Pl = AutrePl;
        Bottle = &Bottle2;
        Efface_les_cases_detruites();   /* c'est CON mais c'est SIMPLE */
        Bottle = &Bottle1;
        Pl = &Pl1;
    }
    else
    {
        Pl = AutrePl;
        Bottle = &Bottle1;
        Efface_les_cases_detruites();
        Bottle = &Bottle2;
        Pl = &Pl2;
    }

    /* efface les NEXT gellules */
    FillRect( Pl->XNext, Pl->YNext, Pl->XNext+(2*TUILEW)-1, Pl->YNext+TUILEH-1, BOTTLEBGCOL );
    FillRect( AutrePl->XNext, AutrePl->YNext, AutrePl->XNext+(2*TUILEW)-1, AutrePl->YNext+TUILEH-1, BOTTLEBGCOL );

    /* et une couronne, une! */
    if( ++AutrePl->Wins > 2 )
    {
        /* animation du gagnant des 3 matches (l'autre joueur!) */
        //Vbl_wait();
    }

    yclearup = 16;
    clrup_tim = 0;

    /* redirige l'autre sur l'attente */
    AutrePl->Step = Attends_et_fermela;

    Pl->Step = GameOver2pl_clearup;
}
static void   GameOver2pl_clearup()
{
auto   UWORD  x;

auto   UBYTE  code;

    if( --clrup_tim > 0 )
        return;

    clrup_tim = 2;
    for( x=1; x<9; x++)
    {
       code = (*Bottle)[x][yclearup];

       (*Bottle)[x][yclearup] = 0;     /*ne pas animer les virus qui ont   */
                                       /* ?t? effac?s de la bouteille      */
       Eff_Case( x, yclearup );        /*nettoye les gellules sur une ligne*/

       /* si on coupe une gellule VERTICALE en 2, afficher une demi-gellule */
       if( (code & (COMPLETE+48)) == COMPLETE )     /* moiti? inf?rieure */
       {
           code = (*Bottle)[x][yclearup-1] & 3;
           (*Bottle)[x][yclearup-1] = code;
           Aff_Case( x, yclearup-1, GELLULES+11+(code<<1) );
       }
    }

    Anime_virus();

    if( --yclearup < 9 )
        Pl->Step = GameOver2pl_afterclearup;
}
static void   GameOver2pl_afterclearup()
{
auto UWORD viruscol;
auto UBYTE *p_case;

    /* musique perdu */
    PlaySound( GAMEOVER_FX );

    /* affiche la couronne du vainqueur */
    PasteIcon( AutrePl->XWin, AutrePl->YWin+(3-AutrePl->Wins)*14, COURONNE, COPYPUT );

    /* cherche une couleur de virus pr?sent dans la bouteille (0-2) */
    p_case = (UBYTE *) (*Bottle);
    while( (viruscol = *(p_case++)&7) < 4 )
        ;
    viruscol = (viruscol & 3)-1;
    if(viruscol>2) viruscol = 2;        /* au cas o?... */

    /* affiche le panneau de la d?faite et le bras du virus qui le tient */
    PasteIcon( Pl->BottleX1+PANNEAUX1, Pl->BottleY1+PANNEAUY1, PANNEAU, COPYPUT );
    PasteIcon( Pl->BottleX1+PANNEAUX1+16, Pl->BottleY1+PANNEAUY1+27, VIRUSBRAS+viruscol, TRANSPUT );

    /* un virus fait des grimaces */
    AnimIcon( &viruswins, "AB", 7+viruscol*9, 30, Pl->BottleX1+PANNEAUX1+22, Pl->BottleY1+PANNEAUY1+28, VIRUSPUT );
    viruswins.p_bgbuf = ReserveBackground( VIRUSBOBSIZE );

    /* start clignote en bas entre les deux bouteilles */
    AnimIcon( &starty, "A-", START, 30, 136, 184, TRANSPUT );
    starty.p_bgbuf = ReserveBackground( BOBSIZE(START) );

    Pl->Step = GameOver_boucle_2pl;
}
static  void    GameOver_boucle_2pl()
{
    /* anime les virus restant dans la bouteille */
    Anime_virus();

    /* anime le virus moqueur */
    RefreshIcon( &viruswins );

    /* fait clignoter START */
    RefreshIcon( &starty );

    if( !KEY_DOWN(Pl->P_ButtonA) )    /* si pas appuy?, return */
        return;

    /* APPUYE LA TOUCHE! on passe au niveau suivant, match suivant */

    ClearBob( &starty );
    ClearBob( &viruswins );

    FreeBackground( starty.p_bgbuf );
    FreeBackground( viruswins.p_bgbuf );

    if( ++Pl->Level >= 21 )
        Pl->Level = 21;

    Pl->Step = Init_Level;

    /* match suivant, niveau suivant, pour l'AUTRE JOUEUR */
    if( ++AutrePl->Level >= 21 )
        AutrePl->Level = 21;

    AutrePl->Step = Init_Level;
}

/**** ROUTINE DU PLAYER QUI ATTENDS - MODE 2 PLAYERS.
**
**    Quand un joueur gagne ou perds, il termine le stage, et l'autre
**    joueur est interrompu, en attendant que le 1er appuye une
**    touche pour passer au level suivant.
**
*****************************************************************/
static  void  Attends_et_fermela(void)
{
    /* les virus qui restaient dans la bouteille continuent de bouger! */
    Anime_virus();
}



/**** Efface les cases d?truites.
**
**    On efface les cases d?truites suite aux explosions.
**    Ces cases ont ?t? marqu?es du code binaire 1100 ce qui indique
**    une combinaison unique et impossible: GELLULE COMPLETE + VIRUS.
**
*********************************************************************/
static  void    Efface_les_cases_detruites()
{
static  UWORD   x,y;
static  UBYTE   code;

    for( x=1; x<=8; x++ )
    {
        for( y=1; y<=16; y++ )
        {
            code = (*Bottle)[x][y];
            if( (code & 12) == 12 )     /* une case qui a explos? */
            {
                Eff_Case( x, y );
                (*Bottle)[x][y] = 0;
            }
        }//Next Y
    }//Next X
}


static  void    Deplace_Gellule()
{
static  SWORD    dxjoy,dyjoy,btjoy;

/*--- Chute de la gellule AUTOMATIQUE, SAUF si on l'ACCELERE deja au Joystick */
   if( !Pl->YPress )
   {
       if( --Pl->Count == 0 )
           Gellule_Go_Down();
   }

/*si elle est bloquee (posee), on ne peut plus la bouger (place ? la suivante!)*/
   if( Pl->Bloque )
       goto draw_bye;

/*--- d?placement par le joueur. */

  /* si le Bouton A est press?, BTJOY vaut >0                   */
  /* si le Bouton B est press?, BTJOY vaut -1 (donc <0) */
  /* sinon BTJOY vaut 0.                                                                */

    dxjoy = 0;
    dyjoy = 0;
    btjoy = 0;

    if( KEY_DOWN( Pl->P_Down  ))  dyjoy = 2;
    if( KEY_DOWN( Pl->P_Right ))  dxjoy = 8;
    if( KEY_DOWN( Pl->P_Left  ))  dxjoy = 4;
    if( KEY_DOWN( Pl->P_ButtonA)) btjoy = 0x10;
    if( KEY_DOWN( Pl->P_ButtonB)) btjoy = -1;

/*--si pas de deplacement gauche ou droite alors phase 2 */

    if( dyjoy == 0 )  Pl->YPress = false;

 /* on peut ACCELERER la chute de la GELLULE en appuyant vers le BAS */
    if( dyjoy > 0 )   Push_Down();
    if( Pl->Bloque )
        goto draw_bye;  /* arh arh! j'ai trouv? le bug cach?!!! */

 /* tourner la gellule dans le SENS HORLOGE?   */
    if( btjoy == 0 )
        Pl->BtPress = 0;
    else if( btjoy > 0 )
        Turn_Clockwise(dxjoy);
    else
        Turn_AntiClockwise();

    if( dxjoy == 0 )
        Pl->XPress = 0;
    else if( dxjoy == 4 )
        A_Gauche();
    else
        A_Droite();

draw_bye:
    Final_Draw_Gellule();
    return;
}

static  void  A_Droite(void)
{
/* ====================== SUB MOVE_GELLULE_DROITE ========================== */
 /* ne pas aller trop vite sur le c?t? si on reste appuy?. */
    if( Pl->XPress == 1 )
    {
        if( --Pl->XWait <= 0 )
            Pl->XWait = 10;
        else
            return;
    }else{
        Pl->XPress = 1;
        Pl->XWait  = 10;
    }

    /* ralentit un peu la chute quand on bouge lat?ralement           */
    /* dans le but d'am?liorer la jouabilit? quand on pose la gellule */
    Pl->Count += 5;

    if( GE[4] != 0 )
    {
    /*---gellule horizontale, poussee a droite */

        /*---si contre bord droit alors bloque -> descend   */
        if( GX == 7 ) return;

        /*--si obstacle alors bloquee, fin */
        if( (*Bottle)[GX+2][GY+1] != 0 ) return;
    }
    else
    {
    /*---si vert et bord droit alors bloque ->descend */
        if( GX == 8 ) return;

        /*---tester les 2cases a cote, si obstacle, bloque ,fin */
        if( (*Bottle)[GX+1][GY]   != 0 ) return;
        if( (*Bottle)[GX+1][GY+1] != 0 ) return;
        if( GY > 0 )
            Eff_Case( GX, GY );
    }
    Eff_Case(GX,GY+1);
    GX++;

    PlaySound( BIP_FX );
}

/*                                                                                                                                                    */
/*---ici la gellule est poussee a gauche                                                                          */
/*                                                                                                                                                        */
static  void  A_Gauche(void)
{
    if( Pl->XPress == 2 )
    {
        if( --Pl->XWait <= 0 )
            Pl->XWait = 10;
        else
            return;
    }else{
        Pl->XPress = 2;
        Pl->XWait  = 10;
    }

    /* ralentit un peu la chute quand on bouge lat?ralement           */
    /* dans le but d'am?liorer la jouabilit? quand on pose la gellule */
    Pl->Count += 5;

    /* si contre le bord  gauche, bloque, fin */
    if(GX == 1)  return;

    /* --si obstacle en bas a gauche, bloque, fin  */
    if( (*Bottle)[GX-1][GY+1] != 0 )  return;

    if(GE[1] == 0)
    {
        /* --gellule HORIZ ,poussee ? gauche <-- pas d'obstacle */
        Eff_Case( GX+1, GY+1 );
    }else{
        /* ---tester l'autre case a gauche(celle du haut), si obstacle, bloque, fin  */
        if( (*Bottle)[GX-1][GY] != 0 )  return;
        if( GY > 0 )
            Eff_Case(GX,GY);
        Eff_Case( GX, GY+1 );
    }
    GX--;

    PlaySound( BIP_FX );
}

/* ========================== SUB TURN CLOCKWISE =========================== */
/* Rotation de la gellule dans le sens Horloger.                                                        */
/*                                                                                                                                                      */
static  void  Turn_Clockwise( SWORD dxjoy )
{

    /* ne pas tourner trop vite! */
    if( Pl->BtPress == 1 )      /* 1 pour le bouton A. */
    {
        if( --Pl->BtWait <= 0 )
            Pl->BtWait = 10;
        else
            return;
    }else{
        Pl->BtPress = 1;        /* signal bouton A enfonc?. */
        Pl->BtWait  = 10;
    }

    /* ralentit un peu la chute quand on tourne la gellule */
    Pl->Count += 5;

    PlaySound( BIP_FX );

    if(GE[1] == 0)  goto  horizontale;

    /*--- elle est verticale, et il faut la coucher                 */
    /*--- elle est collee sur la paroi droite de la bouteille,      */
    /*    peut-elle reculer en se couchant?                         */
    if(GX == 8)  goto  emboite;

    /* si quelque chose la bloque ? droite... */
    if( (*Bottle)[GX+1][GY+1] != 0 )  goto  emboite;

    /* ---ok, elle est pas bloquee, tournons-la */
    GE[4] = GE[1];
    GE[1] = 0;
    if(GY>0)
        Eff_Case(GX,GY);
    return;

/* ---la gellule bloquee a droite peut-elle reculer? */
emboite:
    /* elle est dans une impasse, bloquee ? droite, */
    /*   et contre le bord gauche, fin              */
    if(GX == 1)  return;

    /*---si bloquee a gauche, fin (elle est coincee ? gauche et ? droite!) */
    if( (*Bottle)[GX-1][GY+1] != 0 )  return;

    /*--ok, elle peut se tourner...  */
    GE[4] = GE[1];
    GE[1] = 0;
    if(GY>0)
        Eff_Case(GX,GY);
    /*---et reculer! */
    GX--;
    return;

/*--- la gellule est couch?e, elle se met en verticale en tournant */
horizontale:
    /*si la place pour la t?te est occup?e, la gellule va devoir descendre  */
    if( (*Bottle)[GX][GY] != 0 )  goto  encale;

    /* si le joueur appuye ? droite en m?me temps,                          */
    /*   il essaye d'emboiter la gellule dans un passage ? 90?              */

    /* ? ??  (la case du dessus est inoccup?e)    ? ??                      */
    /* ?ab?  (mais le joueur appuye ? droite      ? a?                      */
    /* ?? ?   pour amener la gellule comme ca)--> ??b?                      */

    if(dxjoy == 8)
    {
        /* la case en bas ? droite est-elle libre? */
        if( (*Bottle)[GX+1][GY+2] == 0 )
        {
            GE[1] = GE[3];
            GE[3] = GE[4];
            GE[4] = 0;
            Eff_Case( GX, GY+1 );
            GX++;
            GY++;
            return;
        }
    }

    /*--parfait, gellule debout  */
    GE[1] = GE[3];
    GE[3] = GE[4];
    GE[4] = 0;
    Eff_Case( GX+1, GY+1 );
    return;


/* La case en haut ? gauche est occup?e, la gellule doit descendre d'une case */
/* pour se mettre debout.                                                     */
encale:

    /* si la gellule est au fond, elle peut plus descendre, elle est bloqu?e. */
    if(GY == 15)  return;

    /* # ####  La case du dessous est occup?e, la gellule peut-elle s'ins?rer */
    /* # ####  debout une case ? droite?                                      */
    /* #  AB#                                                                 */
    /* ####+#                                                                 */

    if( (*Bottle)[GX][GY+2] != 0 )  goto  encale_droite;

    /* ---parfait, y'a la place, on la met debout... */
    GE[1] = GE[3];
    GE[3] = GE[4];
    GE[4] = 0;
    Eff_Case( GX+1, GY+1 );
    /*---et on la descend */
    GY++;
    return;

encale_droite:
    /* en base ? droite c'est occup?, y'a vraiment PLUS DE PLACE! */
    if( (*Bottle)[GX+1][GY+2] != 0 )  return;

    /*--ok, on s'installe debout et ? droite! */
    GE[1] = GE[3];
    GE[3] = GE[4];
    GE[4] = 0;
    Eff_Case( GX, GY+1 );

    /*---et on la descend */
    GX++;
    GY++;
}

/* Rotation de la gellule dans le sens INVERSE d'une Horloge..                     */
/*                                                                                                                                                 */
/*                                                                                                                                                 */
static  void  Turn_AntiClockwise(void)
{

    /* ne pas tourner trop vite! */
    if( Pl->BtPress == 2 )      // 2 pour le bouton B
    {
        if( --Pl->BtWait <= 0 )
            Pl->BtWait = 10;
        else
            return;
    }else{
        Pl->BtPress = 2;        // signal bouton A enfonc?.
        Pl->BtWait  = 10;
    }

    /* ralentit un peu la chute quand on tourne la gellule */
    Pl->Count += 5;

    PlaySound( BIP_FX );

    /* si la gellule est horizontale */
    if(GE[1] == 0)  goto  ta_horiz;

    /*la gellule est verticale et bloqu?e ? droite, peut-elle s'emboiter */
    /* dans un espace ? gauche?                                          */
    /*            OU                         */
    /* si quelque chose la bloque ? droite... */
    if( (GX==8) || ((*Bottle)[GX+1][GY+1] != 0) )
    {
     /*---la gellule bloquee a droite peut-elle reculer? */

     /*elle est dans une impasse, bloquee ? droite,
       et contre le bord gauche, fin               */
       if(GX == 1)  return;

     /*---si bloquee a gauche, fin (elle est coincee ? gauche et ? droite!)*/
       if( (*Bottle)[GX-1][GY+1] != 0 )  return;

     /*--ok, elle peut se tourner... */
       GE[4] = GE[3];
       GE[3] = GE[1];
       GE[1] = 0;
       if(GY>0)
           Eff_Case(GX,GY);

     /*---et reculer! */
       GX--;
       return;
    }

    /*---ok, elle est pas bloquee, tournons-la */
    GE[4] = GE[3];
    GE[3] = GE[1];
    GE[1] = 0;
    if(GY>0)
        Eff_Case(GX,GY);
    return;


/*--- la gellule est couch?e, elle se met en verticale en tournant */
ta_horiz:
    /* attention la gellule doit baisser la t?te y'a un plafond */
    if( (*Bottle)[GX][GY] != 0 )  goto ta_encale;

    /*--parfait, gellule debout */
    GE[1] = GE[4];
    GE[4] = 0;
    Eff_Case(GX+1,GY+1);
    return;

    /*---pas de place au-dessus, alors elle descend pour se mettre debout */
ta_encale:
    /*---si au fond de la bouteille, bloquee, fin */
    if(GY == 15)  return;

    /*---obstacle dessous, bloquee, fin */
    if( (*Bottle)[GX][GY+2] != 0 )  goto  ta_encale_droite;

    /*---parfait, y'a la place, on la met debout... */
    GE[1] = GE[4];
    GE[4] = 0;
    Eff_Case(GX+1,GY+1);

    /*---et on la descend */
    GY++;
    return;

ta_encale_droite:
    /* en base ? droite c'est occup?, y'a vraiment PLUS DE PLACE! */
    if( (*Bottle)[GX+1][GY+2] != 0 )  return;

    /* --ok, on s'installe debout et ? droite! */
    GE[1] = GE[4];
    GE[4] = 0;
    Eff_Case(GX,GY+1);

    /*---et on la descend */
    GX++;
    GY++;
}

/* ======================= SUB FINAL_BLIT_GELLULE =========================== */
/*                                                                            */
/* Affichage de la gellule ? sa nouvelle position apr?s tous les mouvements.  */
/*                                                                            */
/* ========================================================================== */

static  void  Final_Draw_Gellule(void)
{
static  UWORD   x,y;

  /* elle est couch?e... */
   if( GE[1] == 0 ){
       Draw_Gellule_Horiz();
       return;
   }

  /* elle est debout! */
   x = Pl->BottleX1 + (GX * TUILEW);
   y = Pl->BottleY1 - TUILEH + (GY * TUILEH);

   if( GY > 0 )
      /* Affiche la moiti? sup. de la capsule! */
      Aff_Tuile( x, y, (GE[1]-1)*4 +1 +GELLULES );

   /* Affiche la moiti? inf. de la capsule. */
   Aff_Tuile( x, y+TUILEH, (GE[3]-1)*4 +2 +GELLULES );
}


/* ----------------------------------------------------------------------- */
/* gellule horizontale: appell? aussi ? chaque arriv?e de nouvelle gellule */
/* pour la dessiner dans le goulot.                                        */
/* ----------------------------------------------------------------------- */
static  void    Draw_Gellule_Horiz()
{
static  UWORD    x,y,c;

    x = Pl->BottleX1 + ( GX * TUILEW );
    y = Pl->BottleY1 + ( GY * TUILEH );

    c = ((GE[3]-1)*4)+3+GELLULES;
    Aff_Tuile( x, y, c );               // moiti? gauche

    x+=TUILEW;
    c = ((GE[4]-1)*4)+4+GELLULES;
    Aff_Tuile( x, y, c );               // moiti? gauche
}


/* Si on appuye vers le bas, on force la chute de la gellule ACCELEREE,   */
/* donc on appelle la routine de chute de gellule!                                                */
/*                                                                                                                                                */
static  void    Push_Down()
{

  /* au cas o? on reste appuy? vers le bas, ne pas descendre trop vite! */
   if( Pl->YPress )
   {
       if( --(Pl->YWait) <= 0 )
       {
           Pl->YWait = 8;
           Gellule_Go_Down();
       }
       return;
   }
   Pl->YPress = true;
   Pl->YWait  = 8;
   Gellule_Go_Down();
}


/* Cette sous-routine fait descendre la gellule d'une case, selon la       */
/* vitesse en cours.                                                       */
/*                                                                         */

static  void    Gellule_Go_Down()
{

    Pl->Bloque = false;
    Pl->Count  = Pl->SpeedTim[ Pl->Speed ];

    /*--si au fond de la bouteille, alors bloquee! */
    if( GY == 15){
        Gellule_Bloquee();
        return;
    }

    /*---y a t'il un obstacle en dessous a gauche? */
    if( (*Bottle)[GX][GY+2] != 0 )
    {
        Gellule_Bloquee();
        return;
    }

    /*--- elle est horizontale */
    if( GE[1] == 0 )
    {
        /*---est-elle bloquee en bas a droite? */
        if( (*Bottle)[GX+1][GY+2] != 0 )
        {
            Gellule_Bloquee();
            return;
        }

        /*---non, alors elle descend */
        Eff_Case( GX,   GY+1 );
        Eff_Case( GX+1, GY+1 );
        GY++;
        return;
    }

    /* ---verticale, elle descend d'une case. */
    if( GY > 0 ){
       Eff_Case( GX, GY );
    }
    GY++;
}


static  void    Gellule_Bloquee()
{
    /* gellule horizontale, datas dans tableau avec bits infos */
    if( GE[1] ==  0 )
    {
        (*Bottle)[GX][GY+1]   = GE[3] + COMPLETE + MOITIE_GAUCHE;
        (*Bottle)[GX+1][GY+1] = GE[4] + COMPLETE + MOITIE_DROITE;
    }else{
    /* gellule verticale, placer datas dans le tableau avec bits infos */

      /* un cas sp?cial peut se pr?senter, la gellule s'arr?te tout en haut */
      /* avec sa partie sup?rieure cach?e dans le bord sup. de la bouteille */
      /* dans ce cas, on coupe la gellule -> la partie inf. devient demi-gel*/
      if( GY>0 )
      {
          (*Bottle)[GX][GY]   = GE[1] + COMPLETE + MOITIE_HAUT;
          (*Bottle)[GX][GY+1] = GE[3] + COMPLETE + MOITIE_BAS;
      }
      else
          (*Bottle)[GX][GY+1] = GE[3];

    }
    Pl->Bloque = true;

    PlaySound( PAF_FX );
}

//            c = ((*Bottle)[dx][dy]) & 3;
//            Aff_Case( dx, dy, GELLULES+11+(c<<1) );
//            (*Bottle)[dx][dy] &= 7;     /* vire le bit COMPLETE */

/**** Anime les virus sous la loupe.
**
**
**
*******************************************************************/

#define CIRCLE   120     /* nombre de positions dans la table CIRCLE */
#define LOUPECX  57
#define LOUPECY  133     /* coords au centre de la loupe */

static  void    Move_Viruses_ala_loupe()
{
static  UWORD  viruspos[3]={0,80,160};    /* offset dans table CIRCLE     */
static  SWORD   virustim[3];               /* timings pour certaines anims */
static  UWORD  virusnow[3];               /* ?tat actuel d'anim des virus */
static  boolean virusalive[3];             /* le virus est-il encore vivant*/
static  SWORD   timer=0;
static  SWORD   v,x,y;

static  struct Icon_s   bigvirus[3];

    /* changement d'animation pour les virus??? */
    for( v=0; v<3; v++ )
    {
      switch( VirusMsg[v] )
      {
         case VIRUSOUILLE:
              AnimIcon( &bigvirus[v], "AB", v*9+5, 4, 0,0, VIRUSPUT );
              VirusMsg[v] = 0;
              virustim[v] = 120;   /* dur?e de l'animation 'touch?' */
              virusnow[v] = VIRUSOUILLE;
              break;
         case VIRUSDEATH:
              AnimIcon( &bigvirus[v], "A", 3*9, 60, 0,0, VIRUSPUT );
              VirusMsg[v] = 0;
              virustim[v] = 30;
              virusnow[v] = VIRUSDEATH;
              break;
         case VIRUSNIARK:
              AnimIcon( &bigvirus[v], "AB", v*9, 25, 0,0, VIRUSPUT );
              VirusMsg[v] = 0;
              virusnow[v] = VIRUSNIARK;
              break;
         case VIRUSMARCHE:
              AnimIcon( &bigvirus[v], "ABCB", v*9+2, 20, 0,0, VIRUSPUT );
              VirusMsg[v] = 0;
              timer = 20;           /* synchro mouvements avec la marche */
              virusnow[v] = VIRUSMARCHE;
              virusalive[v] = true;
              break;
      }
    }

    /* gestion de certaines animations qui durent un temps donn? */
    for( v=0; v<3; v++ )
    {
        /* l'animation du virus qui r?le dure x secondes puis il */
        /* se remet en marche.                                   */
        if( virusnow[v] == VIRUSOUILLE )
        {
            if( --virustim[v] <= 0 )
            {
                if( Pl->TotVirus[v] == 0 )
                {
                    PlaySound( VIRUSDIE_FX );
                    VirusMsg[v] = VIRUSDEATH;
                }
                else
                    VirusMsg[v] = VIRUSMARCHE;
            }
        }
        else if( virusnow[v] == VIRUSDEATH )
        {
            if( --virustim[v] <= 0 )
            {
                VirusMsg[v] = 0;
                /* le virus dispara?t de la loupe */
                virusalive[v] = false;
            }
        }
    }

    /*si les TROIS virus marchent, alors ils peuvent tourner sous la loupe  */
    /*sinon c'est que un d'eux est ? terre, ou fait des grimaces, les autres*/
    /* ne peuvent pas avancer pendant ce temps                              */
    VirusAvance = true;
    for( v=0; v<3; v++ )
    {
        if( (virusalive[v]) && (virusnow[v]!=VIRUSMARCHE) )
           VirusAvance = false;
    }

    /* faire avancer les virus s'ils le peuvent en ce moment */
    if( --timer <= 0 )
    {
      timer = 20;
      if( VirusAvance )  /* est-c-ki-peuvent-tourner? */
      {
        for( v=0; v<3; v++ )
        {
          if( (viruspos[v] += 2) >= CIRCLE*2 )
               viruspos[v] = 0;
        }
      }
    }


    /* Affichage des VIRUS sous la loupe */
    DisplayPic( block[FONDLOUPE], 24,107 );     /* FOND DE LOUPE */
    for( v=0; v<3; v++ )
    {
        x = *(p_circle+viruspos[v])   + LOUPECX - (VIRUSW/2);
        y = *(p_circle+viruspos[v]+1) + LOUPECY - (VIRUSH/2);
        bigvirus[v].xpos = x;
        bigvirus[v].ypos = y;
        /* afficher seulement s'il est vivant */
        if( virusalive[v] )
            RefreshIcon( &bigvirus[v] );
    }
    DisplayPic( block[BORDLOUPE], 16,100 );    /* BORD DE LA LOUPE */
    Transp_effect();
}


/*** Eff_Case - efface une case dans la bouteille en restaurant le fond.
**
**  x,y : coordonn?es de CASE dans la bouteille x(1-8),y(1-16)
**
*******************************************************************/
static  void    Eff_Case( UWORD x, UWORD y )
{
static  UBYTE   *p_src,*p_dest;
static  UWORD   srcmod,destmod;

    /* Shared BottleX1,BottleY1 */
    x--;
    y--;
    x *= TUILEW;
    p_src   = p_bottlbg1 + x + (y * 80 * TUILEH);
    p_dest  = VID->pScreen() + Pl->BottleX1 + x + TUILEW;
    p_dest += (Pl->BottleY1 + (y * TUILEH)) * SCREEN_WIDTH;

    srcmod  = 80-TUILEW;
    destmod = SCREEN_WIDTH-TUILEW;

    y = TUILEH;
    while( y-- > 0 )
    {
        x = TUILEW;
        while( x-- > 0 )
            *(p_dest++) = *(p_src++);
        p_src  += srcmod;
        p_dest += destmod;
    }
}


/*** Aff_Case -v1.0- Dim 20/10/1996.
**
**  Affiche une case de BLOKPAGE (10*7 pixels each), aux coordonn?es
**  TUILE ( x:0->31, y:0->27 ). PAS DE TRANSPARENCE.
**
**  Les coordonn?es BottleX1, BottleY1 indiquent le coin sup. gauche
**  de la bouteille
**
*********************************************************************/
static  void    Aff_Case( UWORD xcase, UWORD ycase, UWORD casenum )
{
static  UWORD   x,y;

    // Shared: BottleX1, BottleY1
    x = Pl->BottleX1 + ( xcase * TUILEW );
    y = Pl->BottleY1 - TUILEH + ( ycase * TUILEH );

    Aff_Tuile( x, y, casenum );
}



/****************************************************************************/
/*   UTILISE UNIQUEMENT PAR SEEK AND DESTROY.                               */
/*  cette proc d?truit NB cases de la couleur COUL en partant de            */
/*  X,Y et en avancant dans le sens de mx et my                             */
/*  Si un virus est d?truit, Pl->VirusKill augmente, si une gellule compl?te*/
/*  doit ?tre coup?e, l'autre moiti? est r?affich?e en demi-gellule libre.  */
/*  ajoute le code impossible %1100 aux cases d?truites (pour plus tard).   */
/*                                                                          */
/*  Chaque appel de cette fonction augmente Pl->Super de 1 (le compteur de  */
/*   colonnes/rang?es ?limin?es).                                           */
/****************************************************************************/

static  void    Destroy_Gellules( SWORD x, SWORD y, SWORD nb, UBYTE coul, SWORD mx, SWORD my )
{
static  UBYTE   code,c;
static  SWORD    dx,dy;
static  boolean   virus;    /* flag VRAI si au moins un virus d?truit */

    virus = false;

    /*Shared Bottle(),Pl->VirusKill,BottleX1,BottleY1*/
    do{
        /*---lire la case du tableau*/
        code = (*Bottle)[x][y];

        /* si la case a d?j? ?t? d?truite on l'ignore
           (car on fait deux passages crois?s VERTICAL et HORIZONTAL)
           (dans Seek_and_Destroy) */
        /* 12 = code VIRUS + GELLULE COMPLETE, soit une combinaison
           impossible! (donc reconnaissable) */
        if( (code & 12) == 12 )
        {
            x += mx;
            y += my;
            continue;
        }

        /*--code virus+gellulecomplete impossible, that's good!*/
        (*Bottle)[x][y] = (code & 3) + VIRUS+COMPLETE;

        /* On bousille un Virus ... */
        if( (code & VIRUS) != 0 )
        {
            Pl->VirusKill++;

            /* d?compte les virus de chaque couleur (for 1 player mode) */
            /* le bigvirus fait 'OUILLE!', et quand il a fini son anim, */
            /* si le compteur TotVirus de la couleur correspondante est */
            /* ? z?ro, l'anim de DESTRUCTION suit celle de OUILLE.      */
            if( Pl->TotVirus[coul-1] > 0 )
                Pl->TotVirus[coul-1]--;

            VirusMsg[coul-1] = VIRUSOUILLE;

            virus = true;

            Aff_Case( x, y, GELLULES+18+coul );      //explosion de virus
        }
        else
        {
            Aff_Case( x, y, GELLULES+12+(coul<<1) ); //explosion de gellule
        }

        /*---si autre moiti?(complete), couper en 2*/
        if( (code & COMPLETE) != 0 )
        {
            if( (code & 32) == 0 )
                dx = -1;            /* l'autre morceau en haut ou ? gauche */
            else
                dx = 1;             /* l'autre morceau en bas ou ? droite */
            if( (code & 16) == 0 )
            {   dy = dx;
                dx = 0;
            }
            else
                dy = 0;
            dx += x;
            dy += y;
            c = ((*Bottle)[dx][dy]) & 3;
            Aff_Case( dx, dy, GELLULES+11+(c<<1) );
            (*Bottle)[dx][dy] &= 7;     /* vire le bit COMPLETE */
        }

        x += mx;
        y += my;
    }while( --nb > 0 );

    PlaySound( BOUM_FX );       /* de toute facon des gellules explosent */
    if( virus )                 /* un virus au moins a ?t? d?truit */
    {
       /* en mode 2 players, on a droit aussi au bruit de virus TUE m?me */
       /* si on ne les voit pas sous la loupe.                           */
       /* (quand tous les virus de la m?me couleur sont d?truits!)       */
       if( (NPlayers==2) && (Pl->TotVirus[coul-1]==0) )
           PlaySound( VIRUSDIE_FX );
       else
           PlaySound( TOUCHE_FX );
    }

    /* retiens le code couleur de la crasse qui sera peut-?tre envoy?e */
    /* chez l'autre si on d?truit au moins 2 rang?es/colonnes          */
    if( Pl->Super < 4 )
        Pl->BetaCrasses[Pl->Super] = coul;

    /* incr?mente le compteur de rang?es/colonnes d?truites */
    Pl->Super++;
}

/*  SEEK AND DESTROY                                                        */
/*  cherche dans tout le tableau les couleurs align?es de 4 cases ou plus,  */
/*  les ?limine, remplace les cases d?truites par des explosions et des     */
/*  codes %1100 pour les rep?rer. Les gellules coup?es en 2 sont divis?es en*/
/*  deux plus petites. Notez que les VIRUS ne peuvent se d?truirent d'eux   */
/*  m?mes (s'ils sont plus de 3 de la m?me couleur align?e)... il faut au   */
/*  moins une demi-gellule de m?me couleur pour d?truire des virus!         */

static  boolean Seek_And_Destroy(void)
{
static  boolean    destroy;

/*  Ici je ne parcours pas deux fois le tableau (Horiz, puis Vert)
    --> que non: je parcours le tableau par lignes Horizontales, tout en
        comptant les suites de couleurs verticales.                     */
static  SWORD    x,y;
static  SWORD    ncv[8+1];       /*                    */
static  SWORD    lvc[8+1];       /* Last Virus Color ? */
static  SWORD    gelv[8+1];

static  SWORD    coul;
static  SWORD    lastcoul,ncases;
static  SWORD    gel;

  //Shared: Bottle[][], Pl->VirusKill, lvc(), ncv(), GELV()

   /* DESTROY doit valoir YES si des cases ont ?t? d?truites
      (cel? indique qu'il faut continuer ? tester si des cases peuvent tomber)*/
    destroy = false;

  //r?initialiser le vecteur lvc() ? vide
    for( x=1; x<=8; x++ )
    {
        lvc[x] = -1;
        ncv[x] =  1;
    }

    for( y=1; y<=16; y++ )
    {

     //initialiser les variables de tests horizontaux (oui : ? chaque ligne!)
        lastcoul = -1;      /* Last color */
        ncases = 1;         /* nb de cases de m?me couleurs align?es */

     /* gel sert ? v?rifier qu'on ?limine pas une s?rie de virus de m?me
        couleur sans intervention d'une gellule
        (les virus ne peuvent s'?liminer tous seuls)                    */

        for( x=1; x<=8; x++ )
        {
            //chercher la couleur de cette case
            coul = (*Bottle)[x][y] & 0x03;

            //1. TESTS POUR LES SUITES HORIZONTALES
            if( coul != lastcoul )
            {
                // couleur  !=  de la pr?c?dente, mais a-t-on termin? une suite?
                // si oui, ?liminer les cases.
                //
                if( ncases > 3 )
                {
                    if( lastcoul > 0 )
                    {
                        //au moins une partie de gellule dans la s?rie?
                        if( (gel & VIRUS) == 0 )
                        {
                            Destroy_Gellules( x-ncases, y, ncases, (UBYTE)lastcoul, 1, 0 );
                            destroy = true;
                        }
                    }
                }
                ncases   = 1;     /* cette nvelle case, de couleur diff?rente */
                lastcoul = coul;
                gel = (*Bottle)[x][y];
            }
            else
            {
                gel &= ( (*Bottle)[x][y] & VIRUS );
                ncases++;
            }

            //2. TESTS POUR LES SUITES VERTICALES
            if( coul != lvc[x] )
            {
                // couleur  !=  de celle du dessus, a-t-on termin? une suite verticale?
                // si oui, ?liminer cette suite verticale.
                // (sur 16 cases de haut, il peut y en avoir plusieurs!)
                if( ncv[x] > 3 )
                {
                    if( lvc[x] > 0 )
                    {
                        if( (gelv[x] & VIRUS) == 0 )
                        {
                            Destroy_Gellules( x, y-ncv[x], ncv[x], (UBYTE)lvc[x], 0, 1 );
                            destroy = true;
                        }
                    }
                }
                ncv[x]  = 1;
                lvc[x]  = coul;
                gelv[x] = (*Bottle)[x][y];
            }
            else
            {
                gelv[x] &= ( (*Bottle)[x][y] & VIRUS );
                ncv[x]++;
            }
        }//Next X

        //Test de FIN DE LIGNE HORIZONTALE
        if( ncases > 3 ){
            if( lastcoul > 0 ){
                if( (gel & VIRUS) == 0 ){
                    Destroy_Gellules( 9-ncases, y, ncases, (UBYTE)lastcoul, 1, 0 );
                    destroy = true;
                }
            }
        }

    }//Next Y

    //Tests de FIN DE COLONNES VERTICALES
    // pour les 8 COLONNES!
    for( x=1; x<=8; x++ )
    {
        if( ncv[x] > 3 ){
            if( lvc[x] > 0 ){
                if( (gelv[x] & VIRUS) == 0 )
                {
                    Destroy_Gellules( x, 17-ncv[x], ncv[x], (UBYTE)lvc[x], 0, 1 );
                    destroy = true;
                }
            }
        }
    }//Next X

    return( destroy );
}

/* Efface le contenu de la bouteille -> GRAPHIQUEMENT! */
/*                                                     */
/*                                                     */
static  void    Clear_Bottle(void)
{
    CopyBlock(p_bottlbg1,0,0,80,112,80, VID->pScreen(),Pl->BottleX1+TUILEW,Pl->BottleY1,SCREEN_WIDTH);
}

/*  Cette routine anime tous les virus dans la bouteille du joueur en cours */
/*  Il y a deux images par virus, et le changement d'image se fait en m?me  */
/* temps pour les trois types de virus, dans le m?me tempo. Donc il suffit  */
/* d'afficher l'image A de tous les virus, et la fois suivante l'image B de */
/* tous les virus! Pas besoin de stocker l'image en cours pour chaque virus.*/
#define ANIM_VIRUS_RATE  25
static  void    Anime_virus(void)
{
static  UBYTE   code,image;

static  UWORD   xpos,ypos,x,y;

    if( --Pl->VTimer < 0 )
    {
        Pl->VTimer = ANIM_VIRUS_RATE;
        Pl->VFrame = 3 - Pl->VFrame;      /* swappe entre 0 et 3 rapidos */
    }

    image = GELLULES+21+Pl->VFrame;

    ypos = Pl->BottleY1 + (TUILEH*5);
    for( y=6; y<=16; y++ )
    {
        xpos = Pl->BottleX1 + TUILEW;
        for( x=1; x<9; x++ )
        {
            if( (code = (*Bottle)[x][y] & 0x07) > 3 )   /* bit VIRUS? */
                Aff_Tuile( xpos, ypos, (code & 3)+image );
            xpos += TUILEW;
        }
        ypos += TUILEH;
    }
}


/**** MAKE TRANSPARENT TABLE
**
**  Cr?e une table pour acc?l?rer l'effet de transparence de la loupe.
**  Il faut que ONEPLAYER page se trouve dans l'?cran virtuel.
**  La couleur 255 dans la zone de la loupe indique les parties qui
**  seront plus claires (effet de brillant).
**
**********************************************************************/
#define X1TRANSP 24
#define X2TRANSP 88
#define Y1TRANSP 106
#define Y2TRANSP 160      /* coords inclusives pour la zone test?e */
static  void  Make_transp_table()
{
static  UBYTE  *p_src;
static  SBYTE   *p_dest;
static  UBYTE  c;
static  SBYTE   ycpt;
static  UWORD  x,y;

    p_dest = p_transptable;

    for( x=X1TRANSP; x<=X2TRANSP; x++ )
    {
        p_src  = VID->pScreen() + x + ( Y1TRANSP * SCREEN_WIDTH );
        ycpt = 0;
        for( y=Y1TRANSP; y<=Y2TRANSP; y++ )
        {
            c = *(p_src);
            if( c==255 )   /* couleur claire */
            {
                if( ycpt>0 )            /* pr?c?dent Run de coul de fond */
                {
                    *(p_dest++) = ycpt;
                    ycpt = 0;
                }
                ycpt--;
            }
            else                         /* couleur de fond */
            {
                if( ycpt<0 )            /* pr?c?dent Run de coul claire */
                {
                    *(p_dest++) = ycpt;
                    ycpt = 0;
                }
                ycpt++;
            }
            p_src += SCREEN_WIDTH;
        }
        if( ycpt != 0 )
            *( p_dest++ ) = ycpt;       /* enregistre le dernier Run */
        /* marque la fin d'une colonne */
        *(p_dest++) = 0;
    }

}

/**** CREE L'EFFET DE TRANSPARENCE DE LA LOUPE.
**
**  Eclaire certaines parties sous la loupe pour l'effet de verre.
**  -> ajoute simplement un offset de 32 ? tous les pixels qui se
**     trouvent sur les lignes 'brillantes' du verre.
**  -> voir table pr?calcul?e par Make_transp_table()
**
*********************************************************************/
static  void    Transp_effect()
{
static  SBYTE   *p_tab;
static  UBYTE  *p_dest,*p_col;
static  SBYTE   code;
static  SWORD   ncol;

    p_tab = p_transptable;
    ncol  = X2TRANSP - X1TRANSP + 1;
    p_col = VID->pScreen() + X1TRANSP + (Y1TRANSP * SCREEN_WIDTH);

    while( ncol-- > 0 )
    {
        p_dest = p_col;
        while( (code = *(p_tab++)) != 0 )
        {
            if( code > 0 )
                p_dest += (UWORD) code * SCREEN_WIDTH;
            else
            {
                while( code++<0 )
                {
                    *(p_dest) += 32;    /* ?claire ce pixel */
                    p_dest += SCREEN_WIDTH;
                }
            }
        }
        p_col++;
    }

}


/**** -ONE PLAYER GAME-
**
**
**
**
***************************************************************************/

static  void    DrMario_OnePlayerGame(void)
{

static  SBYTE    finished;           /* passe ? un pour interrompre le jeu */

static	boolean   testeu;
static  int    fadestep;

	if (GameState == gs_oneplayer)
	{
		testeu = false;

		DigitMode = dmBLACKONWHITE;               // caract?res noir sur blanc
		
		/* decrunch One Player Page ---------------------------------*/
		
		DecrunchPBM( p_oneplayr, VID->pScreen(), dest_pal );
		memset( src_pal, 0, sizeof(src_pal) );
		VID->SetPalette (src_pal);
		
		/* sauve DrMario initial, dans sa petite fen?tre rouge */
		p_marioinzebox = ReserveBackground( 69*51 );
		//Copy_block(VID->pScreen()+220+(36*SCREEN_WIDTH),p_marioinzebox,69,51,SCREEN_WIDTH,69);
		CopyBlock(VID->pScreen(),220,36,69,51,SCREEN_WIDTH, p_marioinzebox,0,0,69);
		
		/* pr?pare la table de transparence pour la loupe */
		Make_transp_table();
		
		/* sauver la table??? ???????? ????????? ????????? ?????????? */
		
		/* clear bottle */
		//Copy_block(p_bottlbg1,VID->pScreen()+120+(56*SCREEN_WIDTH),80,112,80,SCREEN_WIDTH);
		CopyBlock(p_bottlbg1,0,0,80,112,80, VID->pScreen(),120,56,SCREEN_WIDTH);
		
		
		//if( Music || Sound )
		//    mp_volume = 25;
		
		//----------------INITIALISATIONS MODE 1 JOUEUR----------------------
		
		Pl1.Panning = MIDDLE_PANNING;   /* distribution du son gauche/droite*/
		
		Pl1.Speed = Speed1P;
		Pl1.Level = VirusLevel1P;
		
		Pl1.XNext = 240;
		Pl1.YNext = 40;                 /* o? afficher NEXT gellule         */
		Pl1.XLevel= 270;
		Pl1.YLevel= 126;                /* o? afficher LEVEL                */
		Pl1.XSpeed= 260;
		Pl1.YSpeed= 147;                /* o? afficher SPEED                */
		Pl1.XVirus= 270;
		Pl1.YVirus= 168;                /* o? afficher TOTAL VIRUS          */
		
		Pl1.BottleX1=120-TUILEW;
		Pl1.BottleY1=56;                /* coin sup. gauche INTERIEUR       */
		
		Pl1.VFrame = 0;
		Pl1.VTimer = 0;       /* pour animation des virus dans la bouteille */
		
		Pl1.Step  = Init_Level;
		
		EscapeKey = false;                    /* ne pas quitter la partie encore! */
		
		Score = 0;
		
		Pl = &Pl1;
		
		Bottle = &Bottle1;

		fadestep = 0;
		GameState = gs_oneplayer_fadein;
	}

	else

	if (GameState == gs_oneplayer_fadein)
	{
            Fade_to (main_pal, src_pal, dest_pal, fadestep);
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            fadestep += FADESPEED;
			if (fadestep>100)
			{
				fadestep = 100;
				GameState = gs_oneplayer_loop;
			}
	}
    
	else
		

    //kb_clear();
    /*****************************************************************\
    *                                                                 *
    *              MEGA MEGA BOUCLE DU MODE 1 JOUEUR                  *
    *                                                                 *
    \*****************************************************************/
	if (GameState == gs_oneplayer_loop)
	{
        //these areoften used, so better make a temporary copy for routines
		// than using pointers and dereference everywhere (also it makes more difficult reading)
		GX = Pl->GelluleX;
        GY = Pl->GelluleY;
        GE[1] = Pl->Gellule[1];
        GE[2] = Pl->Gellule[2];
        GE[3] = Pl->Gellule[3];
        GE[4] = Pl->Gellule[4];
        (*Pl->Step)();      /* appelle la routine en cours pour ce player */
        Pl->Gellule[1] = GE[1];
        Pl->Gellule[2] = GE[2];
        Pl->Gellule[3] = GE[3];
        Pl->Gellule[4] = GE[4];
        Pl->GelluleX = GX;
        Pl->GelluleY = GY;

        Move_Viruses_ala_loupe();   /* anime les Virus sous la loupe */

        //Vbl_wait();
        //Scr_put();
        //if( Music || Sound )
        //    MD_Update();            /* update sound channels */

        if( KEY_DOWN( VK_ESCAPE ) || EscapeKey )	//EscapeKey may be set by current Pl->Step routine to end loop
            GameState = gs_oneplayer_end;

        if( KEY_DOWN('T') && (!testeu) )
        {
            Pl->Step = WinGame_1pl;
            testeu = true;
        }

        if( KEY_DOWN('Y') )
            testeu = false;

    }

	else

	if (GameState == gs_oneplayer_end)
	{    
            Fade_to (main_pal, src_pal, dest_pal, fadestep);
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            fadestep -= FADESPEED;

			if (fadestep<0)
			{
				//
				// one player end cleanup
				//
				/* nettoyages avant de partir.... */		
				FreeBackground( p_marioinzebox );
				GameState = gs_titlepage;
			}
	}
}


/**** -TWO PLAYER GAME-
**
**
**
**
***************************************************************************/
static  void    DrMario_TwoPlayerGame(void)
{
static boolean testeu;
static int		fadestep;
	
	if (GameState == gs_twoplayers)
	{
		testeu = false;
		
		DigitMode = dmBLACKONWHITE;               // caract?res noir sur blanc
		
		/* decrunch TWO Player Page ---------------------------------*/
		
		DecrunchPBM( p_twoplayr, VID->pScreen(), dest_pal);
		memset( src_pal, 0, sizeof(src_pal) );
		VID->SetPalette( src_pal );
	
		
		//if( Music || Sound )
		//        mp_volume = 25;
		
		//---------------INITIALISATIONS MODE 2 JOUEURS----------------------
		
		Pl1.PlayerNum = 1;
		Pl2.PlayerNum = 2;  /* num?ros des joueurs */
		
		Pl1.LAutre = &Pl2;
		Pl2.LAutre = &Pl1;  /* les 2 structures se pointent l'une l'autre */
		
		Pl1.Panning = RIGHTPLAYER_PANNING; /* distribution du son sur les */
		Pl2.Panning = LEFTPLAYER_PANNING;  /* haut parleurs               */
		
		Pl1.Speed = Speed1P;
		Pl1.Level = VirusLevel1P;
		Pl1.XNext = 230;
		Pl1.YNext = 45;
		Pl1.XLevel= 165;
		Pl1.YLevel= 36;
		Pl1.XSpeed= 165;
		Pl1.YSpeed= 44;
		Pl1.XVirus= 163;
		Pl1.YVirus= 167;
		Pl1.XWin  = 161;
		Pl1.YWin  = 85;
		
		Pl1.BottleX1=200-TUILEW;
		Pl1.BottleY1=70;
		
		Pl1.VFrame = 0;
		Pl1.VTimer = 0;       /* pour animation des virus dans la bouteille */
		
		Pl1.Step  = Init_Level;
		
		Pl2.Speed = Speed2P;
		Pl2.Level = VirusLevel2P;
		Pl2.XNext = 70;
		Pl2.YNext = 45;
		Pl2.XLevel= 135;
		Pl2.YLevel= 36;                 /* o? afficher LEVEL                */
		Pl2.XSpeed= 125;
		Pl2.YSpeed= 44;                 /* o? afficher SPEED                */
		Pl2.XVirus= 137;
		Pl2.YVirus= 167;                /* o? afficher TOTAL VIRUS          */
		Pl2.XWin  = 141;
		Pl2.YWin  = 85;                 /* o? afficher les COURONNES        */
		
		Pl2.BottleX1=40-TUILEW;
		Pl2.BottleY1=70;
		
		Pl2.VFrame = 0;
		Pl2.VTimer = 0;
		
		Pl2.Step  = Init_Level;
		
		EscapeKey = false;                    /* ne pas quitter la partie encore! */
		
		Pl = &Pl1;
		Bottle = &Bottle1;
		
		Pl1.Wins = 0;                   /* compteurs de victoires ? z?ro    */
		Pl2.Wins = 0;
		
		//-------------------------------------------------------------------
		fadestep = 0;
		GameState = gs_twoplayers_fadein;
	}
	
	else

	if (GameState == gs_twoplayers_fadein)
	{    
            Fade_to (main_pal, src_pal, dest_pal, fadestep);
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            fadestep += FADESPEED;
			if (fadestep>100)
				GameState = gs_twoplayers_loop;
	}
    
	else
	
	//kb_clear();
    /*****************************************************************\
    *                                                                 *
    *              MEGA MEGA BOUCLE DU MODE 2 JOUEUR                  *
    *                                                                 *
    \*****************************************************************/

	if (GameState == gs_twoplayers_loop)
	{    

        Pl = &Pl1;
        Bottle = &Bottle1;
        GX = Pl->GelluleX;
        GY = Pl->GelluleY;
        GE[1] = Pl->Gellule[1];
        GE[2] = Pl->Gellule[2];
        GE[3] = Pl->Gellule[3];
        GE[4] = Pl->Gellule[4];
        (*Pl->Step)();      /* appelle la routine en cours pour ce player */
        Pl->Gellule[1] = GE[1];
        Pl->Gellule[2] = GE[2];
        Pl->Gellule[3] = GE[3];
        Pl->Gellule[4] = GE[4];
        Pl->GelluleX = GX;
        Pl->GelluleY = GY;

        Pl = &Pl2;
        Bottle = &Bottle2;
        GX = Pl->GelluleX;
        GY = Pl->GelluleY;
        GE[1] = Pl->Gellule[1];
        GE[2] = Pl->Gellule[2];
        GE[3] = Pl->Gellule[3];
        GE[4] = Pl->Gellule[4];
        (*Pl->Step)();      /* appelle la routine en cours pour ce player */
        Pl->Gellule[1] = GE[1];
        Pl->Gellule[2] = GE[2];
        Pl->Gellule[3] = GE[3];
        Pl->Gellule[4] = GE[4];
        Pl->GelluleX = GX;
        Pl->GelluleY = GY;

        //Vbl_wait();
        //Scr_put();
        //if( Music || Sound )
           // MD_Update();            /* update sound channels */

        if( KEY_DOWN( VK_ESCAPE ) || EscapeKey)
		{
			fadestep = 100;
            GameState = gs_twoplayers_fadeout;
		}

        if( KEY_DOWN( 'T' ) && (!testeu))
        {
			 UWORD ix;
			 SWORD  n;

            n = (rand() & 3)+1;
            while( n-- > 0 )
            {
               ix = (rand() & 7) + 1;
               (*Bottle)[ix][1] = (rand() % 3) +1;
            }
            testeu = true;
        }

        if( KEY_DOWN('Y') )
            testeu = false;

    }

	else

	if (GameState == gs_twoplayers_fadeout)
	{    
            Fade_to (main_pal, src_pal, dest_pal, fadestep);
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            fadestep -= FADESPEED;
			if (fadestep<0)
				GameState = gs_titlepage;
	}
}



/**** TitlePage -Dim 20/10/1996.
**
**  Page de titre du jeu, et choix de jeu 1player ou 2player.
**
**************************************************************************/

#define LOGOSPEED   35

static  int	DrMario_TitlePage(void)
{

static	boolean	title_initted=false;
	
static  SWORD    choix=1;

static  UWORD    logoframe;
static  SWORD    logotick;

static  struct  Icon_s   pied_mario;
static  struct  Icon_s   coeur;
static  struct  Icon_s   moqueur;

static  boolean  doQuitGame;
static  int    fadein,fadeout;    /* fadein en background de la boucle */

	//
	// setup for titlepage
	// 
	if (GameState == gs_titlepage)
	{
		// decrunch TitlePic  --------------------------------------
		
		DecrunchPBM (p_titlepic, VID->pScreen(), dest_pal);
		memset( main_pal, 0, sizeof(src_pal) );
		VID->SetPalette( src_pal );
		
		//Aff_Zone( 100, 132-(5*TUILEH), "PREVERSION!!!" );
		Aff_Zone( 100, 132, "1 PLAYER GAME\2\2"
			"2 PLAYER GAME" );
		//Scr_put();
		
		fadein = 0;
		fadeout = -1;
		
		AnimIcon( &pied_mario, "AB", FOOTFRAME1, 35, 72, 163, COPYPUT );
		AnimIcon( &coeur, "ABCB", COEURFRAME1, 5, 88, 132, COPYPUT );
		AnimIcon( &moqueur, "AB", 0, 25, 240, 140, VIRUSPUT );
		
		logoframe = LOGOFRAME1;
		logotick = 0;
		
		GameState = gs_titlepage_loop;
	}

    /* ...................................... --- temporaire --- */
    /* C'EST POUR UTILISER PLAYSOUND() ...                       */
    Pl = &Pl1;
    Pl->Panning = MIDDLE_PANNING;
    /* ---------------------------------------------end of hack--*/

    if (GameState == gs_titlepage_loop)
    {
        Aff_Tuile( coeur.xpos, coeur.ypos, ' ' );

        if( KEY_DOWN( Pl1.P_Up ) )
        {
            if( choix > 1 )
            {
                choix--;
                PlaySound( SELECT_FX );
            }
        }
        else if( KEY_DOWN( Pl1.P_Down ) )
        {
            if( choix < 2 )
            {
                choix++;
                PlaySound( SELECT_FX );
            }
        }

        coeur.ypos = 118 + (choix * TUILEH * 2);

        /* Affichages animations  */
        RefreshIcon( &coeur );
        RefreshIcon( &pied_mario );
        FillRect( 240,140,271,160, 0 ); /* efface le fond derri?re le virus */
        RefreshIcon( &moqueur );

        // Animation du logo
        if( ++logotick > LOGOSPEED )
        {   logotick = 0;
            if( ++logoframe > LOGOFRAME1+1 )
                logoframe = LOGOFRAME1;
        }
            PasteIcon( 51, 41, logoframe, TRANSPUT );

        /* fade in title screen */
        if( fadein <= 100 )
        {
            //if( Music || Sound )
            //  mp_volume = fadein;

            Fade_to (main_pal, src_pal, dest_pal, fadein);
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            fadein += FADESPEED;
        }
        else if( fadeout >= 0 )
        {
            /*if( Music || Sound )
            {
             if( QuitGame )
                mp_volume = fadeout;
            }*/

            Fade_to( main_pal, src_pal, dest_pal, fadeout );
			VID->WaitVbl();
            VID->SetPalette( main_pal );
            if( (fadeout -= FADESPEED) < 0 )
			{
				QuitGame = doQuitGame;
				GameState = gs_options;
			}
        }
		

        // position STRATEGIQUE!
        if( KEY_DOWN( VK_ESCAPE ) )
        {
                doQuitGame = true;
                fadeout = 100;      // Start fade-out and then get out!
        }
        if( KEY_DOWN(VK_RETURN) || KEY_DOWN(Pl1.P_ButtonA) || KEY_DOWN(VK_SPACE) )
        {
                doQuitGame = false;
                fadeout = 100;
                PlaySound( START_FX );
        }
    }

    return choix;    //1=1player, 2=2player, other = quit.
}


/*** ShowVirusLevel (OptionPage) - Lun 21/10/1996.
**
**  Affiche le curseur sur la graduation VIRUS LEVEL, OptionPage.
**
**  tuile = num?ro de tuile (BLOKPAGE) pour le curseur.
**  ypos  = coord. y pour afficher le curseur (change selon 1P ou 2P)
**  level = entre 0 et 20.
**
************************************************************************/
static  void    ShowVirusLevel( UWORD tuile, UWORD ypos, SWORD level )
{
static  UWORD   xpos;

    /* 1. efface le curseur pr?c?dent */
    FillRect( 106, ypos, 215, ypos+6, 0 );

    /* 2. affiche le curseur sur la graduation */
    xpos = 106 + ( level * 5 );
    Aff_Tuile( xpos, ypos, tuile );
}

/*** AffDigits (for all game)- Lun 21/10/1996.
**
**  Affiche une valeur (en chiffres!) avec la fonte de BLOKPAGE,
**  aux coords ECRAN donn?es.
**
**  Shared:  -!!! Notez bien !!!-
**
**  La variable principale DigitMode doit ?tre initialis?e
**  ? BLACKONWHITE ou WHITEONBLACK.
**
****************************************************************/
static  void    Aff_Digits( UBYTE ncar, ULONG value, UWORD xpos, UWORD ypos )
{
static  int   ah;

    xpos += ( ncar * TUILEW ) - TUILEW;

    while( ncar-- > 0 )
    {
        ah = value % 10;
        Aff_Tuile( xpos, ypos, DigitMode + ah );
        value /= 10;
        xpos -= 10;
    }
}


/*** OptionBox (OptionPage)- Lun 21/10/1996.
**
**  Affiche le contour s?lectionn? ou non s?lectionn? de l'une des
**  3 options de l' ecran d'options.
**
**  state <- ON  : affiche le contour 'select'
**  state <- OFF : affiche le contour 'unselect'
**
************************************************************************/


static  void    OptionBox( UWORD option, boolean state )
{

/* Pour chaque option, la coord. du coin sup. gauche est la m?me
   pour le graphique SELECT et UNSELECT.                        */

static  UWORD   optx[3] = { 54,54,54 };     /* coordonn?es pour les 3 options */
static  UWORD   opty[3] = { 44,93,135 };

    /* num?ro d'icone pour le graphique SELECT de chaque option */
   /* UNSELECT frame number = SELECT frame number +1           */

static  UWORD   optframe[3] = { OPVLEVL1, OPSPEED1, OPMUSIC1 };

static  UWORD   frame;

    option--;

    frame = optframe[ option ];
    if( !state )
        frame++;    // si c'est OFF c'est UNSELECT

    PasteIcon( optx[ option ], opty[ option ], frame, COPYPUT );
}

/*** MusicTypeBox (OptionPage)- Lun 21/10/1996.
**
**  Affiche le contour s?lectionn? ou non s?lectionn? de l'une des
**  3 options de musiques (ecran d'options).
**
**  state <- ON  : affiche le contour 'select'
**  state <- OFF : affiche le contour 'unselect'
**
************************************************************************/
static  void    MusicTypeBox( UWORD music, boolean state )
{
static  UWORD   musx[3] = {  74, 144, 214 };
static  UWORD   musy[3] = { 157, 157, 157 };
static  UWORD   musbox[3] = { MUSBOX1, MUSBOX1, MUSBOX3 };

static  UWORD   frame;

    music--;

    frame = musbox[ music ];
    if( !state )
        frame++;    // si c'est OFF c'est UNSELECT

    PasteIcon( musx[ music ], musy[ music ], frame, TRANSPUT );
}

/*** ShowSpeed (OptionPage)- Lun 21/10/1996.
**
**  Affiche le curseur SPEED sur la position LOW-MED-HI (1,2 ou 3).
**
*******************************************************************/
static  void    ShowSpeed( UWORD ypos, UWORD tuile, SWORD speed )
{
static  UWORD   xpos;

    speed--;
    xpos = 110 + ( speed * 50 );

    FillRect( 110, ypos, 239, ypos+6, 0 );      // efface le curseur d'avant.

    Aff_Tuile( xpos, ypos, tuile );
    tuile++;
    xpos+=TUILEW;
    Aff_Tuile( xpos, ypos, tuile );
    tuile++;
    xpos+=TUILEW;
    Aff_Tuile( xpos, ypos, tuile );
}


/** OptionsPage
**
**   Shared:
**       NPlayers = 1 or 2 (number of players)
**
**   retourne ON pour jouer, OFF pour retour ? la page titre.
**
**************************************************************************/
static  void    DrMario_OptionsPage(void)
{
static  UWORD   currentoption=1;     /* vaut 1,2,3 pour VIRUSLEVEL, SPEED,
                                                        MUSICTYPE         */

#define VLEVLTIMER 10;       /* temps entre 2 positions VIRUS LEVEL quand
                               on reste appuy? */

static  SWORD    timer1,timer2,timer3;
                             /* ralentis le curseur si reste appuy? */

#define SPEEDCURS1P 128         // (128,129,130)
#define SPEEDCURS2P 131         // (131,132,133)

static  SBYTE    finished;
static  int    fadein,fadeout;    /* fadein en background de la boucle */
static	gamestate_t nextGameState;

static  UWORD   vbox1;              // ypos pour AffDigits() VirusLevel1P

    DigitMode = dmWHITEONBLACK;               // caract?res blanc sur noir

	if (GameState == gs_options)
	{
		/* decrunch OptionPage and fade in. ---------------------------------*/
		
		DecrunchPBM( p_optionpg, VID->pScreen(), dest_pal);
		memset( src_pal, 0, sizeof(src_pal) );
		VID->SetPalette( src_pal );
		
		//if( Music || Sound )
		//	mp_volume = 25;
		
		/* ajouter les modifications OptionPage selon NPlayers = 1 ou 2 */
		if( NPlayers == 1 )
		{
			PasteIcon( 225, 66, VLEVLBOX, COPYPUT );
			vbox1 = 70;
		}
		else
		{
			Aff_Tuile( 100, 28, '2' );                   /* '2' PLAYER GAME  */
			PasteIcon( 225, 59, VLEVLBOX, COPYPUT );
			PasteIcon( 225, 80, VLEVLBOX, COPYPUT );
			PasteIcon( 110, 77, GRADUAT, COPYPUT );
			Aff_Zone( 80, 70,  "1P\2""2P" );             /* VIRUS LEVEL 1P 2P*/
			Aff_Zone( 80, 112, "1P\2""2P" );             /* SPEED 1P 2P      */
			ShowVirusLevel( 127, 81, VirusLevel2P );
			vbox1 = 63;
			Aff_Digits( 2, VirusLevel2P, 230, 84 );
			ShowSpeed( 122, SPEEDCURS2P, Speed2P );
		}
		ShowVirusLevel( 126, 65, VirusLevel1P );
		Aff_Digits( 2, VirusLevel1P, 230, vbox1 );
		ShowSpeed( 109, SPEEDCURS1P, Speed1P );
		OptionBox( currentoption, true );
		MusicTypeBox( MusicType, true );
		
		
		finished = false;
		fadein = 0;
		fadeout = -1;

		GameState = gs_options_loop;
	}
    
	/* ...................................... --- temporaire --- */
    /* C'EST POUR UTILISER PLAYSOUND() ...                       */
    Pl = &Pl1;
    /* ---------------------------------------------end of hack--*/

	if (GameState == gs_options_loop)
    {

        /* 1. TESTER PLAYER 1 UP/DOWN - PLAYER 1 CHOISIS L'OPTION */

        OptionBox( currentoption, false );
        if( KEY_DOWN( Pl1.P_Up ) )
        {
            if( --timer3 < 0 )
            {
                timer3 = VLEVLTIMER;
                if( currentoption > 1 )
                {
                    currentoption--;
                    Pl->Panning = MIDDLE_PANNING;
                    PlaySound( CLIC_FX );
                }
            }
        }
        else if( KEY_DOWN( Pl1.P_Down ) )
        {
            if( --timer3 < 0 )
            {
                timer3 = VLEVLTIMER;
                if( currentoption < 3 )
                {
                    currentoption++;
                    Pl->Panning = MIDDLE_PANNING;
                    PlaySound( CLIC_FX );
                }
            }
        }
        else
            timer3 = 0;
        OptionBox( currentoption, true );

        /* 2. SELON L'OPTION, MODIFIER VIRUSLEVEL, SPEED, MUSIC. */

        if( currentoption == 1 )                /* CHANGE VIRUS LEVEL */
        {
            if( KEY_DOWN( Pl1.P_Left ))
            {   if( --timer1 < 0 )
                {   timer1 = VLEVLTIMER;
                    if( VirusLevel1P > 0 )
                    {
                        VirusLevel1P--;
                        Pl->Panning = 255-(VirusLevel1P*255)/20;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else if( KEY_DOWN( Pl1.P_Right ))
            {   if( --timer1 < 0 )
                {   timer1 = VLEVLTIMER;
                    if( VirusLevel1P < 20 )
                    {
                        VirusLevel1P++;
                        Pl->Panning = 255-(VirusLevel1P*255)/20;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else
                timer1 = 0;
            ShowVirusLevel( 126, 65, VirusLevel1P );
            Aff_Digits( 2, VirusLevel1P, 230, vbox1 );

            if( NPlayers == 2 )
            {
                if( KEY_DOWN( Pl2.P_Left ))
                {   if( --timer2 < 0 )
                    {   timer2 = VLEVLTIMER;
                        if( VirusLevel2P > 0 )
                        {
                            VirusLevel2P--;
                            Pl->Panning = 255-(VirusLevel2P*255)/20;
                            PlaySound( CLIC2_FX );
                        }
                    }
                }
                else if( KEY_DOWN( Pl2.P_Right ))
                {   if( --timer2 < 0 )
                    {   timer2 = VLEVLTIMER;
                        if( VirusLevel2P < 20 )
                        {
                            VirusLevel2P++;
                            Pl->Panning = 255-(VirusLevel2P*255)/20;
                            PlaySound( CLIC2_FX );
                        }
                    }
                }
                else
                    timer2 = 0;
                ShowVirusLevel( 127, 81, VirusLevel2P );
                Aff_Digits( 2, VirusLevel2P, 230, 84 );
            }
        }
        else if( currentoption == 2 )           /* CHANGE SPEED */
        {
            if( KEY_DOWN( Pl1.P_Left ))
            {   if( --timer1 < 0 )
                {   timer1 = VLEVLTIMER;
                    if( Speed1P > 1 )
                    {
                        Speed1P--;
                        Pl->Panning = MIDDLE_PANNING;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else if( KEY_DOWN( Pl1.P_Right ))
            {   if( --timer1 < 0 )
                {   timer1 = VLEVLTIMER;
                    if( Speed1P < 3 )
                    {
                        Speed1P++;
                        Pl->Panning = MIDDLE_PANNING;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else
                timer1 = 0;
            ShowSpeed( 109, SPEEDCURS1P, Speed1P );

            if( NPlayers == 2 )
            {
                if( KEY_DOWN( Pl2.P_Left ))
                {   if( --timer2 < 0 )
                    {   timer2 = VLEVLTIMER;
                        if( Speed2P > 1 )
                        {
                            Speed2P--;
                            Pl->Panning = MIDDLE_PANNING;
                            PlaySound( CLIC2_FX );
                        }
                    }
                }
                else if( KEY_DOWN( Pl2.P_Right ))
                {   if( --timer2 < 0 )
                    {   timer2 = VLEVLTIMER;
                        if( Speed2P < 3 )
                        {
                            Speed2P++;
                            Pl->Panning = MIDDLE_PANNING;
                            PlaySound( CLIC2_FX );
                        }
                    }
                }
                else
                    timer2 = 0;
                ShowSpeed( 122, SPEEDCURS2P, Speed2P );
            }
        }
        else
        {
            /* CHANGER MUSIC TYPE */
            MusicTypeBox( MusicType, false );
            if( KEY_DOWN( Pl1.P_Left ))
            {
                if( --timer1 < 0 )
                {
                    timer1 = VLEVLTIMER;
                    if( MusicType > 1 )
                    {
                        MusicType--;
                        Pl->Panning = MIDDLE_PANNING;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else if( KEY_DOWN( Pl1.P_Right ))
            {
                if( --timer1 < 0 )
                {
                    timer1 = VLEVLTIMER;
                    if( MusicType < 3 )
                    {
                        MusicType++;
                        Pl->Panning = MIDDLE_PANNING;
                        PlaySound( CLIC2_FX );
                    }
                }
            }
            else
                timer1 = 0;
            MusicTypeBox( MusicType, true );
        }

		
        if( fadein <= 100 )
        {
            Fade_to( main_pal, src_pal, dest_pal, fadein );
            VID->WaitVbl();
			VID->SetPalette( main_pal );
            fadein += FADESPEED;
        }
        else if( fadeout >= 0 )
        {
            Fade_to( main_pal, src_pal, dest_pal, fadeout );
            VID->WaitVbl();
			VID->SetPalette( main_pal );
            if( (fadeout -= FADESPEED) < 0 )
				GameState = nextGameState;
        }

        //if( Music || Sound )
        //    MD_Update();            /* update sound channels */

        /* 3. PLAYER 1 BOUTON A = Joue! - ESCAPE = retour page titre. */

        if( KEY_DOWN(Pl1.P_ButtonA) || KEY_DOWN(VK_SPACE) || KEY_DOWN(VK_RETURN) )
        {
            fadeout = 100;

            Pl->Panning = MIDDLE_PANNING;
            PlaySound( START_FX );
			if (NPlayers==1)
				nextGameState = gs_oneplayer;
			else
				nextGameState = gs_twoplayers;
        }
        else if( KEY_DOWN( VK_ESCAPE ))
        {
            fadeout = 100;
			nextGameState = gs_titlepage;
        }
    }

}


static BOOL EnumGameSoundsCallback( HANDLE hFile,
									int iSound,
									struct Directory_s* pEntry )
{
	LPDIRECTSOUNDBUFFER lpSfx;
	
	// note : the file pointer is already set to the beginning of the entry (which is a RIFF WAVE)
	lpSfx = AUD->LoadWave( hFile );

	Sound_ptr[iSound] = lpSfx;
	
	// continue until we have all the sounds
	return ( iSound < MAX_SAMPLES-1 ) ? TRUE : FALSE;
}


//
//
//
boolean Game_Init( BOOL bSound )
{

    /*--------------------------------------------------- MEMORY ALLOCATIONS*/

    //cputs("Mem_Init: ");
    if (!Alloc_all())
		return false;
    //cputs("ok.\r\n");

    p_current_bg = p_main_bg;

    /*--------------------------------------------------- IWAD OPEN --------*/

    //cputs("WAD Init: ");
	CWad DrMWAD ( "drmario.wad" );

    /*--------------------------------------------------- LOAD SOUND DATA --*/

	// startup audio
	bSoundEnabled = TRUE;
	if( bSound )
	{
		AUD = new CAudio( myWindow );

		// load all sounds in S_SOUNDS section
		if( !DrMWAD.EnumSection( "S_SOUNDS", EnumGameSoundsCallback ) )
			InitFail("Error while loading game sounds.");
	}

		/* simple test
	DrMWAD.SeekEntry( "FXCLIC" );
	HANDLE hFile = DrMWAD.GetFileHandle();
	lpSfx = AUD->LoadWave( hFile );
	hahaha:
	if( MessageBox( NULL, "LoadWave succeeded", "yahoo!", MB_YESNO ) == IDYES )
		AUD->StartSound( lpSfx, 31, 128, 0, 128 );
	if( MessageBox( NULL, "Did you hear the sound ?", "yahoo!", MB_YESNO ) == IDNO )
		goto hahaha;
		*/

    /*--------------------------------------------------- LOAD GFX DATA ----*/

    /* Charge BLOKSPAGE -> page de blocs (tous align?s en grille),        */
    /*   comprennent la fonte en blanc sur noir et noir sur blanc,        */
    /*   tous les morceaux de gellules et autres petits patchs graphiques */


    if( (p_titlepic = (UBYTE *) DrMWAD.ReadEntry( "TITLEPIC" )) == NULL )
        InitFail("Err:TITLEPIC lump not found.");

    if( (p_optionpg = (UBYTE *) DrMWAD.ReadEntry( "OPTIONPG" )) == NULL )
        InitFail("Err:OPTIONPG lump not found.");

    if( (p_oneplayr = (UBYTE *) DrMWAD.ReadEntry( "ONEPLAYR" )) == NULL )
        InitFail("Err:ONEPLAYR lump not found.");

    if( (p_twoplayr = (UBYTE *) DrMWAD.ReadEntry( "TWOPLAYR" )) == NULL )
        InitFail("Err:TWOPLAYR lump not found.");

    if( (p_blokpage = (UBYTE *) DrMWAD.ReadEntry( "BLOKPAGE" )) == NULL )
        InitFail("Err:BLOKPAGE lump not found.");

    if( (p_bottlbg1 = (UBYTE *) DrMWAD.ReadEntry( "BOTTLBG1" )) == NULL )
        InitFail("Err:BOTTLBG1 lump not found.");

    if( (p_viruspag = (UBYTE *) DrMWAD.ReadEntry( "VIRUSPAG" )) == NULL )
        InitFail("Err:VIRUSPAG lump not found.");

    if( (p_circle   = (SBYTE *)  DrMWAD.ReadEntry( "CIRCLE" )) == NULL )
        InitFail("Err:CIRCLE lump not found.");

    if( (p_blocks = DrMWAD.ReadSection( "S_BLOCKS", block )) == NULL )
        InitFail("Err:not enough memory for blocks.");

    //cputs("ok.\r\n");

	//--------------------------------------------------------------------------

	bGamePaused = false;

	/* INITIALISE RANDOM NUMBER GENERATOR */
    srand(time(NULL));

    /* INITIALISE LES VARIABLES PRINCIPALES DU JEU */

    VirusLevel1P = 0;
    VirusLevel2P = 0;
    Speed1P = 1;
    Speed2P = 1;
    MusicType = 1;

    QuitGame = false;

    NPlayers = 0;

    TopScore = 0;        /* meilleur score 1 joueur */

    /* TOUCHES PLAYER 1 --- PAR DEFAUT --- */

	SetDefaultKeys( &Pl1 );

    /* TOUCHES PLAYER 2 --- PAR DEFAUT --- */

	SetDefaultKeys( &Pl2 );

	lastTime = timeGetTime();

	GameState = gs_titlepage;
	
	return true;	//succesful game init
}


//
// the main game loop
//
void doGameState (void)
{

	switch (GameState)
	{
	  case gs_titlepage:
	  case gs_titlepage_loop:
			NPlayers = DrMario_TitlePage();                    // TITLE PAGE
			break;
	  case gs_options:
	  case gs_options_loop:
			DrMario_OptionsPage();
			break;
	  case gs_oneplayer:
	  case gs_oneplayer_fadein:
	  case gs_oneplayer_loop:
	  case gs_oneplayer_end:
			DrMario_OnePlayerGame();                // 1 PLAYER GAME
			break;
	  case gs_twoplayers:
	  case gs_twoplayers_fadein:
	  case gs_twoplayers_loop:
	  case gs_twoplayers_fadeout:
			DrMario_TwoPlayerGame();                // 2 PLAYER GAME
			break;
	}
	
}


//
// return true  = continue game
//        false =  exit program
//
boolean Game_Main (void)
{
	ULONG	nowTime;
	SLONG	elapsedTime;
	
	nowTime = timeGetTime();
	elapsedTime = nowTime - lastTime;

	// we do maximum 35 frames per second
	if (elapsedTime < GAMETICTIME)
		return true;
	lastTime = nowTime;

	// run the game logic for as many frames as passed since last
	// time we were here
	while (elapsedTime >= GAMETICTIME)
	{
		doGameState ();
		elapsedTime -= GAMETICTIME;
	}

	// finally update the screen
	VID->FinishUpdate();
	
	return !QuitGame;
}


//
//
//
void Game_Shutdown (void)
{
	
	// shutdown Audio
	SAFE_DELETE( AUD )

	//--------------------------------------------------- FREE GFX DATA

    FreeMem( p_blocks );
    FreeMem( p_circle );
    FreeMem( p_viruspag );
    FreeMem( p_bottlbg1 );
    FreeMem( p_blokpage );
    FreeMem( p_twoplayr );
    FreeMem( p_oneplayr );
    FreeMem( p_optionpg );
    FreeMem( p_titlepic );

    //--------------------------------------------------- FREE SOUND DATA
	/*
    if( Sound )
        Free_Samples();     // lib?re m?moire sound effects

    if( Music )
        ML_Free( p_titlemus );          // free the module
	*/
    
	//--------------------------------------------------- IWAD CLOSE


    //--------------------------------------------------- FREE MEMORY

    Restore_all();

    //--------------------------------------------------- DE-INIT HARDWARE
	/*
    if( Music || Sound )
    {
        if( Sound_initted )
            MD_Exit();
    }
	*/
    
	//--------------------------------------------------- OVER.

}


// called from DrMario.cpp , passed to setup controls dialog
DWORD GetPlayerVars( int iPlayer )
{
	if( iPlayer==1 )
		return (DWORD)(&Pl1);
	else
		return (DWORD)(&Pl2);
}

// called from 'Setup keys' dialog as well if 'default keys' button clicked
void SetDefaultKeys( struct PlayerVars* plV )
{
	if( plV == &Pl1 ) {
		Pl1.P_Up       = VK_UP;
		Pl1.P_Down     = VK_DOWN;
		Pl1.P_Left     = VK_LEFT;
		Pl1.P_Right    = VK_RIGHT;
		Pl1.P_ButtonA  = VK_CONTROL;
		Pl1.P_ButtonB  = VK_SHIFT;
	}
	else if( plV == &Pl2 )
	{
		Pl2.P_Up       = 'R';
		Pl2.P_Down     = 'F';
		Pl2.P_Left     = 'D';
		Pl2.P_Right    = 'G';
		Pl2.P_ButtonA  = 'S';
		Pl2.P_ButtonB  = 'A';
	}
}


void PauseGame()
{
	bGamePaused = true;
}

void UnpauseGame()
{
	bGamePaused = false;
	lastTime = timeGetTime();
}
