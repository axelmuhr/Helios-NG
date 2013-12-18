/* ncc test prog */
/* tests anomoulous last data directive size */

char chara1[] = "20chars...0123456789"; /* should = DATA 24 chara1 */
char chara2[] = "20chars...0123456789"; /* should = DATA 24 chara2 */
/* error is that it equals data 52 chara2 */
int main()
{
	print("NOTHING");
}
