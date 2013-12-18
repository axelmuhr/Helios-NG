/*
    Handle the Ogre status display.
*/

#include "ext.h"

disp_ogre_status(redraw)

/* If redraw is false, the display is not touched if nothing has changed. */
int redraw;
{
    static OGRE last;

    /*
        The Ogre status display occupies the bottom 6 lines of the display.
    */

    /*               0        1         2         3         4
                     1234567890123456789012345678901234567890       */

    if(redraw || last.main_bats != ogre.main_bats)
        if(ogre.main_bats > 0)
        display(18, "Main Batteries:      %d (4/3 D4)", ogre.main_bats);
        else display(18, " ");

    if(redraw || last.sec_bats != ogre.sec_bats)
        if(ogre.sec_bats > 0)
        display(19, "Secondary Batteries: %d (3/2 D3)", ogre.sec_bats);
        else display(19, " ");

    if(redraw || last.missiles != ogre.missiles)
        if(ogre.missiles > 0)
        display(20, "Missiles:            %d (6/5 D3)", ogre.missiles);
        else display(20, " ");

    if(redraw || last.ap != ogre.ap)
        if(ogre.ap > 0)
        display(21, "Anti-personnel:     %2d (1/1 D1)", ogre.ap);
        else display(21, " ");

    if(redraw || last.treads != ogre.treads)
        if(ogre.treads > 0)
        display(22, "Treads:             %2d (1/* D1)", ogre.treads);
        else display(22, " ");

    if(redraw || last.movement != ogre.movement)
        display(23, "Movement:            %d", ogre.movement);

    copy(&last, &ogre, sizeof(last));

}

copy(to, from, size)
char *to, *from;
int size;
{
    int i;

    for(i = 0; i < size; i++) to[i] = from[i];

}
