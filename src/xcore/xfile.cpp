#include "xfile.h"
#include <string.h>

#define HANDLE_OFFSET  	66 // 'B', 'D' = 68 for filenames
#define NUM_HANDLES 	4
int offsets[NUM_HANDLES] = {0};
char filenames[NUM_HANDLES][16] = {{0}};

// #define debugprintf(...) printf(__VA_ARGS__)
#define debugprintf(...)


FILE* xfopen( const char* filename, const char* mode ){
	FILE *fp_tmp;
	debugprintf("xfopen: %s\n", filename);
	pf_open(filename);
	int handle = (filename[0] - HANDLE_OFFSET);
	fp_tmp = (FILE *)handle;

	// copy stuff
	strcpy(filenames[handle], filename);
	offsets[handle] = 0;

	return fp_tmp;
}

size_t xfread( void *buffer, size_t size, size_t count, FILE *stream ){
// size_t xfread( void *buffer, size_t size, size_t count, int stream ){
	UINT num_read;
	debugprintf("xfread size: %d handle: %p\n", size * count, stream);
	pf_read(buffer, size * count, &num_read);
	
	offsets[(int)stream] += size * count;

	return num_read;
}

int xfseek( FILE* stream, long offset, int origin ){
// int xfseek( int stream, long offset, int origin ){
	debugprintf("xfseek handle: %p\n", stream);
	int result = pf_lseek(offset);

	offsets[(int)stream] = offset;

	return result;
}

long xftell( FILE* stream ){
// long xftell( int stream ){
	int val = 0;
	debugprintf("xfread xftell steam: %p, : %d\n", stream, val);

	return val;
}

int xfclose( FILE* stream ){
	debugprintf("xfclose handle: %p\n", stream);

	return 0;
}

/////////////////////////
// Used by w_wad.cpp
/////////////////////////

int xopen(const char *pathname, int flags, ... /* mode_t mode */ ){
	int fp_tmp;
	debugprintf("xopen: %s\n", pathname); 
	pf_open(pathname);
	fp_tmp = pathname[0] - HANDLE_OFFSET;

	// copy stuff
	strcpy(filenames[fp_tmp], pathname);
	offsets[fp_tmp] = 0;

	return fp_tmp; 
}

size_t xread( int stream, void *buffer, size_t size){
	UINT num_read;
	debugprintf("xread size: %d, handle: %d\n", size, stream);

	// HACK
	pf_open(filenames[stream]);
	pf_lseek(offsets[stream]);

	pf_read(buffer, size, &num_read);

	offsets[stream] += size;

	return num_read;
}


int xlseek( int stream, long offset, int origin ){
	debugprintf("xfseek handle: %d, offset: %ld\n", stream, offset);
	int result = pf_lseek(offset);

	offsets[stream] = offset;

	return result;
}

long xtell( int stream ){
	debugprintf("xftell steam: %d\n", stream);

	return offsets[stream];
}

int xclose( int stream ){
	debugprintf("xfclose handle: %d\n", stream);

	return 0;
}

size_t xwrite(int fd, const void *buf, size_t count){

	return 1;
}