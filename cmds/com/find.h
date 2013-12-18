/* Declarations for find.c PRH 1/8/90 */

long get_time ( struct stat, char );
void lfree ( LINK );
int set_up ( int , char** );
int search_dir ( int );
LINK add_to_list ( char** , char** , LINK );
int print_error ( int , char* );
int make_path_list ( int , char** );
int examine ( char* ); 
int form_name ( char* , int );
int test_conds ( struct stat , char* , LINK );
int check_conds ( struct stat , char* , char** , int );
int do_commands ( char** );
int match ( char* , char* );
int get_num ( char* );
int num_check ( int , int , char );
int round ( float );
int time_check ( struct stat , int , int , char );
int flag_check ( char* , char* , char* );
int newer ( struct stat , struct stat , char , char );
int type_check ( struct stat , char* );
int form_command ( char* , char* );
  
