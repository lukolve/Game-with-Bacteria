/* Bakteria | The_Bactery 2002-2012  MIT License
 * by Lukas 'luko' Veselovsky, <lukves@gmail.com>
 */

#include <stdio.h>
#include <allegro.h>

// WxH CONFIG WORD is HD Resolution 
#define CONFIG_WIDTH	1280
#define CONFIG_HEIGHT	720

#define CONFIG_MARGIN_L 20
#define CONFIG_MARGIN_R 20
#define CONFIG_MARGIN_T 10
#define CONFIG_MARGIN_B 80

#define CONFIG_SQ_WIDTH 4
#define CONFIG_SQ_HEIGHT 4

// SND CONFIG WORD is enabled as default
#define CONFIG_SOUND	1

// WBxHB CONFIG WORD of how Player Table
#define CONFIG_X_TABLE (CONFIG_WIDTH-CONFIG_MARGIN_L-CONFIG_MARGIN_R)/ CONFIG_SQ_WIDTH
#define CONFIG_Y_TABLE (CONFIG_HEIGHT-CONFIG_MARGIN_T-CONFIG_MARGIN_B)/CONFIG_SQ_HEIGHT

// some parameters
#define aloclog 512

int score[3];

#define NEUTRAL 0
#define HUMAN 1
#define COMPUTER 2
#define OUTSIDE 9

/* cursor pointer, set for print text position */
int TXT_CURSOR_X;
int TXT_CURSOR_Y;


/* MIDI music */
MIDI *the_music;

/* Deklracia palety */
PALETTE pal;

/* Deklaracia bitmap, ktore sa budu pouzivat ako obrazky*/
BITMAP  *obr1, *obr2, *obr3, *obr4, *custom_cursor;

/* Casovacia premenna.
 * volatile znamena, ze premenna sa bude menit aj mimo program,
 * teda napriklad prerusenim casovaca */
volatile int speed_counter = 0;
 
/* Nasa premenna na kontrolu casovaca */
int time_counter  = 0;

/* Deklaracia, kolko bude trvat jeden cyklus medzi preruseniami */
int DelayTime     = 2;


/* ----------- CASOVAC / TIMER --------------------- */

/* Tato funkcia sa bude volat vzdy v urcitych casovych intervaloch
 * a jej ulohou je zvysovat pocitadlo, ktore pouzivame ako casovac */
void increment_speed_counter()
{ 
  speed_counter++;
}

END_OF_FUNCTION(increment_speed_counter);

/* Nainstaluje casovac */
void SetUpTimer()
{
  LOCK_VARIABLE(speed_counter);
  LOCK_FUNCTION(increment_speed_counter);
  install_int_ex(increment_speed_counter, MSEC_TO_TIMER(20));
}

/* Deklaracia delay, tak ako ste ju poznali v Paskale, ale
 * tentokrat s nasim casovacom */
void Delay(int _x)
{
  int WaitingTime = speed_counter + _x;
  do { } while (speed_counter<WaitingTime);
}

/* Nastavi casovac na pociatocnu hodnotu */ 
void ResetTimer()
{
  speed_counter = 0;
}

/* Load MIDI file to the_music */ 
int init_sndmidi()
{
    /* install a MIDI sound driver */
    if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, "   ") != 0)
    {
    	allegro_message("Error initialising sound system\n%s\n", allegro_error);
      	return -1;
   	}
    
   /* read in the MIDI file */
   the_music = load_midi("sttmp.mid");
   if (!the_music) {
      allegro_message("Error reading MIDI file '%s'\n", "sttmp.mid");
      return -1;
   } 
   return 0;
}

// write char defined in font.bmp - there was all chars on one bitmap
// select position of char on bitmap and put to screen.
void draw_custom_font(int x, int y, int pis)
{
	// void blit(BITMAP *source, BITMAP *dest, int source_x, int 
	// source_y, int dest_x, int dest_y, int width, int height);
    show_mouse(NULL);
  	blit(obr1, screen, 16*pis, 0, x, y, 16, 22);
  	show_mouse(screen);
}

const int MSG_INFO[] = { 15, 11, 0, 24, 4, 17 };

