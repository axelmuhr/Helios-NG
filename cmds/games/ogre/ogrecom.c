/*
    These routines define the Ogre's stategy (such as it is).
    There's lots of room for improvement here.
*/

#include "ext.h"

move_ogre()
{

    init_move_ogre();

    ogre.moves_left = ogre.movement;

    while(ogre.moves_left > 0) {
        move_ogre1();
        ogre_ram();
        cycle();
    }

}

#define INFINITY 32767

/* Move the Ogre one hex. */
move_ogre1()
{

    int weight[7];
    int i, max;
    char a, b;
    char olda, oldb;
/* dyt - prevent oscillation */
    static int osccnt;
    static int oscdir;
    static int mapdir[7] = { 0, 4, 5, 6, 1, 2, 3 };

    a = ogre.l_hex;
    b = ogre.r_hex;

    /* Collect weights for each possible move. These will be maximized. */

    weight[0] = - INFINITY; /* temp patch: getweight(a, b); */
    weight[1] = getweight(a - 1, b - 1);
    weight[2] = getweight(a - 1, b);
    weight[3] = getweight(a, b + 1);
    weight[4] = getweight(a + 1, b + 1);
    weight[5] = getweight(a + 1, b);
    weight[6] = getweight(a, b - 1);

/* dyt - prevent oscillation - decrease weight if returning to old position */
    weight[mapdir[oscdir]] -= 10 * osccnt;

    max = 0;
    for(i = 1; i < 7; i++)
        if(weight[i] > weight[max]) max = i;

/* dyt - record new direction */
    if (max == mapdir[oscdir])
     	  osccnt++;
    else
        osccnt = 0;
    oscdir = max;

/* display(17, "max %d weight %d cnt %d odir %d ndir %d",
	max, weight[max], osccnt, oscdir, mapdir[oscdir]);
   cycle(); */

    switch(max) {

        case 0:
            break;

        case 1:
            a--;
            b--;
            break;

        case 2:
            a--;
            break;

        case 3:
            b++;
            break;

        case 4:
            a++;
            b++;
            break;

        case 5:
            a++;
            break;

        case 6:
            b--;
            break;

    }

    olda = ogre.l_hex;
    oldb =  ogre.r_hex;

    ogre.l_hex = a;
    ogre.r_hex = b;

    update_hex(olda, oldb);

    disp_ogre();
    ogre.moves_left -= 1;

}

/*
    The weight for each hex is a measure of how desirable it is to be in that
    hex; the weights of the six possible directions are maximized.

    The primary consideration is distance to the CP; in the absence of other
    factors, the Ogre will take the fastest course to the CP.
    However, the Ogre will crush any unit it can (units with higher attacks
    chosen first) and moves towards units that are within range of its missiles
    or batteries.  It attempts to weight so that a concentration of dangerous,
    immobile units (like howitzers) is attacked first.

    Testing indicates that this isn't a bad strategy.
*/
getweight(a, b)
char a, b;
{
    int weight = 0;
    int total_attacks;
    int to_target;
    int i;

    total_attacks = ogre.missiles + ogre.main_bats + ogre.sec_bats;

    for(i = 1; i < n_units; i++) {

        if(unit[i].status == DESTROYED) continue;

        to_target = range(a, b, unit[i].l_hex, unit[i].r_hex);

        /*
             If you can crush somebody, do it.
             More dangerous units get crushed first.
        */
        if(to_target == 0) {
	    if(unit[i].type == CP) weight = 50;
            else weight = 10 * unit[i].attack;
            break;
        }

        if(total_attacks <= 0) continue;

        if(to_target <= RANGE_MISSILES && ogre.missiles > 0) {
            weight += unit[i].attack;
            weight += 4 - unit[i].movement;
            total_attacks -= 1;
            continue;
        }

        if(to_target <= RANGE_MAIN && ogre.main_bats > 0) {
            weight += unit[i].attack;
            weight += 4 - unit[i].movement;
            total_attacks -= 1;
            continue;
        }

        if(to_target <= RANGE_SECONDARY && ogre.sec_bats > 0) {
            weight += unit[i].attack;
            weight += 4 - unit[i].movement;
            total_attacks -= 1;
            continue;
        }

        if(to_target <= RANGE_AP && ogre.ap > 0 && 
	    (unit[i].type == INFANTRY || unit[i].type == CP)) {
            weight += unit[i].attack;
            weight += 4 - unit[i].movement;
            total_attacks -= 1;
            continue;
        }

    }

/* make moving towards the CP a goal even in the face of crushing things */
    weight += 40 *
        (range(ogre.l_hex, ogre.r_hex, unit[0].l_hex, unit[0].r_hex)
	    - range(a, b, unit[0].l_hex, unit[0].r_hex)) ;

    if(off_map(a, b) || blocked(a, b)) weight = - INFINITY;
/*
display(17, "%d %d weight %d", a, b, weight); cycle();
*/

    return(weight);

}

