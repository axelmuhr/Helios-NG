/*                                  S K I !
**
**                       Version 6.0 of October 14, 1990
**
**           Copyright 1983-1990 by Mark Stevans for Roy's Games, Inc.
*/

#include <stdio.h>
#include <ctype.h>
#include "ski.h"

/*
**
** Constants controlling the multiple cellular growth automatons that are
** executed in parallel to generate hazards.
**
** Each cellular growth automaton probability element tends to control a
** different facet of hazard generation:
**
**    [0] appearance of new hazards in clear snow
**    [1] hazards edge growth
**    [2] hazards edge stability
**    [3] appearance of holes in solid hazards (this allows the Snoman to
**        work his way through forests)
*/

PRIVATE float prob_tree[] = { 0.0, 30.0, 70.0, 90.0 };
PRIVATE float prob_ice[] = { 0.0, 30.0, 70.0, 90.0 };
PRIVATE float prob_ground[] = { 0.0, 30.0, 70.0, 90.0 };
PRIVATE float prob_snoman_appearance;

/*
** "init" performs various initialization functions.
*/

PRIVATE int init(level_ptr)
LEVEL *level_ptr;
{
	register int i;

	/*
	** Print the game version.
	*/

	printf("SKI!  Version 6.0!\n");

	/*
	** Initialize the random number generator.
	*/

	RANDOM_INITIALIZE((int) time(NIL(long *)));

	/*
	** Initialize the player's position.
	*/

	level_ptr->player_pos = RANDOM_GENERATE() % LINE_LEN;

	/*
	** Initialize the current level.
	*/

	for (i = 0; i < LINE_LEN; ++i)
		level_ptr->slope[i] = REP_SNOW;

	level_ptr->slope[LINE_LEN] = EOS;

	level_ptr->meters_travelled = 0;
	level_ptr->jump_count = -1;
	level_ptr->level_num = 0;
	level_ptr->num_snomen_melted = 0;
	level_ptr->num_jumps_attempted = 0.0;
	level_ptr->player_pos = RANDOM_GENERATE() % LINE_LEN;
	level_ptr->snoman_pos = NO_POSITION;
	level_ptr->icbm_pos = NO_POSITION;
	level_ptr->demon_pos = NO_POSITION;
	level_ptr->player_speed = 0;

	/*
	** Randomize the appearance probabilities.
	*/

	prob_tree[0] = (RANDOM_GENERATE() % 100) / 500.0 + 0.05;
	prob_ice[0] = (RANDOM_GENERATE() % 100) / 500.0 + 0.05;
	prob_ground[0] = (RANDOM_GENERATE() % 100) / 500.0 + 0.05;
	prob_snoman_appearance = (RANDOM_GENERATE() % 100) / 25.0 + 1.0;

	/*
	** Return a code indicating successful completion.
	*/

	return (SUCCESS);
}

/*
** "main" is the top level function of the game.
*/

PUBLIC int main()
{
	LEVEL level;

	/*
	** Initialize the game.
	*/

	if (init(&level) != SUCCESS)
		exit(1);

	/*
	** Perform the game loop until the game is over.
	*/

	for (;;) {
		draw_picture(&level);
		do_user(&level);
		manipulate_objects(&level);
		update_level(&level);
	}
}

/*
** "gen_next_slope" generates the slope of the next level, dependent upon
** the characteristics of the current level, the probabilities, and the
** position of the player.
*/

