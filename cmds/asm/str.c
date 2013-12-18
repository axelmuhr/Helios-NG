
struct X
{
	short	a;
	short	b;
};

struct X sx;

struct X foo() 
{
	struct X x;
	return x;
}

main()
{
	struct X x;

	sx = foo();
}
