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
** "injuries" contains the array of injury descriptions.
*/

PRIVATE char *injuries[] = {
	"However, you escaped injury!", 
	"But you weren't hurt at all!", 
	"But you only got a few scratches.", 
	"You received some cuts and bruises.", 
	"You wind up with a concussion and some contusions.", 
	"You now have a broken rib.", 
	"Your left arm has been fractured.",
	"You suffered a broken ankle.",
	"You have a broken arm and a broken leg.", 
	"You have four broken limbs and a cut!", 
	"You broke every bone in your body!", 
	"I'm sorry to tell you that you have been killed....",
	NIL(char *)
};

/*
** "draw_picture"
*/

PUBLIC int draw_picture(current_level)
LEVEL *current_level;
{
	char level_picture[LINE_LEN + 1];

	/*
	** Create a picture of the current level.
	*/

	strcpy(level_picture, current_level->slope);

	level_picture[current_level->player_pos] = REP_PLAYER;

	if (EXISTS(current_level->snoman_pos))
		level_picture[current_level->snoman_pos] = REP_SNOMAN;
	if (EXISTS(current_level->icbm_pos))
		level_picture[current_level->icbm_pos] = REP_ICBM;
	if (EXISTS(current_level->demon_pos))
		level_picture[current_level->demon_pos] = REP_DEMON;

	/*
	** Print the picture of the current level.
	*/

	printf("%4d %s ", current_level->level_num, level_picture);

	fflush(stdout);

	/*
	** Return a code indicating successful completion.
	*/

	return (SUCCESS);
}

/*
** "do_user"
*/

PUBLIC int do_user(current_level)
LEVEL *current_level;
{
	/*
	** If we are jumping, just finish the line.  Otherwise, check for
	** obstacles, and do a command.
	*/

	if (current_level->jump_count >= 0)
		putchar('\n');
	else {
		check_obstacles(current_level);
		do_command(current_level);
	}
}

/*
** "manipulate_objects"
*/

PUBLIC int manipulate_objects(current_level)
LEVEL *current_level;
{
	/*
	** If there is a Snoman, the player's jet-powered skis may melt him,
	** or he may spontaneously melt.
	*/

	if (EXISTS(current_level->snoman_pos)) {
		if (((ABS(current_level->snoman_pos - current_level->player_pos) <= 1) &&
				PROB(PROB_SKIS_MELT_SNOMAN)) ||
				PROB(PROB_SPONTANEOUS_MELT)) {
			current_level->snoman_pos = NO_POSITION;
			++current_level->num_snomen_melted;
		}
	}

	/*
	** If there is a Snoman, move him towards the player.  If there is a		** tree in the way, the Snoman is blocked.
	*/

	if (EXISTS(current_level->snoman_pos)) {
		if (current_level->snoman_pos < current_level->player_pos) {
			if (current_level->slope[current_level->snoman_pos + 1] != REP_TREE)
				++current_level->snoman_pos;
		}

		else {
			if (current_level->slope[current_level->snoman_pos - 1] != REP_TREE)
				--current_level->snoman_pos;
		}
	}

	/*                  
	** If there is an ICBM, handle it.
	*/

	if (EXISTS(current_level->icbm_pos)) {
		/*
		** If there is a Snoman, move the ICBM towards him.  Else,
		** self-destruct the ICBM.
		*/

		if (EXISTS(current_level->snoman_pos)) {
			if (current_level->icbm_pos < current_level->snoman_pos)
				current_level->icbm_pos += ICBM_SPEED;
			else
				current_level->icbm_pos -= ICBM_SPEED;
		}
		else
			current_level->icbm_pos = NO_POSITION;
	}

	/*
	** If there is a fire demon on the level, handle it.
	*/

	if (EXISTS(current_level->demon_pos)) {
		/*
		** If there is a Snoman on the current level, move the demon
		** towards him.  Else, the demon might decide to leave.
		*/

		if (EXISTS(current_level->snoman_pos)) {
			if (current_level->demon_pos < current_level->snoman_pos)
				current_level->demon_pos += DEMON_SPEED;
			else
				current_level->demon_pos -= DEMON_SPEED;
		}
		else {
			if (PROB(25.0))
				current_level->demon_pos = NO_POSITION;
		}
	}

	/*
	** If there is a Snoman and an ICBM on the slope, the Snoman
	** might get melted.
	*/

	if (EXISTS(current_level->snoman_pos) && EXISTS(current_level->icbm_pos)) {
		if (ABS(current_level->snoman_pos - current_level->icbm_pos) <= ICBM_RANGE) {
			current_level->icbm_pos = NO_POSITION;
			current_level->snoman_pos = NO_POSITION;
			++current_level->num_snomen_melted;
		}
	}

	/*
	** If there is a Snoman and a fire demon, he might get melted.
	*/

	if (EXISTS(current_level->snoman_pos) && EXISTS(current_level->demon_pos)) {
		if (ABS(current_level->snoman_pos - current_level->demon_pos) <= 1) {
			current_level->snoman_pos = NO_POSITION;
			++current_level->num_snomen_melted;
		}
	}
}

/*
** "accident" is called when the player gets into an accident, which ends the
** game.  "msg" is the description of the accident type, and "severity" is
** the severity.  This function should never return.
*/

