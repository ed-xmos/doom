#ifndef __XFILE__
#define __XFILE__

#include <stdio.h>
#include "pff.h"

//Horrible hacks to keep track of which file is open due to petit FFS

FILE* xfopen( const char* filename, const char* mode );
size_t xfread( void *buffer, size_t size, size_t count, FILE *stream );
int xfseek( FILE* stream, long offset, int origin );
long xftell( FILE* stream );
int xfclose( FILE* stream );

/////////////////////////////

int xopen(const char *pathname, int flags, ... /* mode_t mode */ );
size_t xread( int stream, void *buffer, size_t size);
size_t xwrite(int fd, const void *buf, size_t count);
int xlseek( int stream, long offset, int origin );
long xtell( int stream );
int xclose( int stream );


#endif