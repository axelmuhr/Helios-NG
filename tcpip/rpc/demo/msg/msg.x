/* @(#)msg.x	1.1 87/11/04 3.9 RPCSRC */
/*
 * msg.x: Remote message printing protocol
 */
program MESSAGEPROG {
	version MESSAGEVERS {
		int PRINTMESSAGE(string) = 1;
	} = 1;
} = 99;
