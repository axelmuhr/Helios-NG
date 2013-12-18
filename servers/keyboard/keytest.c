#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <syslib.h>
#include <message.h>
#include <ioevents.h>
#include <string.h>

int main(int argc, char **argv)
{
	Stream *s;
	Object *o, *mc;
	char mcname[100];
	
	if (argc==1)
		return 1;

	MachineName(mcname);
	if (( mc = Locate(NULL, mcname)) == NULL) {
		printf("Cannot locate processor %s!\n",mcname);
		return 1;
	}

	if (( o = Locate(mc, argv[1])) == NULL) {
		printf("Cannot locate %s\n", argv[1]);
		return 1;
	}

	if ((s = Open(mc, argv[1] , O_ReadOnly)) == NULL) {
		printf("Null %s stream\n", argv[1]);
		return 1;
	}
		
	if ( argc == 3 && strcmp(argv[2], "events") == 0) {
		Port p = EnableEvents(s, Event_Keyboard);
		
		if (p == NullPort) {
			printf("Null Event port!\n");
			return 1;
		}

		forever {
			MCB m;
			IOEvent Key;
			char c, scan, shifts, toggles;
			int e;

			InitMCB(&m, MsgHdr_Flags_preserve, p, NullPort, 0);
			m.Data = (byte *)&Key;
			m.MsgHdr.DataSize = Keyboard_EventSize;
			if ((e = GetMsg(&m)) != 0) {
				printf("Error from getmsg (%x)\n",e);
				return 1;
			}

			c = (char)Key.Device.Keyboard.Key;
			scan = Key.Device.Keyboard.What & 0xff;
			shifts = (Key.Device.Keyboard.What >> 16) & 0xff;
			toggles = (Key.Device.Keyboard.What >> 24) & 0xff;

			if (c < 32)
				c = '!';

			printf("ascii (%#x) %c, scan %x, shifts %x, toggles %x\n", c, c, scan, shifts, toggles);

			if (Key.Device.Keyboard.Key == 4) {
				printf("^D\n");
				return 0;
			}
		}
	}
	else {
		forever {
			char c;

			Read(s, &c, 1, OneSec);

			printf("%#x:%c\n",c,c);

			if (c == 4) {
				printf("^D\n");
				return 0;
			}
		}
	}
}
	
