#ifdef __HELIOS
#define char int
#endif

typedef struct {

    char    type;
    char    attack;
    char    range;
    char    defend;
    char    movement;
    char    range_to_ogre;
    char    fired;
    char    moves_left;
    char    status;
    char    l_hex;
    char    r_hex;

} UNIT;

typedef struct {

    char    missiles;
    char    main_bats;
    char    sec_bats;
    char    ap;
    char    treads;
    char    movement;
    char    moves_left;
    char    l_hex;
    char    r_hex;
    char    init_treads;
    char    where;

} OGRE;

/* unit types */

#define CP          'C'
#define HVYTANK     'T'
#define MSLTANK     'M'
#define GEV         'G'
#define HOWITZER    'H'
#define INFANTRY    'I'


/* unit statuses */
#define OK          1
#define DISABLED    2
#define DESTROYED   3

/* directions */
#define RIGHT       'D'
#define UPRIGHT     'E'
#define DOWNRIGHT   'X'
#define LEFT        'A'
#define UPLEFT      'W'
#define DOWNLEFT    'Z'
#define SIT         'S'
#define PASS        'P'
#define REDRAW      '\014'
#define BEEP	    '\07'

#define TRUE        1
#define FALSE       0

#define N_UNITS     47

#define DEF_MISSILES    3
#define DEF_MAIN        4
#define DEF_SECONDARY   3
#define DEF_AP          1

#define ATK_MISSILES    6
#define ATK_MAIN        4
#define ATK_SECONDARY   3
#define ATK_AP          1

#define RANGE_MISSILES      5  
#define RANGE_MAIN          3
#define RANGE_SECONDARY     2
#define RANGE_AP            1

#ifdef __HELIOS
#undef char
#endif
