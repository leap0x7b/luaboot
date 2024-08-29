#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <luaboot/e9.h>
#include <luaboot/efi.h>
#include <luaboot/flanterm.h>

#define _stdin (FILE*)-1
#define _stdout (FILE*)-2

FILE *stdin = _stdin, *stdout = _stdout, *stderr = _stdout;

static void _write_char(FILE* file, char ch) {
    errno = file->_errno = 0;
    if (file == _stdout) {
        if (ch == '\n')
            efi_console_printf("\r\n");
        else
            efi_console_printf("%c", ch);
    } else {
        if (!file->_can_write) {
            errno = file->_errno = EINVAL;
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

size_t fread(void *ptr, size_t size, size_t count, FILE *restrict stream) {
    errno = stream->_errno = 0;
    if (stream == _stdin) {
        EFI_INPUT_KEY key = efi_console_read_key();
        //wctomb(c, key.UnicodeChar);
        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            ptr = "\n";
            fprintf(stdout, "\n");
        } else if (key.UnicodeChar == CHAR_BACKSPACE) {
            ptr = "\b";
            fprintf(stdout, "\b \b");
        } else {
            snprintf(ptr, 512, "%lc", key.UnicodeChar);
            fprintf(stdout, "%s", ptr);
        }
    } else {
        if (!stream->_can_read) {
            errno = stream->_errno = EINVAL;
            return 0;
        }
        if (stream->_is_stream) {
            return stream->read(stream->arg, ptr, 0, 1);
        } else {
            return stream->read(stream->arg, ptr, stream->off, size * count);
        }
    }
    return size * count;
}

size_t fwrite(const void *restrict ptr, size_t size, size_t count, FILE *stream) {
    errno = stream->_errno = 0;
    for (size_t i = 0; i < (size * count); i++) {
        _write_char(stream, *((char *)(ptr) + i));
    }
    return (size) * (count);
}

int fclose(FILE *stream) {
    stream->close(stream->arg);
    return 0;
}

int fseek(FILE *stream, long offset, int mode) {
    if (mode == SEEK_SET) {
        int off = stream->seek(stream->arg, offset, 0);
        stream->off = off;
        return off;
    } else {
        int off = stream->seek_lookup(stream->arg, mode, 0, offset);
        stream->off = stream->seek(stream->arg, off, 0);
        return off;
    }
}

int ftell(FILE *stream) {
    return stream->seek_lookup(stream->arg, SEEK_CUR, 0, 0);
}

int feof(FILE *stream) {
    return stream->off == stream->seek_lookup(stream->arg, SEEK_END, 0, 0);
}

int getc(FILE *stream) {
    errno = stream->_errno = 0;
    if (!stream) {
        errno = stream->_errno = EINVAL;
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
        if (chr == '\b' && (stream == _stdout || stream->_is_text)) {
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

static int64_t uefi_tty_write(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
        uint8_t ch = ((uint8_t *)buf)[i];
        if (ch == '\n')
            efi_console_printf("\r\n");
        else
            efi_console_printf("%c", ch);
    }
    return max;
}

static int64_t flanterm_tty_write(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
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

static int64_t serial_write(void *arg, void *buf, uint64_t off, uint64_t max) {
    EFI_SERIAL_IO_PROTOCOL *serial;
    EFI_GUID serial_guid = EFI_SERIAL_IO_PROTOCOL_GUID;
    EFI_STATUS status = BS->LocateProtocol(&serial_guid, NULL, (void**)&serial);
    serial->SetAttributes(serial, 115200, 0, 1000, NoParity, 8, OneStopBit);
    serial->Write(serial, &max, buf);
    return max;
}

static int64_t uefi_gop_read(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
        ((uint8_t *)buf)[i] = ((uint8_t *)GOP->Mode->FrameBufferBase)[i + off];
    }
    return max;
}

static int64_t uefi_gop_write(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
        ((uint8_t *)GOP->Mode->FrameBufferBase)[i + off] = ((uint8_t *)buf)[i];
    }
    return max;
}

static uint64_t uefi_gop_seek_lookup(void *arg, int mode, uint64_t pos, uint64_t offset) {
    if (mode == SEEK_SET || mode == SEEK_CUR) {
        return pos + offset;
    } else if (mode == SEEK_END) {
        return GOP->Mode->FrameBufferSize;
    }
    return 0;
}

static uint64_t uefi_gop_seek(void *arg, uint64_t pos, uint64_t offset) {
    return pos;
}

static int64_t random_read(void *arg, void *buf, uint64_t off, uint64_t max) {
    for (uint64_t i = 0; i < max; i++) {
        srand(time(NULL));
        ((uint64_t *)buf)[i] = rand();
    }
    return max;
}

static int64_t uefi_file_read(void *arg, void *buf, uint64_t off, uint64_t max) {
    ((EFI_FILE_HANDLE)arg)->Read((EFI_FILE_HANDLE)arg, &max, buf);
    return max;
}

static int64_t uefi_file_write(void *arg, void *buf, uint64_t off, uint64_t max) {
    ((EFI_FILE_HANDLE)arg)->Write((EFI_FILE_HANDLE)arg, &max, buf);
    return max;
}

static void uefi_file_close(void *arg) {
    ((EFI_FILE_HANDLE)arg)->Close((EFI_FILE_HANDLE)arg);
}

static void uefi_file_remove(void *arg) {
    ((EFI_FILE_HANDLE)arg)->Delete((EFI_FILE_HANDLE)arg);
}

static uint64_t uefi_file_seek_lookup(void *arg, int mode, uint64_t, uint64_t offset) {
    if (mode == SEEK_CUR) {
        uint64_t pos = 0;
        ((EFI_FILE_HANDLE)arg)->GetPosition((EFI_FILE_HANDLE)arg, &pos);
        return pos + offset;
    } else if (mode == SEEK_END) {
        EFI_FILE_INFO file_info;
        EFI_GUID file_info_guid = EFI_FILE_INFO_ID;
        uint64_t buf_size = sizeof(EFI_FILE_INFO);
        ((EFI_FILE_HANDLE)arg)->GetInfo((EFI_FILE_HANDLE)arg, &file_info_guid, &buf_size, (void *)&file_info);
        return file_info.FileSize + offset;
    }
    return 0;
}

static uint64_t uefi_file_seek(void *arg, uint64_t pos, uint64_t offset) {
    ((EFI_FILE_HANDLE)arg)->SetPosition(((EFI_FILE_HANDLE)arg), pos + offset);
    return pos + offset;
}

EFI_FILE_HANDLE uefi_get_volume() {
    EFI_GUID file_io_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *file_io;
    EFI_FILE_HANDLE volume;

    BS->HandleProtocol(LIP->DeviceHandle, &file_io_guid, (void **)&file_io);
    file_io->OpenVolume(file_io, &volume);

    return volume;
}

bool _fopen(FILE *file, const char *path) {
    if (!strcmp(path, "/dev/console")) {
        file->read = uefi_tty_read;
        //file->write = flanterm_tty_write;
		file->write = uefi_tty_write;
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
        //file->write = flanterm_tty_write;
		file->write = uefi_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/stderr")) {
        //file->write = flanterm_tty_write;
		file->write = uefi_tty_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/serial")) {
        file->read = serial_read;
        file->write = serial_write;
        file->_is_text = 1;
        file->_is_stream = 1;
        return 1;
    } else if (!strcmp(path, "/dev/fb")) {
        if (GOP == NULL) {
            errno = EIO;
            return 0;
        }

        file->seek_lookup = uefi_gop_seek_lookup;
        file->seek = uefi_gop_seek;
        file->read = uefi_gop_read;
        file->write = uefi_gop_write;
        return 1;
    } else if (!strcmp(path, "/dev/random")) {
        file->read = random_read;
        file->_is_stream = 1;
        return 1;
    } else {
        EFI_FILE_HANDLE volume = uefi_get_volume();
        EFI_FILE_HANDLE handle;
        wchar_t *dest = malloc(strlen(path) * 2);
        const char *tmp = path;
        char *i = strchr(tmp, '/');
        while (i) {
            *i = '\\';
            i = strchr(tmp, '/');
        }
        mbstowcs(dest, tmp, strlen(tmp) * 2);

        uint64_t flags = 0;
        if (file->_can_read) flags |= EFI_FILE_MODE_READ;
        if (file->_can_write) flags |= EFI_FILE_MODE_WRITE;
        EFI_STATUS status = volume->Open(volume, &handle, dest, flags, file->_can_read && !file->_can_write ? EFI_FILE_MODE_READ : 0);
        if (EFI_ERROR(status)) {
            switch (status) {
                case EFI_NOT_FOUND:
                    errno = file->_errno = ENOENT;
                    return 0;
                case EFI_ACCESS_DENIED:
                    errno = file->_errno = EACCES;
                    return 0;
                case EFI_WRITE_PROTECTED:
                    errno = file->_errno = EROFS;
                    return 0;
                case EFI_DEVICE_ERROR:
                    errno = file->_errno = EIO;
                    return 0;
                case EFI_NO_MEDIA:
                    errno = file->_errno = ENXIO;
                    return 0;
                case EFI_OUT_OF_RESOURCES:
                    errno = file->_errno = ENOMEM;
                    return 0;
                case EFI_VOLUME_FULL:
                    errno = file->_errno = ENOSPC;
                    return 0;
                case EFI_INVALID_PARAMETER:
                    errno = file->_errno = EINVAL;
                    return 0;
                default:
                    errno = file->_errno = EUNKERR;
                    return 0;
            }
        }

        file->seek_lookup = uefi_file_seek_lookup;
        file->seek = uefi_file_seek;
        file->read = uefi_file_read;
        file->write = uefi_file_write;
        file->close = uefi_file_close;
        file->remove = uefi_file_remove;
        file->arg = (void *)handle;
        return 1;
    }
    errno = file->_errno = ENOENT;
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
    f->_is_text = 0;
    f->_is_stream = 0;
    f->_errno = 0;
    bool append = 0; 

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
                f->_can_read = f->_can_write = 1;
                break;
            case 'a':
                f->_can_write = 1;
                append = 1;
                break;
            case 'b':
                break;
            default:
                free(f);
                errno = EINVAL;
                return NULL;
        }
    }

    if (_fopen(f, pathname)) {
        if (f->_is_stream && append) {
            f->close(f->arg);
            free(f);
            errno = EINVAL;
            return NULL;
        }
        if (append) f->off = f->seek_lookup(f->arg, SEEK_END, 0, 0);
        else f->off = 0;
    } else {
        free(f);
        if (!errno) errno = EUNKERR;
        return NULL;
    }

    return f;
}

int ferror(FILE *stream) {
    return stream->_errno;
}

void clearerr(FILE *stream) {
    stream->off = 0;
    stream->_errno = 0;
}

int remove(const char *path) {
    errno = 0;

    FILE *file = fopen(path, "+");
    file->remove(file->arg);

    return 0;
}

FILE *freopen(const char *, const char *, FILE *file) { errno = file->_errno = EUNIMPL; e9_printf("todo: freopen\n"); return file; }
int rename(const char *, const char *) { errno = EUNIMPL; e9_printf("todo: rename\n"); return 0; }
int setvbuf(FILE *file, char *, int, size_t) { errno = file->_errno = EUNIMPL; e9_printf("todo: setvbuf\n"); return 0; }
FILE *tmpfile(void) { errno = EUNIMPL; e9_printf("todo: tmpfile\n"); return NULL; }
char *tmpnam(char *) { e9_printf("todo: tmpnam\n"); return "temp"; }
void ungetc(int, FILE *file) { errno = file->_errno = EUNIMPL; e9_printf("todo: ungetc\n"); }