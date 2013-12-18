#include <stdio.h>
#include <graph.h>

void display_vc (struct videoconfig) ;

void test_mode (int mode_id, char *mode_name) ;

int main()
{

/* 
-- VGA modes 
*/

  test_mode (_VRES2COLOR   , "VRES2COLOR")   ; /* 17  640 x 480, BW */
  test_mode (_VRES16COLOR  , "VRES16COLOR")  ; /* 18  640 x 480, 16 color */
  test_mode (_MRES256COLOR , "MRES256COLOR") ; /* 19  320 x 200, 256 color */

/*
-- Max resolution & max colour (VGA)
-- Note : _MAXRESMODE  = _VRES16COLOR
          _MAXRESCOLOR = _MRES256COLOR
*/

  test_mode (_MAXRESMODE   , "MAXRESMODE")   ; /* -3  highest resolution */
  test_mode (_MAXCOLORMODE , "MAXCOLORMODE") ; /* -2  most colors */

  _setvideomode(_DEFAULTMODE);

  return 0;
}

void display_vc (struct videoconfig vc)
{
  printf("pixel width           = %d\n",vc.numxpixels);
  printf("pixel height          = %d\n",vc.numypixels);
  printf("text width            = %d\n",vc.numtextcols);
  printf("text height           = %d\n",vc.numtextrows);
  printf("number of colours     = %d\n",vc.numcolors);
  printf("bits per pixel        = %d\n",vc.bitsperpixel);
  printf("number of video pages = %d\n",vc.numvideopages);
  printf("mode                  = %d\n",vc.mode);
  printf("adapter               = %d\n",vc.adapter);
  printf("monitor               = %d\n",vc.monitor);
  printf("memory                = %d\n",vc.memory);
}

void test_mode (int mode_id, char *mode_name) 
{
  int ch ; 
  struct videoconfig vc;

  _setvideomode(mode_id);

  printf ("\nVideo Mode = %s\n\n", mode_name) ;
  fflush (stdout) ;
  _getvideoconfig(&vc);
  display_vc (vc) ;
  printf ("\nHit enter to continue ...") ;
  fflush (stdout) ;
  ch = getchar () ;
}
