static char ID[] = "@(#)maze.c	2.1 ";

/*
	maze, print a labarynth
	Usages:
		maze			# print a randomly selected maze
					# number printed at top is seed.
		maze <seed>		# print given maze with answer
		maze <seed> n		#print given maze, no answer
		maze <seed> d b		# debugging outputs
*/
#define	SIZE	38
static unsigned	seed = 0;
static int	answer = 0;
static int	b_answer = 0;
static int	debug = 0;
static char	a[ SIZE+2 ][ 2*SIZE+4 ];
static int	b[ SIZE+2 ][ SIZE+2 ];
static int	i, j;
static int	i_old, j_old;
static int	i_first, j_first;
#ifndef __HELIOS
static long	time();
#endif
static int	i_center, j_center;
static int	v;
static int	start, finish;
static int	backcount = 0;

main( argc, argv )
int	argc;
char	*argv[];
{
	seed = time((long *) 0);
	if( argc > 1 ) {
		seed = atoi( argv[1] );
		answer = 1;
	}
	printf("Maze seed is %u\n", seed );
	if( argc > 2 ) {
		while( *argv[2] )
			switch( *argv[2]++ ) {
				case 'b':		/* show backout paths */
					b_answer++;
					break;
				case 'n':		/* supress answers */
					answer = 0;
					b_answer = 0;
					break;
				case 'd':		/* debug output */
					debug++;
					break;
			}
	}

	srand( seed );
	for( i = 0; i < SIZE+2; i++)
		for( j = 0; j < 2*SIZE+4; j++)
			a[i][j] = ' ';
	for( j = 1; j < 2*SIZE+2; j++)
		a[0][j] = '_';
	for( i = 1; i < SIZE+1; i++)
		for( j = 1; j < 2*SIZE+2; j++)
			if( j%2 )
				a[i][j] = '|';
			else
				a[i][j] = '_';
	goodpath();
	badpath();
	if( debug )
		prtmaze();
	verybad();
	verybad();
	clean();
	prtmaze();
	exit( 0 );
}

prtmaze()
{
	int	ii, jj;
	for( ii = 0; ii < SIZE+2; ii++ ) {
		for( jj = 0; jj < SIZE+2; jj++ ) {
			if( b[ii][jj] == start )
				printf("S\b");
			else if( b[ii][jj] == finish )
				printf("F\b");
			else if( answer && b_answer && ii == i_center && jj == j_center )
				printf("+\b");
			else if( answer && b_answer && b[ii][jj] < -5000 )
				printf("~\b");
			else if( answer && b[ii][jj] > 0 )
				printf("*\b");
			printf("%c%c", a[ii][2*jj], a[ii][2*jj+1] );
			if( 0 < jj  &&  jj <= SIZE  &&
				( a[ii][2*jj+1] == '|' )  &&
				( a[ii][2*jj] == '_'  ||  a[ii][2*jj+2] == '_' ) )
				printf("\b_");
		}
		printf("\n");
	}
	if( answer )
		printf("path length = %d\n", finish - start);
	if( b_answer )
		printf("back-outs = %d\n", backcount);
}

goodpath()
{

	i = i_old = i_first = i_center = SIZE/2 - SIZE/3 + rand()%(2*SIZE/3);
	j = j_old = j_first = j_center = SIZE/2 - SIZE/3 + rand()%(2*SIZE/3);
	b[i][j] = 16000;
	while( move( &i, &j ) ) {
		mark( i, j, &i_old, &j_old, 1 );
		if( surrounded( i, j )  &&  b[i][j] - 16000 < 4*SIZE )
			backout( -1 );
	}
	finish = b[i][j];
	edge( i < SIZE/2 ? 0 : SIZE+1, j < SIZE/2 ? 0 : SIZE+1 );
	i = i_old = i_first;
	j = j_old = j_first;
	if( surrounded( i, j ) )
		backout( 1 );
	while( move( &i, &j ) ) {
		mark( i, j, &i_old, &j_old, -1 );
		if( surrounded( i, j )  &&  finish - b[i][j] < 8*SIZE )
			backout( 1 );
	}
	start = b[i][j];
}

mark( k, l, k_old, l_old, s)
int	k, l, *k_old, *l_old, s;
{
	b[k][l] = b[*k_old][*l_old] + s;
	if( k > *k_old )
		a[k-1][2*l] = ' ';
	if( k < *k_old )
		a[k][2*l] = ' ';
	if( l > *l_old )
		a[k][2*l-1] = '_';
	if( l < *l_old )
		a[k][2*l+1] = '_';
	*k_old = k;
	*l_old = l;
}

