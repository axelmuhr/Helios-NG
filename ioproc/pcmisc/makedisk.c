/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   R A W  D I S K  P R O G R A M                  --
--         -------------------------------------------                  --
--                                                                      --
--              Copyright (C) 1989, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--  makedisk.c                                                           --
--                                                                      --
--           A raw disk device                                          --
--                                                                      --
--  Author:  BLV 12/2/89                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %I% %G%  Copyright (C) 1989, Perihelion Software Ltd.       */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>

#define eq ==
#define ne !=
#define uint	unsigned int
#define uchar	unsigned char
#define WORD    long
#define word    long

extern int disk_read(int drive, int no_sects, WORD first_sect, char *buffer);
extern int disk_write(int drive, int no_sects, WORD first_sect, char *buffer);
extern void setversion(void);
extern int version_number;

#define	bytes_per_sec_off	 0
#define sec_per_alloc_off	 2 
#define reserve_sec_off		 3 
#define no_fats_off			 5 
#define sizeof_root_off		 6
#define total_sec_off		 8
#define media_off			10
#define sec_per_fat_off		11
#define sec_per_track_off	13
#define no_heads_off		15
#define no_hidden_off		17

void display_root(char *);
void hack_root(char *);
void hack_FAT(void);
void hack_directory(void);

char main_buf[2048];
int media_byte, no_fats, sec_per_fat, cur_sec, drive;

int main(int argc, char **argv)
{ char *arg;
  int result, i;

  setversion();
  if (argc ne 2)
   { fprintf(stderr, "Usage : makedisk <drive letter>\n");
     exit(0);
   }

  arg = *(++argv);
  if (isupper(*arg)) *arg = tolower(*arg);
  if ((*arg < 'a') || (*arg > 'z'))
   { fprintf(stderr, "Usage : makedisk <drive a, b, c, d, ..., z>\n");
     exit(0);
   }

  drive = *arg - 'a';

  result = disk_read(drive, 1, 0L, &(main_buf[0]));
  if (result ne 0)
   { fprintf(stderr, "Error reading disk : %x\n", result);
     exit(0);
   }

  display_root(&(main_buf[0]));

  if (drive eq 2)
   { fprintf(stderr, "You do not really want to foul up drive C do you...");
     exit(0);
   }

  printf(
  "\nAbout to convert disk to raw format, this will erase all data on the disk.\n");

  printf("\nPress return to continue, or ctrl-C to abort.\n");
  (void) getchar();

  hack_root(&(main_buf[0]));
  cur_sec = 1;
  for (i = 0; i < no_fats; i++)
   hack_FAT();

  hack_directory();

  return(0);
}


void display_root(char *data)
{ uchar *alloc = &(data[11]);
  long  number_sectors;
  long  number_cylinders;

  number_sectors = (long) ((alloc[9] << 8) + alloc[8]);

  if (alloc[7] > 0x7F) alloc[7] = 0;

  media_byte  = alloc[10];
  no_fats     = alloc[5];
  sec_per_fat = (alloc[12] << 8) + alloc[11];
 
  printf("Disk statistics : \n");
  printf("The following information must be put into the devinfo file\n");
  printf("    sectorsize       %d\n", (alloc[1] << 8) + alloc[0]);
  printf("    sectors          %d\n", (alloc[14] << 8) + alloc[13]);
  printf("    tracks           %d\n", (alloc[16] << 8) + alloc[15]);

  if ((number_sectors == 0L) && (version_number >= 4))
   { 
     number_sectors = (WORD) alloc[24];
     number_sectors = (256L * number_sectors) + (WORD) alloc[23];
     number_sectors = (256L * number_sectors) + (WORD) alloc[22];
     number_sectors = (256L * number_sectors) + (WORD) alloc[21];
   }

  number_sectors -= 2L;
  number_sectors -= (WORD) (no_fats * sec_per_fat);
  number_cylinders = number_sectors / ((long) ((alloc[16] << 8) + alloc[15]));
  number_cylinders /= ((long) ((alloc[14] << 8) + alloc[13]));
  printf("    cylinders        %ld\n", number_cylinders);
}

void hack_root(char *data)
{
  memcpy(&(data[3]), " HELIOS ", 8);
  (void) disk_write(drive, 1, 0L, &(main_buf[0]));
}  

void hack_FAT()
{ int i;

  main_buf[0] = (char) media_byte;     /* copy of media descriptor byte */
  for (i = 1; i < 512; i++)
   main_buf[i] = (char) 0x00FF;

  (void) disk_write(drive, 1, (WORD) cur_sec, &(main_buf[0])); cur_sec++;
  main_buf[0] = (char) 0x00FF;
  for (i = 1; i < sec_per_fat; i++)
   { (void) disk_write(drive, 1, (WORD) cur_sec, &(main_buf[0]));
     cur_sec++;
   }
}

void hack_directory()
{ int i;
  memset(main_buf, 0, 512);
  (void) disk_write(drive, 1, (WORD) cur_sec, &(main_buf[0]));
}