void draw_info() 
{
	int i;

	for (i = 0; i < 6; i++) //// sizeof(MSG_INFO)
		draw_custom_font(TXT_CURSOR_X + 16 * i, TXT_CURSOR_Y, MSG_INFO[i]);
	
	if (winner_test()==1) 
		draw_custom_font(TXT_CURSOR_X + 16 * (i+1), TXT_CURSOR_Y, 31); 
	else if (winner_test()==2) 
		draw_custom_font(TXT_CURSOR_X + 16 * (i+1), TXT_CURSOR_Y, 32);

	draw_custom_font(TXT_CURSOR_X + 16 * (i+3), TXT_CURSOR_Y, 22);
	draw_custom_font(TXT_CURSOR_X + 16 * (i+4), TXT_CURSOR_Y,  8);
	draw_custom_font(TXT_CURSOR_X + 16 * (i+5), TXT_CURSOR_Y, 13);
}


/* --------------PROGRAM------------------- */

int Init_Game()
{    
  int ic;	

  /* Inicializacia Allegra -- nutne uviest, inak nemozete pouzivat 
   * fukcie Allegra (teda mozete, ale necudujte sa, ze obcas nepracuju
   * spravne) */
  allegro_init();
   
   
  install_mouse();   
  
  /* Instalacia ovladacu klavesnice 
   *  - odchytava viac klaves naraz
   *  - zabranuje preplneniu klavesnicoveho buffera */ 
  install_keyboard();

  /* Instalacia sluzieb casovaca
   * Velmi dolezite, pretoze pokial ho nemate nainstalovany, nemusi
   * vam klavesnica pracovat spolahlivo */
  install_timer();
  
  init_sndmidi();
  
  /* Detekcia grafiky AUTODETECT -- mozete si zvolit vlastnu, ak
   * napriklad chcete, aby sa vam hra zobrazovala iba v okne
   * mozete pouzit GFX_XWINDOWS (pre Linuxy) */
  if(set_gfx_mode(GFX_AUTODETECT, CONFIG_WIDTH, CONFIG_HEIGHT, 0, 0)<0) {
    printf("[printf] Problem with initialize a  graphics mode !\n");
    return 2;
  }
  
  /* Nastavi hlbku rozlisenia 8 = 256 farieb */
  set_color_depth(8);
   
  obr1 = load_bmp("font.bmp",pal);
  obr2 = load_pcx("obr2.pcx",pal);
  obr3 = load_pcx("obr3.pcx",pal);
  obr4 = load_pcx("obr4.pcx",pal);
   
  if ((obr1 == NULL) ||
      (obr2 == NULL) ||
      (obr3 == NULL) ||
      (obr4 == NULL)) {
    	set_gfx_mode(GFX_TEXT,0,0,0,0);
    
    	printf("[printf] Error: one, or more (*.pcx) are missing !\n");
    	return 1;
  }
  
  /* create a custom mouse cursor bitmap... */
   custom_cursor = create_bitmap(32, 32);
   clear_to_color(custom_cursor, bitmap_mask_color(screen));
   for (ic=0; ic<4; ic++)
      circle(custom_cursor, 16, 16, ic*2, palette_color[ic]);

   /* select the custom cursor and set the focus point to the middle of it */
   set_mouse_sprite(custom_cursor);
   set_mouse_sprite_focus(16, 16);
   
   /* Instalacia a nastavenie casovaca */
   ResetTimer();
   SetUpTimer();
      
  return 0;
}

void Done_Game()
{
  destroy_midi(the_music);
  destroy_bitmap(obr1);
  destroy_bitmap(obr2);
  destroy_bitmap(obr3);
  destroy_bitmap(obr4);
  destroy_bitmap(custom_cursor);  
  set_gfx_mode(GFX_TEXT,0,0,0,0);
  /* Textovy mod */

  remove_timer();
  /* odinstalovanie casovaca */
  remove_keyboard();
  /* odisntalovanie klavesnice */
  allegro_exit();
  /* Ukoncenie prace s Allegrom */
  
  printf("[printf] Quit Program ...\n");
}


/* 
 * The Game Logic of Bacteria 
 * 
 */

int dna[CONFIG_X_TABLE][CONFIG_Y_TABLE];
int a,b,s,h,hh,nahod;
int m[20];
int u,v;


/*
void bunka(int x, int y, int c)
{
 
 rect(screen, 4*x,4*y,4*x+3,4*y+3,c);

 rect(screen, 4*x+1,4*y+1,4*x+2,4*y+2, c-8);

}
*/

