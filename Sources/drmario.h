
// drmario.h : game definitions

#include "fabtypes.h"
#include "CAudio.h"
#include "CLogFile.h"
#include "CVideo.h"

#define DEBUG   0               /* set to 1 for WAD i/o debugging   */
#define VERSION 101


// these query the keyboard in real-time
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)


//
// structures
//


#define TRANSP_COL  255         /* numéro de couleur transparente       */

#define COPYPUT     0           /* copie tous les pixels sans distinction */
#define TRANSPUT    1           /* la couleur 255 est transparente        */
#define VIRUSPUT    2           /* affiche un virus avec transparence     */

#define BOTTLEBGCOL 124         /* couleur de fond principale de la bouteille*/

struct Icon_s
{
    UWORD	 currentframe;      /* frame actually displayed                  */
    UWORD    initframe;         /* first frame corresponding to A in animseq */
    UWORD    framerate;         /* how many video refresh between 2 frames   */
    char     *animseq;           /* points to start of animation sequence     */
    char     *p_anim;            /* points current pos. into anim sequence    */
    SWORD    counter;           /* count time between frames                 */
    SWORD    xpos, ypos;        /* actual position where Icon is displayed   */
    UWORD    mode;              /* display mode is COPYPUT,TRANSPUT,...      */
    UBYTE    *p_bgbuf;           /* pointe buffer de sauvegarde du fond       */
                                /* ou NULL */
    SWORD    xold,yold;         /* dernière pos. de sauvegarde du fond       */
    SWORD    oldw,oldh;         /* taille du morceau de fond sauvegardé      */
};

#define VIRUSOUILLE  1   /* un petit virus a été killé dans la bouteille */
#define VIRUSDEATH   2   /* TOUS les virus d'une mˆme couleur sont morts */
#define VIRUSNIARK   3   /* partie perdue: les gros virus se moquent     */
#define VIRUSMARCHE  4   /* pour initialiser la ronde des virus          */


#define TRANSPTABLEMAX 4096    /* estimation! cf Make_transp_table() */

/* ces macros donnent la taille d'un Block (format BLK) en accédant les */
/* champs XSIZE, YSIZE au début du Block.                               */
#define BOBWIDTH(bluk)  ((UWORD) *((UWORD*)block[bluk]))
#define BOBHEIGHT(bluk) ((UWORD) *((UWORD*)(block[bluk]+2)))

#define BOBSIZE(bluk)   (BOBWIDTH(bluk)*BOBHEIGHT(bluk))    /* taille  RAW */
#define ICONSIZE(bluk)  (BOBWIDTH(bluk)*BOBHEIGHT(bluk)+8)  /* taille .BLK */



#define VIRUSW        32      /* taille d'un virus en pixels */
#define VIRUSH        21
#define VIRUSBOBSIZE  (VIRUSW*VIRUSH)

#define MAINBGBUFSIZE (320*200) /* taille max. du buffer de sauvegarde des */
                                /* fonds.                                  */
#define COLORMAPSIZE    768     /* size of a colormap buffer
                                       classical R,G,B * 256 color. */


#define DEUX    1           /* DECALAGES LOGIQUES PLUS LISIBLES             */
#define QUATRE  2
#define HUIT    3
#define SEIZE   4


#define TUILEW  10			// size of tiles in BLOKPAGE
#define TUILEH  7


#define FADESPEED 2         // fade in/ fade out speed (1 is slower)


// ------
// sounds
// ------

enum GameSound_t {
     CLIC_FX,		/* numero des samples dans Sound_ptr[...] */
     BOUM_FX,		/* boum! quand des cases sont détruites   */
     PAF_FX,		/* paf! quand une gellule est posée       */
     BIP_FX,		/* bip! quand on déplace la gellule horiz.*/
     OUILLE_FX,		/* ouille! quand on bute un virus!        */
     CLIC2_FX,		/* deuxieme bip dans les options <---->   */
     GAMEOVER_FX,
     START_FX,
     SELECT_FX,
     TOUCHE_FX,		/* quand un virus sous la loupe est touché*/
     VIRUSDIE_FX,
     CHUTE_FX,
     SUPER_FX,
     WIN_FX,		/* un joueur gagne un match (en 2 players)*/

     MAX_SAMPLES	/* si ca dépasse la routine le signalera! */
};

#define SFX_NUMVOICES   3       /* nombre de voix suppl. pour effets sonores */

// panning values used with play sample MikMod function
#define MIDDLE_PANNING          (SEP_RANGE/2)   // sound in middle
#define LEFTPLAYER_PANNING      (SEP_RANGE-1)   // sound left of stereo
#define RIGHTPLAYER_PANNING     0				// sound right of stereo


// ---------------------------------------------------------
// Icons entries number in S_BLOCKS section for OPTIONS PAGE
// ---------------------------------------------------------
#define MAXBLOCKS   50  /* nb d'Icons (format BLK) max ds le tableau de ptrs*/