PRIVATE int gen_next_slope(current_level, next_level)
LEVEL *current_level;
LEVEL *next_level;
{
	register int i;
	register char *current_slope;
	register char *next_slope;
	register int player_pos;
	short num_nearby_trees, num_nearby_ice, num_nearby_ground;

	/*
	** Cache some convenient values.
	*/

	current_slope = current_level->slope;
	next_slope = next_level->slope;
	player_pos = current_level->player_pos;

	/*
	** Generate each character of the next level.
	*/

	for (i = 0; i < LINE_LEN; ++i) {
		/*
		** Count the number of nearby trees, ice patches, and
		** ground patches on the current level.
		*/

		num_nearby_trees = 0;
		num_nearby_ice = 0;
		num_nearby_ground = 0;

		if (current_slope[i] == REP_TREE)
			++num_nearby_trees;
		if (current_slope[i] == REP_ICE)
			++num_nearby_ice;
		if (current_slope[i] == REP_GROUND)
			++num_nearby_ground;

		if (i > 0) {
			if (current_slope[i - 1] == REP_TREE)
				++num_nearby_trees;
			else if (current_slope[i - 1] == REP_ICE)
				++num_nearby_ice;
			else if (current_slope[i - 1] == REP_GROUND)
				++num_nearby_ground;
		}

		if (i < (LINE_LEN - 1)) {
			if (current_slope[i + 1] == REP_TREE)
				++num_nearby_trees;
			else if (current_slope[i + 1] == REP_ICE)
				++num_nearby_ice;
			else if (current_slope[i + 1] == REP_GROUND)
				++num_nearby_ground;
		}

		/*
		** Generate this character of the next level based upon
		** the characteristics of the nearby characters on the
		** current level.
		*/

		if (PROB(prob_tree[num_nearby_trees]) &&
				((i != player_pos) || (num_nearby_trees > 0)))
			next_slope[i] = REP_TREE;
		else if (PROB(prob_ice[num_nearby_ice]) &&
				((i != player_pos) || (num_nearby_ice > 0)))
			next_slope[i] = REP_ICE;
		else if (PROB(prob_ground[num_nearby_ground]) &&
				((i != player_pos) || (num_nearby_ground > 0)))
			next_slope[i] = REP_GROUND;
		else
			next_slope[i] = REP_SNOW;
	}

	/*
	** Terminate the slope string.
	*/

	next_slope[LINE_LEN] = EOS;

	/*
	** Return a code indicating successful completion.
	*/

	return (SUCCESS);
}

/*
** "update_level" moves to the next level.
*/

PRIVATE int update_level(current_level)
LEVEL *current_level;
{
	LEVEL next_level;

	/*
	** Go to the next level, and move the player.
	*/

	++current_level->level_num;

	current_level->meters_travelled += ABS(current_level->player_speed) + 1;

	/*
	** Figure out the new player position based on a modulo
	** addition.  Note that we must add the line length into the
	** expression before taking the modulus to make sure that we
	** are not taking the modulus of a negative integer.
	*/

	current_level->player_pos = (current_level->player_pos +
			current_level->player_speed + LINE_LEN) % LINE_LEN;

	/*
	** Generate the updated slope.
	*/

	gen_next_slope(current_level, &next_level);

	strcpy(current_level->slope, next_level.slope);

	/*
	** If the player was jumping, decrement the jump count.
	*/

	if (current_level->jump_count >= 0)
		--current_level->jump_count;

	/*
	** If there is no Snoman, one might be created.
	*/

	if (!EXISTS(current_level->snoman_pos)) {
		if (PROB(prob_snoman_appearance)) {
			/*
			** Make sure that the Snoman does not appear too
			** close to the player.
			*/

			do {
				current_level->snoman_pos = RANDOM_GENERATE() % LINE_LEN;
			} while (ABS(current_level->snoman_pos -
					current_level->player_pos) <=
					MIN_SNOMAN_APPEARANCE_DISTANCE);
		}
	}

	/*
	** Increase the initial appearance probabilities of all obstacles and
	** the Snoman.
	*/

	prob_tree[0] *= LEVEL_MULTIPLIER;
	prob_ice[0] *= LEVEL_MULTIPLIER;
	prob_ground[0] *= LEVEL_MULTIPLIER;
	prob_snoman_appearance *= LEVEL_MULTIPLIER;

	/*
	** Return a code indicating successful completion.
	*/

	return (SUCCESS);
}
