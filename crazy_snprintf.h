#ifndef __CRAZY_SNPRINTF_H__
#define __CRAZY_SNPRINTF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

uint32_t crazy_snprintf_impl( char *buf, uint32_t max, ... );

#define CRAZY_SNPRINTF( BUF, MAX, ... )	crazy_snprintf_impl( BUF, MAX, ##__VA_ARGS__, 0, 0 )

#endif
