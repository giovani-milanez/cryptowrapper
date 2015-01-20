#ifndef CRYPTOWRAPPER_DEFS_
#define CRYPTOWRAPPER_DEFS_

#ifdef _WIN32
#  ifdef cryptowrapper_EXPORTS
#    define CRYPTOWRAPPER_API __declspec(dllexport)
#    define EXPIMP_TEMPLATE
#  else
#    define CRYPTOWRAPPER_API __declspec(dllimport)
#    define EXPIMP_TEMPLATE extern
#  endif
#else
#  define CRYPTOWRAPPER_API
#endif

#endif
