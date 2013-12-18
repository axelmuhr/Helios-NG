/*
    Resolve all attacks and rams from both directions.

    Michael Caplinger, Rice University, March 1982.
*/

#include "ext.h"

static char *odd_names[] = {
    "0/1",
    "1/2",
    "1/1",
    "2/1",
    "3/1",
    "4/1",
    "+",
};

static char crt[6][7] = {

    OK, OK,        OK,        OK,        DISABLED,  DISABLED,  DESTROYED,
    OK, OK,        OK,        DISABLED,  DISABLED,  DESTROYED, DESTROYED,
    OK, OK,        DISABLED,  DISABLED,  DESTROYED, DESTROYED, DESTROYED,
    OK, OK,        DISABLED,  DESTROYED, DESTROYED, DESTROYED, DESTROYED,
    OK, DISABLED,  DESTROYED, DESTROYED, DESTROYED, DESTROYED, DESTROYED,
    OK, DESTROYED, DESTROYED, DESTROYED, DESTROYED, DESTROYED, DESTROYED,

};

odds(attack, defend)
int attack, defend;
{
    int result;

    result = (defend > 0) ? attack / defend + 1 : 6;

    if(result > 6) result = 6;

    if(result == 1)
        result = (2 * attack < defend) ? 0 : 1;

    return(result);

}

char *odd_str(attack, defend)
int attack, defend;
{

    return(odd_names[odds(attack, defend)]);

}


/* Resolve all attacks on the Ogre. */
ogre_resolve(allocations)
OGRE *allocations;
{

    display(16, "Resolving..."); cycle();

    if(allocations -> missiles > 0) {
        if(crt[roll()][odds(allocations -> missiles, DEF_MISSILES)] ==
            DESTROYED) ogre.missiles -= 1;
    }

    if(allocations -> main_bats > 0) {
        if(crt[roll()][odds(allocations -> main_bats, DEF_MAIN)] ==
            DESTROYED) ogre.main_bats -= 1;
    }

    if(allocations -> sec_bats > 0) {
        if(crt[roll()][odds(allocations -> sec_bats, DEF_SECONDARY)] ==
            DESTROYED) ogre.sec_bats -= 1;
    }

    if(allocations -> ap > 0) {
        if(crt[roll()][odds(allocations -> ap, DEF_AP)] ==
            DESTROYED) ogre.ap -= 1;
    }

    if(allocations -> treads > 0) {
        if(crt[roll()][odds(1, 1)] == DESTROYED)
            decrease_treads(allocations -> treads);

    }

    /* erase the odds. */
    movecur(18, 40); eeol();
    movecur(19, 40); eeol();
    movecur(20, 40); eeol();
    movecur(21, 40); eeol();
    movecur(22, 40); eeol();

    /* update the Ogre status display. */
    disp_ogre_status(FALSE);
    check_over();
}

/* Resolve an Ogre attack on a defending unit. */
def_resolve(i)
{
    char result;

    if(unit[i].status != DESTROYED && unit[i].fired > 0) {

        result = crt[roll()][odds(unit[i].fired, unit[i].defend)];

        /* Infantry is a special case. */
        if(unit[i].type == INFANTRY) {
            if(result != OK)		/* Infantry is fragile */
		unit[i].status = DESTROYED;
            update_hex(unit[i].l_hex, unit[i].r_hex);
	    return;
            }

        switch(unit[i].status) {

            case OK:
                unit[i].status = result;
                break;

            case DISABLED:
                if(result != OK) unit[i].status = DESTROYED;
                break;

        }

        if(unit[i].status != OK) 
            update_hex(unit[i].l_hex, unit[i].r_hex);

    }

}

roll()
{

    return(rand() % 6);

}

/* Routine called for each hex the Ogre moves through, to handle rams. */
ogre_ram()
{
    int i, hit_infantry = FALSE;

    /* Rule 5.03 */
    for(i = 0; i < n_units; i++)
        if(unit[i].l_hex == ogre.l_hex &&
           unit[i].r_hex == ogre.r_hex &&
           unit[i].status != DESTROYED)

            switch(unit[i].type) {

                case INFANTRY:

                    /* Rule 5.04 */
                    if(ogre.ap > 0 && !hit_infantry) {
                        unit[i].status = DESTROYED ;
			hit_infantry = TRUE ;
                    }
                    break;

                default:

                    /* Rule 5.031 */
                    if(unit[i].movement == 0 || 
                       unit[i].status == DISABLED) {
                        unit[i].status = DESTROYED;
                        decrease_treads( (unit[i].type == HVYTANK) ? 2 : 1);
                        disp_ogre_status(FALSE);
                    }
                    else {
                        unit[i].status = (roll() > 3) ? DESTROYED : DISABLED;
                        decrease_treads( (unit[i].type == HVYTANK) ? 2 : 1);
                        disp_ogre_status(FALSE);
                    }
                    break;

            }


}

/* See if a defender has rammed the Ogre. */
def_ram(i)
int i;
{
    if(unit[i].l_hex == ogre.l_hex &&
       unit[i].r_hex == ogre.r_hex &&
       unit[i].type != INFANTRY) {

        /* Rule 5.036 */

        decrease_treads(1);
        unit[i].status = DESTROYED;
        disp_ogre_status(FALSE);
	disp_ogre() ;

    }

}

decrease_treads(attack)
int attack;
{

    /* Rule 6.05 */

    /* Now, where is the movement factor going? */

    ogre.treads  -= attack;
    ogre.movement = 0;
    if(ogre.treads > 0)  ogre.movement = 1;
    if(ogre.treads > ogre.init_treads / 3) ogre.movement = 2;
    if(ogre.treads > 2 * ogre.init_treads / 3) ogre.movement = 3;

}

