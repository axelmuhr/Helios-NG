/*
    OGRE: a tactical ground combat game set in 2085.

    Adapted from the Metagaming Microgame by Steve Jackson.

    This version was written for a Vax 11/780 under Unix
    by Michael Caplinger, Rice University, February-March 1982.

    Paper game (c) 1977 by Steve Jackson
    This implementation (c) 1982, 1984 by Michael Caplinger
*/

#include <signal.h>
#include <ctype.h>

#define MAIN
#include "ext.h"

main(argc, argv)
char **argv;
{

    int handler();
    int mark;

    signal(SIGINT, handler);

    if(argc > 1)
        switch(argv[1][0]) {

            case '3':
                mark = 3;
                break;

            case '5':
                mark = 5;
                break;

            default:
                mark = 3;
                break;
        }

    else mark = 3;

    set_term();
    srand(time(0));
    init_units(mark);
    init_ogre(mark);
    disp_ogre_status(TRUE);

    while(1) {

        init_round();

        /* The Ogre fires. */
        assign_fire_ogre();
        check_over(); 

        /* Player moves, and fires. */
        move_def();
        attack_def();

        /* Let the GEVs move their extra 3 turns. */
        init_gev2();
        move_def();

        /* The Ogre moves. */
        move_ogre();
        check_over(); 

    }

}

handler() {

    clear_screen();
    reset_term();
    exit(0);

}

/*
    Get a character. If it's a ^L, redraw the screen.
*/

readchar() {
	int	c ;

	while ((c = getchar()) == REDRAW) redraw_screen() ;
	return (islower(c) ? toupper(c) : c) ;
	}

/*
    See if the game is over, and die if it is.
*/
check_over()
{
    char *message;
    int over;

    over = FALSE;

    if(unit[0].status == DESTROYED) {
        message = "The Ogre wins!!";
        over = TRUE;
    }
    if(ogre.movement == 0) {
        message = "You win!!";
        over = TRUE;
    }

    if(over) {
        clear_screen();
        reset_term();
        printf("%s\n", message);
        exit(0);
    }

}