badpath()
{

	v = b[i][j];
	edge( 0, 0 );
	edge( SIZE+1, SIZE+1 );
	while( nextstart() ) {
		v = b[i][j];
		b[i][j] = 1;
		i_old = i_first = i;
		j_old = j_first = j;
		while( move( &i, &j ) ) {
			mark( i, j, &i_old, &j_old, -2 );
			if( rand()%5 == 0 )
				sidepath( i, j, 5 );
		}
		i = i_first;
		j = j_first;
	}
}

nextstart()
{
	int	x;

	for( x = v + 1; x < v + 4 ; x++ ) {
		if( x == finish )
			return( 0 );
		find( x );
	}
	return( 1 );
}


move( k, l )
int	*k, *l;
{
	int	nk, nl;

	if( surrounded( *k, *l ) )
		return( 0 );
	if( *k < 1  ||  *l < 1  ||  *k > SIZE  ||  *l > SIZE )
		return( 0 );
	do {
		nk = *k;
		nl = *l;
		switch( ( rand() >> 11 )%4 ) {
		case 0:
			nk = *k - 1;
			break;
		case 1:
			nk = *k + 1;
			break;
		case 2:
			nl = *l - 1;
			break;
		case 3:
			nl = *l + 1;
			break;
		}
	}  while( b[nk][nl] != 0 );
	*k = nk;
	*l = nl;
	return( 1 );
}

surrounded( k, l )
int	k, l;
{
	if(	b[ k+1 ][ l ] != 0  &&
		b[ k-1 ][ l ] != 0  &&
		b[ k ][ l+1 ] != 0  &&
		b[ k ][ l-1 ] != 0  )
		return( 1 );
	else
		return( 0 );
}

edge( k, l )
int	k, l;
{
	int	x;
	if( k < 1  ||  k > SIZE )
		for( x = 0; x < SIZE+2; x++ )
			if( b[k][x] == 0 )
				b[k][x] = -2;
	if( l < 1  ||  l > SIZE )
		for( x = 0; x < SIZE+2; x++ )
			if( b[x][l] == 0 )
				b[x][l] = -2;
}

sidepath( k, l, chance )
int	k, l, chance;
{
	int	k_old, l_old;

		k_old  = k;
		l_old  = l;
		chance += 15;
		while( move( &k, &l ) ) {
			mark( k, l, &k_old, &l_old, -2 );
			if( rand()%chance == 0 )
				sidepath( k, l, chance );
		}
}

backout( d )
int	d;
{
	int	x;

	do {
		x = b[i][j] + d;
		b[i][j] *= -1;
		find( x );
	} while( surrounded( i, j ) );
	i_old = i;
	j_old = j;
	backcount++;
}

find( x )
int	x;
{
	if( x == b[i+1][j] )
		i += 1;
	else if( x == b[i-1][j] )
		i -= 1;
	else if( x == b[i][j+1] )
		j += 1;
	else if( x == b[i][j-1] )
		j -= 1;
	else
		printf("help\n");
}

verybad()
/*
	attempt to fill in the remaining unoccupied squares with bad paths
*/
{
	int	k, l;

	for( i = 1; i < SIZE ; i++ )
		for( j = 1; j < SIZE ; j++ )
			if( adjacent( i, j, &k, &l ) ) {
				if( b[k][l] > 0 )
					b[k][l] = 1;
				sidepath( k, l, 5 );
			}
}

adjacent( i_in, j_in, i_out, j_out )
int	i_in, j_in, *i_out, *j_out;
/*
	find an occumpied space next to an unoccupied space
*/
{

	if( b[i_in][j_in] != 0 )
		return( 0 );	/* space is occupied */
	*i_out = i_in;
	*j_out = j_in;
	if( b[ i_in ][ j_in-1 ] != 0  &&  j_in-1 > 0 )
		*j_out = j_in - 1;
	else if( b[ i_in ][ j_in+1 ] != 0  &&  j_in+1 < SIZE+1 )
		*j_out = j_in + 1;
	else if( b[ i_in-1 ][ j_in ] != 0  &&  i_in-1 > 0 )
		*i_out = i_in - 1;
	else if( b[ i_in+1 ][ j_in ] != 0  &&  i_in+1 < SIZE+1 )
		*i_out = i_in + 1;
	else
		return( 0 );	/* no adjacent occupied square found */
	return( 1 );
}

clean()
{
	int	k, l;

	for( k = 0; k < SIZE; k++ )
		for( l = 0; l < 2*SIZE; l++ )
			if( a[k][l+1] == '_'  &&
				(a[k][l] == ' ' && a[k][l+2] == ' ') )
				a[k][l+1] = ' ';
}
