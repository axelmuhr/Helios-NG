
int main()
{
	int t = time(0);
	printf("%s\n",ctime(&t));
	printf("%s %s\n",__DATE__,__TIME__);
}
