#define MAXNAMELEN 255

typedef char *nametype;
bool_t xdr_nametype();


typedef struct namenode *namelist;
bool_t xdr_namelist();


struct namenode {
	nametype name;
	namelist next;
};
typedef struct namenode namenode;
bool_t xdr_namenode();


struct readdir_res {
	int errno;
	union {
		namelist list;
	} readdir_res_u;
};
typedef struct readdir_res readdir_res;
bool_t xdr_readdir_res();


#define DIRPROG ((u_long)76)
#define DIRVERS ((u_long)1)
#define READDIR ((u_long)1)
extern readdir_res *readdir_1();

