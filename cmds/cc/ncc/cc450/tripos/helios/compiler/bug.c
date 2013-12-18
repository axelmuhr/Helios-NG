typedef void (*vfp)(void);
extern void a(void);
extern void b(void);
extern void c(void);
vfp abc[] = {a, b, b};
