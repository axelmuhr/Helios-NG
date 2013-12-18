/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- prototype.h							--
--                                                              --
--	The function prototypes               			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

BYTE  *find_blanks(BYTE *);
BYTE  *skip_blanks(BYTE *);
DirStream *read_dir(STRING localname);
DosDirEntry *search(int num, int *chain);
WORD direct_read(WORD start, WORD size, char *buf);
WORD direct_write(WORD start, WORD size, char *buf);
WORD flatten(string);
WORD flop_read(WORD start, WORD size, char *buf);
WORD flop_write(WORD start, WORD size, char *buf);
WORD read_from_file(WORD start, WORD size, char *buf, FileInfo *info);
WORD write_to_file(WORD start, WORD size, char *buf, FileInfo *info);
int  open_file(STRING localname, FileInfo *);
int Flop_init(char *buf, int size);
int change_date(STRING localname);
int check_disc(word mode, word *ret_code);
int create_obj(string name, int type);
int delete_object(STRING localname);
int drive_statistics(WORD *sizeptr, WORD *availptr);
int exists_obj(STRING localname);
int format_disc(word mode);
int get_dir_chain(int first, int *chain);
int get_dir_entry(string subdir_name, int *chain, int num_entries);
int get_fat_entry(int);
int get_file_sector(int numsector, int first_cluster);
int get_free_cluster(void);
int get_object_info(string localname, FileInfo *file_info);
int grow(int cluster, int zero, int *fat_change);
int last_clus(int first);
int object_info(string localname, FileInfo *file_info);
int put_fat_entry(int numentry, int code, int *fat_change);
int read_boot(Boot *);
int read_fat(int numfat);
int read_sector(int sector);
int recursive_search(void);
int reduce_dir(int first, int *fat_change);
int reload(void);
int rename_object(STRING oldname, STRING newname);
int reset_chain(int *chain);
int subdir(string path);
int substr(char *fromdir, char*todir);
int undo_fat(int old_end);
int write_dir_entry(int numentry, DosDirEntry *dir, int *chain);
int write_fat(int *fat_change);
int zapit(int first, int *fat_change);
string GetFullName(STRING, MCB *);
string GetLocalName(string);
string get_name_and_path(string localname, char **filename, char **pathname);
string lastbit(string);
void Action(DCB *dcb, DiscReq *req);
void Dir_Open(MCB *, string, string);
void Dir_Read(MCB *, DirStream *);
void Dir_Seek(MCB *, DirStream *);
void Drive_Create(MCB *, string);
void Drive_Delete(MCB *, string);
void Drive_Locate(MCB *, string);
void Drive_ObjInfo(MCB *, string);
void Drive_Open(MCB *, string);
void Drive_Rename(MCB *, string);
void Drive_ServerInfo(MCB *, string);
void Drive_SetDate(MCB *, string);
void Fault(word, char *, word);
void File_GetSize(MCB *, FileInfo *);
void File_Open(MCB *, string, string, WORD openmode);
void File_Read(MCB *, FileInfo *);
void File_Seek(MCB *, FileInfo *);
void File_Write(MCB *, FileInfo *);
void Format_or_Check(MCB *);
void GSPServer(MyDevice *, WORD);
void GSPWorker(MyDevice *, MCB *);
void InvalidFun(MCB *, string);	
void Return(MCB *, WORD FnRc, WORD ContSize, WORD DataSize);
void SendError(MCB *, WORD);
void SendOpenReply(MCB *, string name, WORD type, WORD flags, Port reply);
void close_file(FileInfo *info);
void exit(int);
void fixname( string filename, string name, string ext);
void free(void *ptr);
void handle_interrupt(void);
void mk_entry(string entryname, char attrib, int cluster, WORD size, 
		DosDirEntry *dir);
void move_to_buf(int, char *, int, int, word, int);
void unixname(string name, string ext, string newname);
