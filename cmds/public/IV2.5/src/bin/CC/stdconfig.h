/*
 * Defaults for compiler configuration.
 */

/* C preprocessor */
#if !defined(CC_cpp)
#define CC_cpp "/lib/cpp"
#endif

/* translator/compiler */
#if !defined(CC_compiler)
#   if defined(cfront)
#       define CC_compiler "/usr/local/bin/cfront"
#   else
#       define CC_compiler "/usr/local/bin/c++"
#   endif
#endif

/* compiler output suffix */
#if !defined(CC_suffix)
#   if defined(cfront)
#       define CC_suffix "..c"
#   else
#       define CC_suffix ".s"
#   endif
#endif

#if !defined(CC_library)
#   if defined(cfront)
#       define CC_library "-lC"
#   else
#       define CC_library "-lgnulib"
#   endif
#endif

#if !defined(CC_rt)
#   if defined(cfront)
#       define CC_rt ""
#   else
#       define CC_rt "/usr/local/lib/crt0+.o"
#   endif
#endif

#if !defined(CC_cc)
#   define CC_cc "/bin/cc"
#endif

#if !defined(CC_as)
#   define CC_as "/bin/as"
#endif

#if !defined(CC_ld)
#   define CC_ld "/bin/cc"
#endif

#if defined(cfront) || defined(CC_munch)
#   define CC_domunch
#endif

#if !defined(CC_munch)
#   define CC_munch "/usr/local/bin/munch"
#endif
