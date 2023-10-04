#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <luaboot/stdlib.h>
#include <luaboot/string.h>
#include <luaboot/printf.h>
#include <luaboot/e9.h>
#include <luaboot/efi.h>
#include <luaboot/flanterm.h>

#define meta_outchar_const (FILE*)-1

FILE *stdin = NULL, *stdout = meta_outchar_const, *stderr = meta_outchar_const;

static void _write_char(FILE* file, char ch) {
	errno = 0;
	if (file == meta_outchar_const) {
		if (ch == '\n')
			efi_console_printf("\r\n");
		else
			efi_console_printf("%c", ch);
	} else {
		if (!file->_can_write) {
			errno = EINVAL;
			return;
		}
		if (file->_is_stream) {
			file->write(file->arg, &ch, 0, 1);
		} else {
			int64_t wr = file->write(file->arg, &ch, file->off, 1);
			if (wr < 0) return;
			file->off += wr;
		}
	}
}

void _put_char(char character) {
    _write_char(stdout, character);
}

int fflush(FILE* f) {
    (void)f;
    return 0;
}

int fprintf(FILE* out, const char* format, ...) {
    va_list args;
    va_start(args, format);

    char tmp[512];

    int ret = vsnprintf(tmp, 512, format, args);
	for (int i = 0; i < strlen(tmp); i++)
    	_write_char(out, tmp[i]);

    va_end(args);
    return ret;
}

size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream) {
	errno = 0;
    for (size_t i = 0; i < (size * nitems); i++) {
        _write_char(stream, *((char*)(ptr) + i));
    }
    return (size) * (nitems);
}

int getc(FILE *stream) {
	errno = 0;
	if (!stream) {
		errno = EINVAL;
		return -1;
	}
	uint8_t buf = 0;
	if (stream->_is_stream) {
		intptr_t i = stream->read(stream->arg, &buf, 0, 1);
		if (i <= 0) return -1;
		return buf;
	} else {
		intptr_t i = stream->read(stream->arg, &buf, stream->off, 1);
		if (i <= 0) return -1;
		stream->off += i;
		return buf;
	}
}

void* fgets(void* s, uint64_t n, FILE* stream) {
	errno = 0;
    uint8_t* data = (uint8_t*)s;
    uint64_t c = 0;
    while (c < n) {
        int chr = getc(stream);
		if (chr < 0) {
			if (c) {
				*data++ = 0;
				return s;
			}
			return NULL;
		}
		if (chr == '\n') {
            *data++ = '\n';
            c++;
            if (c >= n) data--;
            *data++ = 0;
            return s;
        }
        if (chr == '\x7f' && (stream == meta_outchar_const || stream->_is_text)) {
            // backspace
            if (!c) continue;
            c--;
            *(--data) = 0;
            continue;
        }
        *data++ = chr;
        c++;
    }
    *data++ = 0;
    return s;
}

static int64_t uefi_tty_read(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
		EFI_INPUT_KEY key = efi_console_read_key();
		char *c = malloc(512);
		//wctomb(c, key.UnicodeChar);
		if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
			c = "\n";
			fprintf(stdout, "\n");
		} else if (key.UnicodeChar == CHAR_BACKSPACE) {
			c = "\b";
			fprintf(stdout, "\b \b");
		} else {
			snprintf(c, 512, "%lc", key.UnicodeChar);
			fprintf(stdout, "%s", c);
		}

		for (int j = 0; j < strlen(c); j++)
        	((uint8_t *)buf)[i + j] = c[j];
    }
    return max;
}

static int64_t uefi_tty_write(void *arg, const void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max;i++) {
		uint8_t ch = ((uint8_t *)buf)[i];
		if (ch == '\n')
			efi_console_printf("\r\n");
		else
			efi_console_printf("%c", ch);
    }
    return max;
}

static int64_t flanterm_tty_write(void *arg, const void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max;i++) {
		uint8_t ch = ((uint8_t *)buf)[i];
		flanterm_printf("%c", ch);
    }
    return max;
}

