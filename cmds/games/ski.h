/*                                  S K I !
**
**                       Version 6.0 of October 14, 1990
**
**           Copyright 1983-1990 by Mark Stevans for Roy's Games, Inc.
*/

/*
** Utility macros.
*/

#define EOS	'\0'
#define NIL(X)	((X) NULL)

#define PUBLIC
#define PRIVATE		static

#define SUCCESS		0
#define FAILURE		-1

#define NO_POSITION	-1
#define PROB(X)		(((RANDOM_GENERATE() % 10000) / 100.0) <= (X))
#define ABS(X)		(((X) >= 0) ? (X) : - (X))
#define EXISTS(X)	((X) >= 0)

/*
** The representations used for each object.
*/

#define REP_SNOW	'.'
#define REP_TREE	'Y'
#define REP_PLAYER	'I'
#define REP_GROUND	' '
#define REP_ICE		'#'
#define REP_SNOMAN	'A'
#define REP_ICBM	'*'
#define REP_DEMON	'D'

/*
** "LINE_LEN" defines the length of a screen line.
*/

#define LINE_LEN				70

/*
** "MIN_SNOMAN_APPEARANCE_DISTANCE" is the minimum distance from the player
** at which a new Snoman may appear.
*/

#define MIN_SNOMAN_APPEARANCE_DISTANCE		3

/*
** "LEVEL_MULTIPLIER" is a constant multiplied into the first element of the
** cellular growth automaton probability array with each passing level.
*/

#define LEVEL_MULTIPLIER			1.01

/*
** "MAX_HORIZONTAL_PLAYER_SPEED" defines the absolute value of the maximum
** horizontal player speed.
*/

#define MAX_HORIZONTAL_PLAYER_SPEED			5

/*
** "PROB_SKIS_MELT_SNOMAN" is the probability that the player's jet-powered
** skis will melt the Snoman during each turn in which the player is jumping
** over the Snoman.
*/

#define PROB_SKIS_MELT_SNOMAN			20.0

/*
** "PROB_SPONTANEOUS_MELT" is the probability that the Snoman melts
** spontaneously during any given turn.
*/

#define PROB_SPONTANEOUS_MELT			1.0

/*
** "ICBM_SPEED" is the horizontal speed of an ICBM.
*/

#define ICBM_SPEED		3

/*
** "ICBM_RANGE" is the horizontal Snoman lethality range of the ICBM.
*/

#define ICBM_RANGE		2

/*
** "DEMON_RANGE" is the horizontal Snoman lethality range of the demon.
*/

#define DEMON_RANGE		1

/*
** "ICBM_SPEED" is the maximum horizontal speed of an ICBM.
*/

#define DEMON_SPEED		1

/*
** "PROB_BAD_SPELL" is the probability that the "Incant Fire Demon" spell will
** be bad during each incantation.
*/

#define PROB_BAD_SPELL		10.0

/*
** "PROB_BAD_TELEPORT" is the probability that the teleportation device will
** fail to safely teleport the player during each application.
*/

#define PROB_BAD_TELEPORT	10.0

/*
** "PROB_BAD_ICBM" is the probability that a nuclear ICBM will detonate in
** the player's backpack during each launch.
*/

#define PROB_BAD_ICBM		30.0

/*
** "PROB_SLIP_ON_ICE" is the probability that the player will slip on the ice
** and fall down for each turn during which the player is skiiing on ice.
*/

#define PROB_SLIP_ON_ICE	2.0

/*
** "PROB_FALL_ON_GROUND" is the probability that the player will fall down
** for each turn during which the player is skiiing on bare ground.
*/

#define PROB_FALL_ON_GROUND	10.0

/*
** "PROB_HIT_TREE" is the probability that the player will hit a tree in
** each turn during which the player is skiiing past a tree.
*/

#define PROB_HIT_TREE		25.0

/*
** "PROB_BAD_LANDING" is the probability that the player will land badly and
** fall down during a landing from a jump or hop.
*/

#define PROB_BAD_LANDING	3.0

/*
** "POINTS_PER_JUMP" is the number of points awarded to the player for the
** successful completion of one jump.  For scoring purposes, a hop is
** considered to consist of exactly one half-jump.
*/

#define POINTS_PER_JUMP			20.0

/*
** "POINTS_PER_METER" is the number of points awarded to the player for each
** meter of horizontal or vertical motion during each turn.
*/

#define POINTS_PER_METER		1.0

/*
** "POINTS_PER_MELTED_SNOMAN" is the number of points awarded to the player
** for each Snoman that melts during the course of the game, irregardless of
** whether the player passively caused the Snoman to melt by luring him from a
** Snobank, or actively melted the Snoman using his skis, an ICBM, or with
** the assistance of the Fire Demon.
*/

#define POINTS_PER_MELTED_SNOMAN	100.0

/*
** "POINTS_PER_INJURY_DEGREE"
*/

#define POINTS_PER_INJURY_DEGREE	-40.0

/*
** The injury categories.
*/

#define SLIGHT_INJURY		0
#define MODERATE_INJURY		3
#define SEVERE_INJURY		6

/*
** The randomness of injury degrees.
*/

#define INJURY_RANDOMNESS	6

/*
** "LEVEL" describes a playing level.
*/

typedef struct {
	short meters_travelled;
	short jump_count;
	short level_num;
	short num_snomen_melted;
	float num_jumps_attempted;
	short player_pos;
	short snoman_pos;
	short icbm_pos;
	short demon_pos;
	short player_speed;
	char slope[LINE_LEN + 1];
} LEVEL;

/*
** Declarations of common library functions.
*/

extern long time();

/*
** Random number generator definitions
*/

#ifdef __HELIOS
#define USE_RAND
#endif

#ifdef USE_RAND

extern int rand();

#define RANDOM_INITIALIZE	srand
#define RANDOM_GENERATE		rand

#else

extern long random();

#define RANDOM_INITIALIZE	srandom
#define RANDOM_GENERATE		random

#endif
