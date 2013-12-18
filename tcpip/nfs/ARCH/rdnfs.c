#include <syslib.h>
#include <task.h>

main(int argc, char **argv)
{
	Stream *s;
	word size;
	char *buf;
	
	if( argc != 2 ) exit(1);
	
	MyTask->Flags |= Task_Flags_stream|Task_Flags_error;
	
	s = Open(cdobj(),argv[1],O_ReadOnly);
	
	size = GetFileSize(s);
	
	buf = Malloc(size);
	
	Read(s,buf,size,-1);
}