static int64_t serial_read(void *arg, void *buf, uint64_t off, uint64_t max) {
	EFI_SERIAL_IO_PROTOCOL *serial;
	EFI_GUID serial_guid = EFI_SERIAL_IO_PROTOCOL_GUID;
	EFI_STATUS status = BS->LocateProtocol(&serial_guid, NULL, (void**)&serial);
	serial->SetAttributes(serial, 115200, 0, 1000, NoParity, 8, OneStopBit);
	serial->Read(serial, &max, buf);
    return max;
}

static int64_t serial_write(void *arg, const void *buf, uint64_t off, uint64_t max) {
	EFI_SERIAL_IO_PROTOCOL *serial;
	EFI_GUID serial_guid = EFI_SERIAL_IO_PROTOCOL_GUID;
	EFI_STATUS status = BS->LocateProtocol(&serial_guid, NULL, (void**)&serial);
	serial->SetAttributes(serial, 115200, 0, 1000, NoParity, 8, OneStopBit);
	serial->Write(serial, &max, buf);
    return max;
}

_Bool _fopen(FILE *file, const char *path) {
    if (!strcmp(path, "/dev/console")) {
        file->read = uefi_tty_read;
        file->write = flanterm_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/ueficonsole")) {
        file->read = uefi_tty_read;
        file->write = uefi_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/stdin")) {
        file->read = uefi_tty_read;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/stdout")) {
        file->write = flanterm_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/stderr")) {
        file->write = flanterm_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/serial")) {
        file->read = serial_read;
        file->write = serial_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    }
    errno = ENOENT;
    return 0;
}

FILE *fopen(const char *pathname, const char *mode) {
	errno = 0;
	FILE* f = (FILE*)malloc(sizeof(FILE));
	if (!f) {
		errno = ENOMEM;
		return NULL;
	}
	f->_can_read = 0;
	f->_can_write = 0;
	f->_is_buffered = 0;
	f->_is_text = 0;
	f->_is_stream = 0;
	uint8_t plus = 0, append = 0; 
	while (*mode) {
		char m = *mode++;
		switch (m) {
			case 'r':
				f->_can_read = 1;
				break;
			case 'w':
				f->_can_write = 1;
				break;
			case '+':
				plus = 1;
				break;
			case 'a':
				f->_can_write = 1;
				append = 1;
				break;
			default:
				free(f);
				errno = EINVAL;
				return NULL;
		}
	}
	if (plus) { f->_can_read = f->_can_write = 1; }
	if (_fopen(f, pathname)) {
		if (f->_is_stream && append) {
			f->close(f->arg);
			free(f);
			errno = EINVAL;
			return NULL;
		}
		if (append) f->off = f->seek_lookup(SEEK_END, 0, 0);
		else f->off = 0;
	} else {
		free(f);
		if (!errno) errno = EUNKERR;
		return NULL;
	}
	return f;
}

void clearerr(FILE *) { e9_printf("todo: clearerr\n"); abort(); }
int fclose(FILE *) { e9_printf("todo: fclose\n"); abort(); }
int feof(FILE *) { e9_printf("todo: feof\n"); abort(); }
int ferror(FILE *) { e9_printf("todo: ferror\n"); abort(); }
size_t fread(void *, size_t, size_t, FILE *) { e9_printf("todo: fread\n"); abort(); }
FILE *freopen(const char *, const char *, FILE *) { e9_printf("todo: freopen\n"); abort(); }
int fseek(FILE *, long, int) { e9_printf("todo: fseek\n"); abort(); }
int ftell(FILE *) { e9_printf("todo: ftell\n"); abort(); }
int remove(const char *) { e9_printf("todo: remove\n"); abort(); }
int rename(const char *, const char *) { e9_printf("todo: rename\n"); abort(); }
int setvbuf(FILE*, char*, int, size_t) { e9_printf("todo: setvbuf\n"); abort(); }
FILE *tmpfile(void) { e9_printf("todo: tmpfile\n"); abort(); }
char *tmpnam(char *) { e9_printf("todo: tmpnam\n"); abort(); }
void ungetc(int, FILE *) { e9_printf("todo: ungetc\n"); abort(); }