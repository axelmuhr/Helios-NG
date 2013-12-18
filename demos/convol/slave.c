#include <stdio.h>
#include <helios.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>
#include <nonansi.h>

typedef struct control_info {
	WORD	height;
	WORD	width;
} control_info;

control_info slave_info;
BYTE **cur_picture, **new_picture;

#define input     0
#define output    1

void initialise_memory(void);
void read_my_part_of_the_picture(void);
void read_extra_scan_lines(void);
void perform_convolution(int fncode);
void send_back_data(void);

int main(void)
{ int  result, fncode;
  Carrier *mem;
  
  mem = AllocFast(1500, &MyTask->MemPool);
  
                     /* get the configuration info from the master */  
  result = read(input, (char *) &slave_info, sizeof(control_info));
  if (result < sizeof(control_info))
   exit(1);

                    /* initialise suitable buffers */
  initialise_memory();

                    /* inform the master that the slave is ready */
  fncode = TRUE;
  if (write(output, (char *) &fncode, sizeof(int)) < sizeof(int))
   exit(1);

  read_my_part_of_the_picture();

  forever
   { if (read(input, (char *) &fncode, sizeof(int)) < sizeof(int))
      exit(1);

     read_extra_scan_lines();
     
     if (mem == Null(Carrier))
       perform_convolution(fncode);
     else
       Accelerate(mem, perform_convolution, 4, fncode);
          
     send_back_data();
   }     
}

void initialise_memory(void)
{ int size, i;

  size = slave_info.height + 8;   /* add four scan lines on either side */
  cur_picture = (BYTE **) malloc( size * sizeof(BYTE *));
  new_picture = (BYTE **) malloc( size * sizeof(BYTE *));
  if ((cur_picture == Null(BYTE *)) || (new_picture == Null(BYTE *)))
   exit(1);

  size *= slave_info.width;
  cur_picture[0] = (BYTE *) malloc( size * sizeof(BYTE));
  new_picture[0] = (BYTE *) malloc( size * sizeof(BYTE));
  if ((cur_picture[0] == Null(BYTE)) || (new_picture[0] == Null(BYTE)))
   exit(1);
   
  for (i = 1; i < (slave_info.height + 8); i++)
   { cur_picture[i] = cur_picture[i-1] + slave_info.width;
     new_picture[i] = new_picture[i-1] + slave_info.width;
   }   
} 

void read_my_part_of_the_picture(void)
{ int result, amount;

  amount = slave_info.height * slave_info.width;
  result = read(input, cur_picture[4], amount);
  if (result < amount)
   exit(1);
}

void read_extra_scan_lines(void)
{ int result, amount;

  amount = 4 * slave_info.width;
  result = read(input, cur_picture[0], amount);
  if (result < amount)
   exit(1);
  result = read(input, cur_picture[slave_info.height + 4], amount);
  if (result < amount)
   exit(1);
}

void send_back_data(void)
{ int result, amount;
  BYTE **temppic = cur_picture;

  cur_picture = new_picture; new_picture = temppic;
  
  amount = slave_info.height * slave_info.width;
  result = write(output, cur_picture[4], amount);
  if (result < amount)
   exit(1);
}

int matrix1[9][9] = { {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, 81, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                    };

int matrix2[9][9] = { {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1,  0,  0,  0,  0,  0, -1, -1 },
                      {  -1, -1,  0,  1,  1,  1,  0, -1, -1 },
                      {  -1, -1,  0,  1, 49,  1,  0, -1, -1 },
                      {  -1, -1,  0,  1,  1,  1,  0, -1, -1 },
                      {  -1, -1,  0,  0,  0,  0,  0, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 }
                    };

int matrix3[9][9] = { {   1,  1,  1,  1,  1,  1,  1,  1,  1 },
                      {   1,  1,  1,  1,  1,  1,  1,  1,  1 },
                      {   1,  1,  1,  1,  1,  1,  1,  1,  1 },
                      {   1,  1,  1,  1,  1,  1,  1,  1,  1 },
                      {   0,  0,  0,  0,  1,  0,  0,  0,  0 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                      {  -1, -1, -1, -1, -1, -1, -1, -1, -1 }
                    };
int matrix4[9][9] = { {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  1,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 },
                      {  -1, -1, -1, -1,  0,  1,  1,  1,  1 }
                    };

int matrix5[9][9] = { {   0,  1,  0,  0,  0,  0,  0,  0,  0 },
                      {  -1,  0,  1,  0,  0,  0,  0,  0,  0 },
                      {   0, -1,  0,  1,  0,  0,  0,  0,  0 },
                      {   0,  0, -1,  0,  1,  0,  0,  0,  0 },
                      {   0,  0,  0, -1,  1,  1,  0,  0,  0 },
                      {   0,  0,  0,  0, -1,  0,  1,  0,  0 },
                      {   0,  0,  0,  0,  0, -1,  0,  1,  0 },
                      {   0,  0,  0,  0,  0,  0, -1,  0,  1 },
                      {   0,  0,  0,  0,  0,  0,  0, -1,  0 }
                    };
                    
void perform_convolution(int fncode)
{ int y, x, i, j;
  int matrix[9][9];
  int sum = 0;

  switch(fncode)
   { case 1 : memcpy( matrix, matrix1, 9 * 9 * sizeof(int)); break;
     case 2 : memcpy( matrix, matrix2, 9 * 9 * sizeof(int)); break;
     case 3 : memcpy( matrix, matrix3, 9 * 9 * sizeof(int)); break;
     case 4 : memcpy( matrix, matrix4, 9 * 9 * sizeof(int)); break;
     case 5 : memcpy( matrix, matrix5, 9 * 9 * sizeof(int)); break;
     default: { BYTE **temppic = cur_picture;
                cur_picture    = new_picture;
                new_picture    = temppic;
                return;
              }
   }
 
  for (j = 0; j < 9; j++)
   for (i = 0; i < 9; i++)
    sum += matrix[j][i];
     
  for (y = 4; y < (slave_info.height + 4); y++)
   for ( x = 0; x < slave_info.width; x++)
    { int result = 0;


      for (j = 0; j < 9; j++)
       for (i = 0; i < 9; i++)
        { int curvalue;

          if ((x < 4) || (x > (slave_info.width - 4)))
           curvalue = (int) (cur_picture[y - 4 + j])[0];
          else
           curvalue = (int) (cur_picture[y - 4 + j])[x - 4 + i];

          result += (curvalue * matrix[j][i]);
        }
      if (result < 0)
       result = 0;

      (new_picture[y])[x] = (BYTE) ( result / sum );
    }
}