#define FOOTFRAME1  0   /* 2 frames DrMario tappe du pied */
#define LOGOFRAME1  2   /* 2 frames LOGO TitlePage        */
#define COEURFRAME1 4
#define GRADUAT  7
#define VLEVLBOX 8
#define OPVLEVL1 9
#define OPVLEVL2 10
#define OPSPEED1 11
#define OPSPEED2 12
#define OPMUSIC1 13
#define OPMUSIC2 14
#define MUSBOX1  15
#define MUSBOX2  16
#define MUSBOX3  17
#define MUSBOX4  18
#define BRASLANCE 19    /* 3 images du bras de DrMario lancant une gellule */
#define BORDLOUPE 22    /* FORMAT PACKE DE DOOM! */
#define FONDLOUPE 23    /*      idem             */
#define MARIOWI1  24
#define START     26
#define WINPANEL  27
#define COURONNE  28    /* suivi du fond (sans la couronne) */
#define PANNEAU   29
#define VIRUSBRAS 30
#define LOSPANEL  33
#define MARIOLOS  34


// ------------
// tile numbers
// ------------
#define GELLULES    (139)      // first of series of tiles for drawing the capsules
#define TRGELLULES  (139+27)   // capsules with transparent borders (color 255)


// -----------
// digit modes
// -----------
#define dmBLACKONWHITE    20      /* la valeur indique en fait le numéro */
#define dmWHITEONBLACK    48      /* de block sur BLOKPAGE du '0'        */


// bitflags for tiles in the bottle
#define VIRUS           4       /* case = VIRUS !                     */
#define COMPLETE        8       /* case = moitié de gellule complète  */
#define MOITIE_GAUCHE   32+16
#define MOITIE_DROITE   16
#define MOITIE_HAUT     32
#define MOITIE_BAS      0       /* ce sont les VALEURS des bits! */


// ----------------
// Player structure
// ----------------

struct  PlayerVars{

                struct PlayerVars *LAutre;   /* pointe les données adverses */

                UBYTE   PlayerNum;      /* numéro du joueur (1-2)           */

                UBYTE   Panning;        /* 0-255 samples panning <--O-->    */

                UBYTE   Nx1,Nx2;        /* next gellule                     */
                UBYTE   Gellule[5];

                UWORD   XNext,YNext;    /* o— afficher Next gellule         */
                UWORD   XLevel,YLevel;
                UWORD   XSpeed,YSpeed;
                UWORD   XVirus,YVirus;
                UWORD   XWin,YWin;

                UBYTE   P_Up;
                UBYTE   P_Down;
                UBYTE   P_Left;
                UBYTE   P_Right;
                UBYTE   P_ButtonA;
                UBYTE   P_ButtonB;      /* touches pour ce joueur */

                /* coords du coin sup. gauche intérieur de la bouteille
                    -10 en X !!!!                                           */
                UWORD   BottleX1,BottleY1;

                SWORD   Speed;          /* Player Speed (1=LOW,2=MED,3=HI)  */
                SWORD   Level;          /* Virus Level                      */

                SWORD   NVirus;         /* nb de Virus restant in-ze-bouteil*/

                SWORD   Wins;           /* nb de couronnes gagnées (2pl)    */

                /*-----Remplit_Bouteille-----*/
                SWORD   Coul;

                /*-------Next_Gellule--------*/
                SWORD   Nth;            /* compteur de gellules depuis le
                                           début de chaque niveau. Après la
                                           10ème la vitesse de chute accélère
                                           un peu. */
                SWORD   SpeedTim[4];    /* 3 vitesses de chute (en vbls)    */

                SWORD   Count;          /* compteur pour descente gellules  */

                UBYTE   XPress;         /* indique si on reste appuyé vers
                                           la gauche ou la droite           */
                boolean YPress;         /* indicateurs si on reste appuyé   */
                boolean Bloque;

                UBYTE   BtPress;        /* indique quand un bouton (A,B) est
                                           enfoncé                          */
                SWORD   BtWait;         /* délaye la rotation de gellule si
                                           on reste appuyé sur un bouton    */

                SWORD   XWait;          /* délaye déplacement horizontal si
                                           on reste appuyé sur gauche/droite*/
                SWORD   YWait;          /* compteur descend gellule accéléré*/

                UBYTE   VirusKill;
                UBYTE   TotVirus[3];    /* nb de Virus encore vivants pour  */
                                        /* chaque couleur.                  */

                SWORD   GelluleX,GelluleY;  /* coords de la gellule chutant */

                SBYTE   Explose;        /* counter to wait some time when
                                           blocks are destroyed             */
                SBYTE   Chute;          /* autre compteur pour délayer la
                                           chute des blocs libérés suite aux
                                           explosions                       */
                SWORD   Super;          /* compte nb rangées/colonnes détrui*/
                                        /*-tes en une seule série           */

                boolean ChuteCrasse;    /* ON si des crasses doivent chuter */

                UBYTE   BetaCrasses[4]; /* un code couleur pris dans chaque */
                                        /* rangée/colonne détruite, BETA car*/
                                        /* sera copié dans Crasses[] adverse*/
                                        /* seulement quand les chutes sont  */
                                        /* terminées. QUESTION D'EVITER QUE */
                                        /* l'un modifie les Crasses pendant */
                                        /* que l'autre les lit (les recoit) */

                /*------ ces variables sont placées par l'autre joueur -----*/
                UWORD   NCrasses;       /* nb de pièces envoyées par l'autre*/
                                        /* (2-4) 0 indique 'PAS de crasses' */
                UBYTE   Crasses[4];     /* codes couleur des pièces         */

                /*-----Anim des virus dans la bouteille------*/
                UBYTE   VFrame;
                SBYTE   VTimer;

                void    (*Step)(void);
};


// ------
// protos
// ------

extern	CVideo*	VID;			// the display object (window surface or fullscreen directdraw surface)
extern	HWND	myWindow;		// main app window
extern	CLogFile* LOG;
