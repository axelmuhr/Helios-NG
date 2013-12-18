extern void* _sbrk(int);

void* sbrk(int n) { return _sbrk(n); }
