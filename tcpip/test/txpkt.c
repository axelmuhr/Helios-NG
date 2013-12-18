#include <helios.h>
#include <syslib.h>
#include <stdio.h>

typedef struct NetInfo
{
  word Mask;
  word Mode;
  word State;
  char Addr[8];	
} NetInfo;

typedef struct
{
	char dest[6];
	char src[6];
	short type;
	char data[1500];
} EPkt;
int main()
{
  Object* o = Locate(NULL,"/ether");	
  Stream* ns = Open(o,NULL,O_WriteOnly);	
  NetInfo ni;
  int i;
  byte p[100];
  ni.Mask = 7;
  GetAttributes(ns,&ni);
  printf("%2x%2x%2x%2x%2x%2x\n",
         ni.Addr[0],  
         ni.Addr[1],  
         ni.Addr[2],  
         ni.Addr[3],  
         ni.Addr[4],  
         ni.Addr[5]);
         
	i = 0;
	for( i < 6; i++ ) p[i] = 0xff;
	for( i < 12; i++ ) p[i] = ni.Addr[i-6];
	p[i++] = 0x08;
	p[i++] = 0x06;
  	p[i++] = 0;
  	p[i++] = 1;
  	p[i++] = 0;
  	p[i++] = 8;
  	p[i++] = 6;
  	p[i++] = 4;
  	p[i++] = 0;
  	p[i++] = 1;
  	
	Write(ns,&p,64,-1);
  
}

