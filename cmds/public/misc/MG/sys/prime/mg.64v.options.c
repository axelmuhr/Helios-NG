/* options for Primos Mg by Robert A. Larson

/* 64v mode version
-64v
-newfortran
/* define to indicate primos
-define __50SERIES
/* get routine names in stack dump
-store_owner_field
/* tell it to run fast
-standardintrinsics
-optimize
/* find varargs.h in *>sys>prime
-include *>sys>prime

/* real options
-define NO_DIRED                /* no 64v mode support in dired code
-define PREFIXREGION
-define NOTAB