PRIVATE int accident(current_level, msg, severity)
LEVEL *current_level;
char *msg;
short severity;
{
	int degree;
	long score;

	/*
	** Compute the degree of the player's injuries.
	*/

	degree = severity + (RANDOM_GENERATE() % INJURY_RANDOMNESS);

	/*
	** Print a message indicating the termination of the game.
	*/

	printf("\n\n%s  %s\n\n", msg, injuries[degree]);

	/*
	** Print the statistics of the game.
	*/

	printf("You skiied %d meters with %.1f jumps and melted %d Snowm%cn.\n",
			current_level->meters_travelled,
			current_level->num_jumps_attempted,
			current_level->num_snomen_melted,
			(current_level->num_snomen_melted == 1) ? 'a' : 'e');

	/*
	** Initially calculate the player's score based upon the number of
	** meters travelled.
	*/

	score = current_level->meters_travelled * POINTS_PER_METER;

	/*
	** Add bonus points for the number of jumps completed.
	*/

	score += current_level->num_jumps_attempted * POINTS_PER_JUMP;

	/*
	** Add bonus points for each Snoman that melted during the course of
	** the game.
	*/

	score += current_level->num_snomen_melted * POINTS_PER_MELTED_SNOMAN;

	/*
	** Subtract a penalty for the degree of injury experienced by the
	** player.
	*/

	score += degree * POINTS_PER_INJURY_DEGREE;

	/*
	** Print the player's score.
	*/

	printf("Your score for this run is %ld.\n", score);

	/*
	** Exit the game with a code indicating successful completion.
	*/

	exit(0);
}

/*
** "check_obstacles"
*/

PRIVATE int check_obstacles(current_level)
LEVEL *current_level;
{
	short player_pos;
	char *slope;

	player_pos = current_level->player_pos;
	slope = current_level->slope;

	/*
	** If we are just landing after a jump, we might fall down.
	*/

	if ((current_level->jump_count == 0) && PROB(PROB_BAD_LANDING))
		accident(current_level, "Whoops!  A bad landing!",
				SLIGHT_INJURY);

	/*
	** If there is a tree in our position, we might hit it.
	*/

	if ((slope[player_pos] == REP_TREE) && PROB(PROB_HIT_TREE))
		accident(current_level, "Oh no!  You hit a tree!",
				SEVERE_INJURY);

	/*
	** If there is bare ground under us, we might fall down.
	*/

	if ((slope[player_pos] == REP_GROUND) && PROB(PROB_FALL_ON_GROUND))
		accident(current_level, "You fell on the ground!",
				MODERATE_INJURY);

	/*
	** If we are on ice, we might slip.
	*/

	if ((slope[player_pos] == REP_ICE) && PROB(PROB_SLIP_ON_ICE))
		accident(current_level, "Oops!  You slipped on the ice!",
				SLIGHT_INJURY);

	/*
	** If there is a Snoman next to us, he may grab us.
	*/

	if (EXISTS(current_level->snoman_pos) &&
			(ABS(current_level->snoman_pos - player_pos) <= 1))
		accident(current_level, "Yikes!  The Snoman's got you!",
				MODERATE_INJURY);
}

/*
** "do_command" gets a command from the player and performs it.
*/

PRIVATE int do_command(current_level)
LEVEL *current_level;
{
	register int player_pos;
	char command_buf[LINE_LEN + 1];
	short command_key;

	player_pos = current_level->player_pos;

	/*
	** Print a prompt, and read a command.
	*/

	printf("? "); fflush(stdout);

	fgets(command_buf, sizeof command_buf, stdin);

	command_key = command_buf[0];

	if (islower(command_key))
		command_key = toupper(command_key);

	switch (command_key) {
	case 'R': /* Move right */
		if ((current_level->slope[player_pos] != REP_ICE) &&
				(current_level->player_speed < MAX_HORIZONTAL_PLAYER_SPEED))
			++current_level->player_speed;

		break;

	case 'L': /* Move left */
		if ((current_level->slope[player_pos] != REP_ICE) &&
				(current_level->player_speed > -MAX_HORIZONTAL_PLAYER_SPEED))
			--current_level->player_speed;

		break;

	case 'J': /* jump */
		current_level->jump_count = (RANDOM_GENERATE() % 6) + 4;
		current_level->num_jumps_attempted += 1.0;
		break;

	case 'H': /* Do a hop */
		current_level->jump_count = (RANDOM_GENERATE() % 3) + 2;
		current_level->num_jumps_attempted += 0.5;
		break;

	case 'T': /* Attempt teleportation */
		if (PROB(PROB_BAD_TELEPORT))
			accident(current_level,
					"You materialized 25 feet in the air!",
					SLIGHT_INJURY);

		current_level->player_pos = RANDOM_GENERATE() % LINE_LEN;
		break;

	case 'I': /* Launch backpack ICBM */
		if (PROB(PROB_BAD_ICBM))
			accident(current_level,
					"Nuclear blast in your backpack!",
					SEVERE_INJURY);

		current_level->icbm_pos = player_pos;
		break;

	case 'D': /* Incant spell for fire demon */
		if (PROB(PROB_BAD_SPELL))
			accident(current_level,
					"A bad spell -- the demon grabs you!",
					MODERATE_INJURY);

		current_level->demon_pos = RANDOM_GENERATE() % LINE_LEN;
		break;

	default:
		break;
	}
}
