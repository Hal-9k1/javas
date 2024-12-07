#ifndef UNIXISH_HPP
#define UNIXISH_HPP

#if (defined(__linux__) || defined(__CYGWIN__)) && !defined(FORCE_WINDOWS)
#  define UNIXISH
#endif

#endif
