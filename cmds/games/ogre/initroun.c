#include "ext.h"

init_round()
{

    int i;

    for(i = 0; i < n_units; i++) {

        unit[i].moves_left = unit[i].movement;
        if(unit[i].status == DISABLED) {
            unit[i].status = OK;
            update_hex(unit[i].l_hex, unit[i].r_hex);
        }
        unit[i].range_to_ogre =
            range(ogre.l_hex, ogre.r_hex, unit[i].l_hex, unit[i].r_hex);

    }

}

init_move_ogre()
{

    int i;

    for(i = 0; i < n_units; i++) {

        unit[i].range_to_ogre =
            range(ogre.l_hex, ogre.r_hex, unit[i].l_hex, unit[i].r_hex);

    }

}

init_def_attack()
{

    int i;

    for(i = 0; i < n_units; i++) {

        if(unit[i].status == OK) {
            unit[i].fired = FALSE;
            unit[i].range_to_ogre =
                range(ogre.l_hex, ogre.r_hex, unit[i].l_hex, unit[i].r_hex);
        }

    }

}

init_ogre_attack()
{

    int i;

    for(i = 0; i < n_units; i++) {

            unit[i].fired = 0;
            unit[i].range_to_ogre =
                range(ogre.l_hex, ogre.r_hex, unit[i].l_hex, unit[i].r_hex);

    }

}

init_gev2()
{
    int i;

    for(i = 0; i < n_units; i++)
        if(unit[i].status == OK && unit[i].type == GEV)
            unit[i].moves_left = 3;

}
