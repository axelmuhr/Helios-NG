int AbortLinkTx(struct LinkInfo *link);

typedef struct LinkInfo {
	int	Flags;		/* flag byte			*/
	int	Mode;		/* link mode/type		*/
	int	State;		/* link state			*/
	int	spare2[2];	/* spare 			*/
} LinkInfo;

int main(int argc, char **argv)
{
	int s;
	LinkInfo *l = (LinkInfo *)argc;
	struct LinkInfo *p = (struct LinkInfo *)argc;

	{
		s = AbortLinkTx(l);
		s = AbortLinkTx((struct LinkInfo *)l);
		s = AbortLinkTx(p);
	}
return 0;
}