void bunka(int x, int y, int c)
{
	rect(screen, 
		CONFIG_MARGIN_L + CONFIG_SQ_WIDTH*x,
		CONFIG_MARGIN_T + CONFIG_SQ_HEIGHT*y,
		CONFIG_MARGIN_L + CONFIG_SQ_WIDTH*x + CONFIG_SQ_WIDTH - 1,
		CONFIG_MARGIN_T + CONFIG_SQ_HEIGHT*y + CONFIG_SQ_HEIGHT - 1,
		c);
	rect(screen, 
		CONFIG_MARGIN_L + CONFIG_SQ_WIDTH*x + 1,
		CONFIG_MARGIN_T + CONFIG_SQ_HEIGHT*y + 1,
		CONFIG_MARGIN_L + CONFIG_SQ_WIDTH*x + CONFIG_SQ_WIDTH - 2,
		CONFIG_MARGIN_T + CONFIG_SQ_HEIGHT*y + CONFIG_SQ_HEIGHT - 2,
		c - 8);
}


int end_test()
{
	for (a=1; a<=CONFIG_X_TABLE; a++)

  		for (b=1; b<=CONFIG_Y_TABLE; b++ ) {

    		if (dna[a][b]==NEUTRAL) return 1;

  		}
  	return 0;
}

/*
int winner_test()
{
	
	int c = 0;
	int d = 0;
	
	for (a=1; a<=CONFIG_X_TABLE; a++)

  		for (b=1; b<=CONFIG_Y_TABLE; b++ ) {

    		if (dna[a][b]==1) c++;
    		if (dna[a][b]==2) d++;

  		}
  		
  	if (c>=d) return 1; else return 2;
}
*/

int winner_test()
{
	score[HUMAN] 	= 0;
	score[COMPUTER] = 0;
	for (a=1; a <= CONFIG_X_TABLE; a++)
  		for (b=1; b <= CONFIG_Y_TABLE; b++)
  			switch (dna[a][b]) {
  				case HUMAN: score[HUMAN]++;break;
  				case COMPUTER: score[COMPUTER]++;break;
  			}
  	return (score[HUMAN] > score[COMPUTER])? HUMAN : COMPUTER;
}


void stage_init()
{
  for (a=0; a<=CONFIG_X_TABLE; a++)
  	for (b=0; b<=CONFIG_Y_TABLE; b++ ) {
    	dna[a][b]=OUTSIDE;
  }

  for (a=1; a<CONFIG_X_TABLE; a++)
  	for (b=1; b<CONFIG_Y_TABLE; b++) {
    	dna[a][b]=NEUTRAL;
  }

  for (a=1; a<=hh; a++) m[a]=a;
}


/* recalc a bactery life */
void stage_recalc()
{
  for (a = 1; a < CONFIG_X_TABLE; a++)
	    for (b = 1; b < CONFIG_Y_TABLE; b++) 
		    for (h = 1; h <= hh; h++) 
	      		if (dna[a][b]==m[h]) 
	      			switch (random()%5) {
	      				case 1: if (dna[a+1][b]==0) dna[a+1][b]=m[h]+10; break;
						case 2: if (dna[a][b+1]==0) dna[a][b+1]=m[h]+10; break;
						case 3: if (dna[a-1][b]==0) dna[a-1][b]=m[h]+10; break;
						case 4: if (dna[a][b-1]==0) dna[a][b-1]=m[h]+10; break;
	      			}
	
	show_mouse(NULL);
	for (a=1; a < CONFIG_X_TABLE; a++)
		for (b=1; b < CONFIG_Y_TABLE; b++)
		    for (h=1; h<=hh; h++) {
      			if (dna[a][b]==m[h]+10) { 
      				dna[a][b]=m[h];
	 				bunka(a, b, m[h]+8);
      			}
		    	if (dna[a][b]==OUTSIDE) bunka(a, b, 20);
		    }
      			
	show_mouse(screen);
}



