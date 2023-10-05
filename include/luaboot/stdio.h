#pragma once
#include <stdint.h>
#include <stddef.h>
#include <luaboot/printf.h>
#include <luaboot/efi.h>

typedef struct FILE {
	uint8_t _is_text, _can_read, _can_write, _is_stream;
	int _errno;
	void *arg;
	uint64_t off;

	uint64_t (*seek_lookup)(void *arg, int mode, uint64_t pos, uint64_t offset);
	uint64_t (*seek)(void *arg, uint64_t pos, uint64_t offset);
	int64_t (*read)(void *arg, void *buf, uint64_t off, uint64_t maxcnt);
	int64_t (*write)(void *arg, void *buf, uint64_t off, uint64_t maxcnt);
	void (*remove)(void *arg);
	void (*close)(void *arg);
} FILE;

#define L_tmpnam 1024
#define BUFSIZ 1024

int remove(const char *path);
int rename(const char *old, const char *new);

int fprintf(FILE *, const char *format, ...);
int ferror(FILE *);
int fclose(FILE *);
int feof(FILE *);
int fseek(FILE *, long offset, int whence);
int ftell(FILE *);
int fflush(FILE *);
void *fgets(void *, uint64_t, FILE *);
FILE *fopen(const char *, const char*);
FILE *freopen(const char *, const char *, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);
char *tmpnam(char *s);
FILE *tmpfile(void);
int getc(FILE*);
void ungetc(int c, FILE *);
void clearerr(FILE *);
int setvbuf(FILE *, char *, int, size_t);
int getchar(void);
extern FILE *stdin, *stdout, *stderr;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IONBF 0
#define _IOFBF 0
#define _IOLBF 0

#define EOF ((int)(-1))