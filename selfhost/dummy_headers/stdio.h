#ifndef STDIO_H
#define STDIO_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define SEEK_SET 0
#define SEEK_END 2

typedef long size_t;
typedef struct _IO_FILE FILE;
typedef void _IO_lock_t;
typedef void *__off_t;

struct _IO_FILE {
    int _flags; /* High-order word is _IO_MAGIC; rest is flags. */

    /* The following pointers correspond to the C++ streambuf protocol. */
    char *_IO_read_ptr;   /* Current read pointer */
    char *_IO_read_end;   /* End of get area. */
    char *_IO_read_base;  /* Start of putback+get area. */
    char *_IO_write_base; /* Start of put area. */
    char *_IO_write_ptr;  /* Current put pointer. */
    char *_IO_write_end;  /* End of put area. */
    char *_IO_buf_base;   /* Start of reserve area. */
    char *_IO_buf_end;    /* End of reserve area. */

    /* The following fields are used to support backing up and undo. */
    char *_IO_save_base;   /* Pointer to start of non-current get area. */
    char *_IO_backup_base; /* Pointer to first valid character of backup area */
    char *_IO_save_end;    /* Pointer to end of non-current get area. */

    struct _IO_marker *_markers;

    struct _IO_FILE *_chain;

    int _fileno;
    int _flags2;

    __off_t _old_offset; /* This used to be _offset but it's too small.  */

    /* 1+column number of pbase(); 0 is unknown. */
    // unsigned short _cur_column;
    // signed char _vtable_offset;
    int _cur_column;
    char _vtable_offset;
    char _shortbuf[1];

    _IO_lock_t *_lock;
};

void exit(int status);
FILE *fopen(char *filename, char *mode);
int fseek(FILE *stream, long offset, int origin);
long ftell(FILE *stream);
size_t fread(void *ptr, size_t size, size_t n, FILE *stream);
int fclose(FILE *stream);
int printf(char *format, ...);
int fprintf(FILE *stream, char *format, ...);
int snprintf(char *s, size_t n, char *format, ...);
int vfprintf(FILE *stream, char *format, va_list args);
int puts(char *s);
int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int getc(FILE *stream);
int getchar(void);
int ungetc(int c, FILE *stream);
int scanf(char *format, ...);
int fscanf(FILE *stream, char *format, ...);
int sscanf(char *str, char *format, ...);
int sprintf(char *str, char *format, ...);

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#endif