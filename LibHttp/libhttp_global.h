#pragma once

#if defined(_WIN32) || defined(__WIN32__)
#  if defined(LIBHTTP_EXPORTS) // add by CMake
#    define  LIBHTTPSHARED_EXPORT __declspec(dllexport)
#  else
#    define  LIBHTTPSHARED_EXPORT __declspec(dllimport)
#  endif // MyLibrary_EXPORTS
#elif defined(linux) || defined(__linux)
# define LIBHTTPSHARED_EXPORT
#endif

