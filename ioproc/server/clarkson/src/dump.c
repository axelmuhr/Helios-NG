/* comments are an admission that your code isn't clear enough */

/* History:86,1 0 */
/* Fri Dec 15 11:08:53 1989 send_pkt was also dumping two bytes too many. 0 */
/* Thu Dec 14 14:31:36 1989 receive_upcall was dumping another two bytes too many */
/* Wed Aug 09 12:22:30 1989 receive_upcall was dumping two bytes too many */

#include <stdio.h>
#include <ctype.h>

/* the format of the following struct is dependent upon trace.asm */

struct {
	unsigned length;
	unsigned char function;
	unsigned long time;
	unsigned char error;
	union {
		int handle;
		unsigned char bytes[2000];
		struct {
			unsigned version;
			unsigned char class;
			unsigned type;
			unsigned char number;
			unsigned char basic;
		} di;
		struct {
			unsigned char if_class;
			unsigned if_type;
			unsigned char if_number;
			unsigned typelen;
			unsigned handle;
		} at;
		struct {
			unsigned handle;
			unsigned length;
		} ga;
		struct {
			unsigned handle;
			unsigned mode;
		} srm;
	} data;
} record;

char *functions[] = {
	"function_0",
	"driver_info",
	"access_type",
	"release_type",
	"send_pkt",
	"terminate",
	"get_address",
	"reset_interface",
	"function_8",
	"function_9",
	"get_parameters",
	"as_send_pkt",
	"function_12",
	"function_13",
	"function_14",
	"function_15",
	"function_16",
	"function_17",
	"function_18",
	"function_19",
	"set_rcv_mode",
	"get_rcv_mode",
	"set_multicast_list",
	"get_multicast_list",
	"get_statistics",
	"set_address",
};

enum {
	bad_function,
	driver_info,
	access_type,
	release_type,
	send_pkt,
	terminate,
	get_address,
	reset_interface,
	get_parameters = 8,
	as_send_pkt,
	set_rcv_mode = 20,
	get_rcv_mode,
	set_multicast_list,
	get_multicast_list,
	get_statistics,
	set_address,
	receive_upcall = 255
};

char *errors[] = {
	"no_error",
	"bad_handle",
	"no_class",
	"no_type",
	"no_number",
	"bad_type",
	"no_multicast",
	"cant_terminate",
	"bad_mode",
	"no_space",
	"type_inuse",
	"bad_command",
	"cant_send",
	"cant_set",
	"bad_address",
};

main()
{
	FILE *inf;
	static first_time = 1;
	unsigned long time_zero;
	char args[80], results[80];

	inf = fopen("trace.out", "rb");
	if (!inf) {
		fprintf(stderr, "dump: cannot open \"trace.out\"\n");
		exit(1);
	}

	setvbuf(stdout, NULL, _IOFBF, 4096);

	for(;;) {
		if (fread(&record.length, 2, 1, inf) != 1)
			break;
		fread(&record.function, 1, record.length - 2, inf);
		if (first_time) {
			first_time = 0;
			time_zero = record.time;
		}
		args[0] = '\0';
		switch(record.function) {
		case get_statistics:
		case terminate:
		case reset_interface:
		case release_type:
		case get_address:
		case receive_upcall:
			sprintf(args, "%d", record.data.handle);
			break;
		case set_rcv_mode:
			sprintf(args, "%d %d", record.data.srm.handle, record.data.srm.mode);
			break;
		case access_type:
			sprintf(args, "%d %d %d %d", record.data.at.if_class,
			    record.data.at.if_type,
			    record.data.at.if_number,
			    record.data.at.typelen);
			break;
		default:
			strcpy(args, "");
		}
		if (record.error != 0) {
			sprintf(results, "%s (%d)",
				record.error >= (sizeof errors)/(sizeof errors[0])?"?":errors[record.error],
				record.error);
		} else switch(record.function) {
		case driver_info:
			sprintf(results, "%d %d %d %d %d",
			    record.data.di.version, record.data.di.class,
			    record.data.di.type, record.data.di.number,
			    record.data.di.basic);
			break;
		case access_type:
			sprintf(results, "%d", record.data.at.handle);
			break;
		case send_pkt:
			sprintf(results, "%d bytes", record.length - 8);
			break;
		case receive_upcall:
			sprintf(results, "%d bytes", record.length - 10);
			break;
		default:
			strcpy(results, errors[0]);
		}
		printf("%7.2f %s(%s) = %s\n",
		    (float)(record.time - time_zero) / 18.2,
		    record.function == receive_upcall?"receive_upcall":
			functions[record.function], args, results);
		switch(record.function) {
		case get_address:
			dump_bytes(&record.data.bytes[4], record.data.ga.length);
			break;
		case receive_upcall:
			dump_bytes(&record.data.bytes[2], record.length - 10);
			break;
		case send_pkt:
			dump_bytes(&record.data.bytes[0], record.length - 8);
			break;
		}
	}
}

dump_bytes(unsigned char *bytes, int count)
{
	int n;
	char buf[16];
	int address;
	void fmtline();

	address = 0;
	while(count){
		if (count > 16) n = 16;
		else n = count;
		fmtline(address,bytes,n);
		address += n;
		count -= n;
		bytes += n;
	}
}
/* Print a buffer up to 16 bytes long in formatted hex with ascii
 * translation, e.g.,
 * 0000: 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?
 */
void
fmtline(addr,buf,len)
int addr;
char *buf;
int len;
{
	char line[80];
	register char *aptr,*cptr;
	unsigned register char c;
	void ctohex();

	memset(line,' ',sizeof(line));
	ctohex(line,addr >> 8);
	ctohex(line+2,addr & 0xff);
	aptr = &line[6];
	cptr = &line[55];
	while(len-- != 0){
		c = *buf++;
		ctohex(aptr,c);
		aptr += 3;
		c &= 0x7f;
		*cptr++ = isprint(c) ? c : '.';
	}
	*cptr++ = '\n';
	fwrite(line,1,(unsigned)(cptr-line),stdout);
}
/* Convert byte to two ascii-hex characters */
static
void
ctohex(buf,c)
register char *buf;
register int c;
{
	static char hex[] = "0123456789abcdef";

	*buf++ = hex[c >> 4];
	*buf = hex[c & 0xf];
}
