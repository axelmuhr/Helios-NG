/* test prog for ARM NCC */

#if 1
char fred4[256] = "Bla Bla...."; /* does work */
char fred1[256] = "Bla Bla...."; /* does work */
static char fredstatic[] = "Bla Bla...."; /* doesn't work */
char fred2[] = "Bla Bla...."; /* does work */
#else
char fred[] = "Bla Bla...."; /* does work */
#endif

int main()
{
	printf(fredstatic);
}
