int
small_leaf( int arg )
{
	return arg * 2;
}

int
medium_leaf( int arg )
{
	int array[ 20 ];

	array[ arg ] = 4;

	return array[ arg ];
}

int
big_leaf( int arg )
{
	int array[ 60 ];

	array[ arg * 3 ] = 4;

	return array[ arg ];
}

int
mammoth_leaf( int arg )
{
	int array[ 65536  ];

	array[ arg ] = 4;

	return array[ arg ];
}

int
small_func( int arg )
{
	return small_leaf( arg );
}

int
medium_func( int arg )
{
	int array[ 20 ];

	array[ arg ] = medium_leaf( arg );

	return array[ arg ];
}

int
big_func( int arg )
{
	int array[ 60 ];

	array[ arg ] = big_leaf( arg );

	return array[ arg ];
}

int
mammoth_func( int arg )
{
	int array[ 65536 ];

	array[ arg ] = mammoth_leaf( arg );

	return array[ arg ];
}