void player_action()
{  
 	char msg[80];
  //int WaitingTime;	 
 
    poll_mouse(); 
    
    show_mouse(NULL);  
    
	u = (mouse_x - CONFIG_MARGIN_L - CONFIG_SQ_WIDTH) / 4; 
	if (u >= CONFIG_X_TABLE) u = CONFIG_X_TABLE - 1;
	v = (mouse_y - CONFIG_MARGIN_T - CONFIG_SQ_HEIGHT) / 4;
	if (v >= CONFIG_Y_TABLE) v = CONFIG_Y_TABLE - 1;

	sprintf(msg, "xy = %-7d", u);
    textout(screen, font, msg, CONFIG_WIDTH-180, CONFIG_HEIGHT-20,
    makecol(128, 128, 128));
      
	sprintf(msg, ",%-7d", v);
	textout(screen, font, msg, CONFIG_WIDTH-100, CONFIG_HEIGHT-20,
    makecol(128, 128, 128));

	show_mouse(screen);
      
 /*get_mouse_mickeys(&mickeyx, &mickeyy);*/
 
 u=mouse_x; v=mouse_y;
 /*u=mickeyx;
 v=mickeyy;*/
 if (mouse_b==1) if (dna[(u/4)][(v/4)]==NEUTRAL) {
	  if (s==HUMAN) {  
	   	dna[(u/4)][(v/4)]=s;

      	show_mouse(NULL);
      	bunka((u/4),(v/4),s+8);
      	show_mouse(screen);  
      	// only two players
	  	s++; if (s==3) s=1;
	}
 }
 	// if Player is Computer s=2
	if (s==COMPUTER) { 
		u=random()%CONFIG_X_TABLE; 
		v=random()%CONFIG_Y_TABLE; 
		
		while (dna[(u)][(v)]!=NEUTRAL) {
			u=random()%CONFIG_X_TABLE; 
			v=random()%CONFIG_Y_TABLE; 
		}
		
		dna[(u)][(v)]=s;
   		show_mouse(NULL);
   		bunka((u),(v),s+8);
   		show_mouse(screen);

		//ResetTimer();

		//WaitingTime = speed_counter + 100;
  		//do { 
  		//	prepocet();
      	//} while (speed_counter<WaitingTime);
		s++;
		if (s==3) s=1;
	}
	
	// small delay for not to fast play game
	ResetTimer();
	Delay(3);

 h++;

}

// X--Z, y
int create_hdna(int x, int z, int y) {
	int r;
	
	for (r = x; r < z; r++)
		dna[r][y]=OUTSIDE;
}

// x, Y||Z
int create_vdna(int x, int y, int z) {
	int r;
	
	for (r = y; r < z; r++)
		dna[x][r]=OUTSIDE;
}

int create_level() {
	//int u,v;
	//for (r = 1; r < CONFIG_WIDTH/5; r++)
	//	for (u = 1; u < CONFIG_X_TABLE/2; u++)
	//		dna[10+u][v*10]=OUTSIDE;
	
	create_vdna(20, 20, 100);
	create_vdna(40, 120, 200);
	create_vdna(60, 20, 100);
	create_vdna(80, 120, 200);
	create_vdna(100, 20, 100);
	create_vdna(120, 120, 200);
	create_vdna(140, 20, 100);
	create_vdna(160, 120, 200);
	create_vdna(180, 20, 100);
	create_vdna(200, 120, 200);
	create_vdna(220, 20, 100);
	create_vdna(240, 120, 200);
	create_vdna(260, 20, 100);
	create_vdna(280, 120, 200);
}

void main(void)
{
  int c, state;

  Init_Game();
    
  /* start up the MIDI file */
  if (the_music) play_midi(the_music, TRUE);

  /*dna[50][50]=1;*/
  /* start up the MIDI file */
  //play_midi(the_music, TRUE);
  
  show_mouse(screen);
  
  /* Loop to run several games... */
  do {
  	clear(screen);
  		
  	//draw_sprite(screen,obr4, 10, 450);
  	textprintf(screen,font,40,CONFIG_HEIGHT-20,15, 
  	"Bakteria | The Bactery 2002-2012  MIT License");
  	textprintf(screen,font,100,CONFIG_HEIGHT-10,7,"by Lukas 'luko' Veselovsky");

	TXT_CURSOR_X = CONFIG_WIDTH/2-100;
  	TXT_CURSOR_Y = CONFIG_HEIGHT-70;
  	
  	state = 0;
  	
  	hh=8; stage_init();
  	
  	s=1; 

	create_level();

  	do {    
    	player_action();

    	stage_recalc();

		if (end_test()==0) {
			draw_info();
			state=-1;	
		}
    	
    	poll_mouse();
    	
    	if (keypressed()) c=readkey();
    	
	} while ((state!=-1)&&(!key[KEY_ESC]));    
	
  	// small delay for test if you want exit from playing game
	ResetTimer();
	Delay(150);
  } while (!key[KEY_ESC]);    
  
  Done_Game();
}
END_OF_MAIN();
/* Tento riadok MUSI byt v kazdej Linuxovej verzii!*/