#define INCR(i) i = (i == n_units - 1) ? 0 : i + 1

#define MIN(a, b) (((a) > (b)) ? (a) : (b))

/* 
    Figure out who the Ogre will fire at. In this code, the "fired" element
    of the unit description is the number of hit points assigned against that
    unit.
*/
assign_fire_ogre()
{

    int i, unitno, nmissiles;

    init_ogre_attack();

    /*
        The basic strategy here is to fire at the next unit in range. Since
        the units are sorted by value, this will hopefully (although not 
        always) result in reasonable choices for targets.
        Experience indicates that the Ogre often overkills (which is OK)
        but fails to attack some valuable possibility (not OK).  Some
        work needs to be done here.
    */

    unitno = nextunit(RANGE_AP, 0);

/*
 * The difference between this and the board game - the board game only
 * lets the ogre AP a hex once per turn, as opposed to once per unit.
 */
    for(i = 0; i < ogre.ap; i++) {

        if(unit[unitno].range_to_ogre <= RANGE_AP &&
          (unit[unitno].type == CP || unit[unitno].type == INFANTRY)) {

            unit[unitno].fired += ATK_AP;
            display_attack("AP", unitno);

        }
        unitno = nextunit(RANGE_AP, unitno);

    }

    unitno = nextunit(RANGE_SECONDARY, unitno);

    for(i = 0; i < ogre.sec_bats; i++) {

        if(unit[unitno].range_to_ogre <= RANGE_SECONDARY) {

            unit[unitno].fired += ATK_SECONDARY;
            display_attack("secondary battery", unitno);

        }
        unitno = nextunit(RANGE_SECONDARY, unitno);

    }

    unitno = nextunit(RANGE_MAIN, unitno);

    for(i = 0; i < ogre.main_bats; i++) {

        if(unit[unitno].range_to_ogre <= RANGE_MAIN) {

            unit[unitno].fired += ATK_MAIN;
            display_attack("main battery", unitno);

        }
        unitno = nextunit(RANGE_MAIN, unitno);

    }

    unitno = nextunit(RANGE_MISSILES, unitno);

    nmissiles = ogre.missiles;

    for(i = 0; i < nmissiles; i++) {

        if(unit[unitno].status != DESTROYED &&
	    /* don't fire at infantry... 27 Oct 83 */
	    unit[unitno].type != INFANTRY && 
            unit[unitno].range_to_ogre <= RANGE_MISSILES) {

            unit[unitno].fired += ATK_MISSILES;
            ogre.missiles -= 1;
            display_attack("missile", unitno);
            disp_ogre_status(FALSE);

        }
        unitno = nextunit(RANGE_MISSILES, unitno);

    }

}

#include <stdio.h>
 
cycle()
{

    fflush(stdout);
    sleep(1);

}

/*
    Display and resolve an attack on a single defending unit.
*/
display_attack(weapon, target)
char *weapon;
int  target;
{

    /* No point if the unit is already destroyed. */
    if(unit[target].status == DESTROYED) return;

    display(16, "Ogre fires %s at unit at hex %d%d", weapon,
        unit[target].l_hex, unit[target].r_hex);

    movecur_hex(unit[target].l_hex, unit[target].r_hex);

    cycle();

    def_resolve(target);

}

nextunit(range, unitno)
int range;
int unitno;
{
    int start;

    start = unitno;
    INCR(unitno);
    while(unitno != start) {
        if(range == 1) {
            if(unit[unitno].status != DESTROYED &&
                (unit[unitno].type == CP || unit[unitno].type == INFANTRY) &&
                unit[unitno].range_to_ogre <= range)
                return(unitno);
        }
        else {
            if(unit[unitno].status != DESTROYED &&
                unit[unitno].range_to_ogre <= range)
                return(unitno);
        }
        INCR(unitno);
    }

    return(unitno);

}
