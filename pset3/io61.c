#include "io61.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#define BLOCKSIZE 4096
// io61.c
//    YOUR CODE HERE!


// io61_file
//    Data structure for io61 file wrappers. Add your own stuff.

struct io61_file {
    int fd;
    int mode;
    size_t seek;
    struct {
        size_t offset;
        unsigned char data[BLOCKSIZE];
        void* data_ptr;
        size_t data_sz;
    } cache_slot;
};


// io61_fdopen(fd, mode)
//    Return a new io61_file that reads from and/or writes to the given
//    file descriptor `fd`. `mode` is either O_RDONLY for a read-only file
//    or O_WRONLY for a write-only file. You need not support read/write
//    files.

io61_file* io61_fdopen(int fd, int mode) {
    assert(fd >= 0);
    io61_file* f = (io61_file*) malloc(sizeof(io61_file));
    io61_initb(f,fd);
    f->mode = mode;
    (void) mode;
    return f;
}

void io61_initb(io61_file* f, int fd) {
    f->fd = fd;  
    f->seek = 0;
    f->cache_slot.offset = 0;
    f->cache_slot.data_ptr = NULL;
}

// io61_close(f)
//    Close the io61_file `f` and release all its resources, including
//    any buffers.

int io61_close(io61_file* f) {
    io61_flush(f);
    int r = close(f->fd);
    free(f);
    return r;
}

// io61_readc(f)
//    Read a single (unsigned) character from `f` and return it. Returns EOF
//    (which is -1) on error or end-of-file.

int io61_readc(io61_file* f) {
    size_t seek = f->seek;
    if (seek < f->cache_slot.data_sz) 
    {
        f->seek += 1;
        return f->cache_slot.data[seek];
    } else {
        f->cache_slot.data_sz = read(f->fd, f->cache_slot.data, BLOCKSIZE);
        if (f->cache_slot.data_sz < 1)
            return EOF;
        else {
            f->cache_slot.offset += f->cache_slot.data_sz;
            f->seek = 1;
            return f->cache_slot.data[0];            
        }
    }
}

// io61_read(f, buf, sz)
//    Read up to `sz` characters from `f` into `buf`. Returns the number of
//    characters read on success; normally this is `sz`. Returns a short
//    count if the file ended before `sz` characters could be read. Returns
//    -1 an error occurred before any characters were read.

ssize_t io61_read(io61_file* f, char* buf, size_t sz) {
    size_t nread = 0;

    while (nread != sz) {
        if (sz >= BLOCKSIZE) {
            size_t bytes_read = read(f->fd, buf, sz);
            if (bytes_read != sz)
                break;
            else
                nread += bytes_read;
        } else if (sz == 1) {
            int ch = io61_readc(f);
            if (ch == EOF)
                break;
            buf[nread] = ch;
            ++nread;
        } else {
            if (f->seek != f->cache_slot.data_sz) {
                size_t n = sz - nread;

                if (n > f->cache_slot.data_sz - f->seek)
                    n = f->cache_slot.data_sz - f->seek;

                memcpy(&buf[nread], &f->cache_slot.data[f->seek], n);
                // printf("      Cache (R) %i\n", n);

                f->seek += n;

                // printf("      Cache (R) - Position %i\n", f->seek);
                nread += n;

            } else {
                f->cache_slot.offset += f->cache_slot.data_sz;
                f->seek = f->cache_slot.data_sz = 0;
                
                ssize_t n = read(f->fd, f->cache_slot.data, BLOCKSIZE);
                
                if (n > 0) {
                    // printf("      System (R) %i\n", n);
                    f->cache_slot.data_sz = n;
                } else
                    return nread ? nread : n;
            }
        }
    }

    return nread;

}

// io61_writec(f)
//    Write a single character `ch` to `f`. Returns 0 on success or
//    -1 on error.

int io61_writec(io61_file* f, int ch) {

    size_t seek = f->seek;
    size_t written = 0;

    if (seek >= BLOCKSIZE) {
        written = write(f->fd, f->cache_slot.data, BLOCKSIZE);

        if (written == BLOCKSIZE) {
            f->cache_slot.offset += BLOCKSIZE;
            seek = 0;

            f->cache_slot.data[seek] = ch;
            f->seek = 1;
            f->cache_slot.data_sz = 1;
            return 0;
        } else
            return -1;
    } else {
        f->cache_slot.data[seek] = ch;
        f->seek += 1;
        f->cache_slot.data_sz += 1;
        return 0;
    };
}

// io61_write(f, buf, sz)
//    Write `sz` characters from `buf` to `f`. Returns the number of
//    characters written on success; normally this is `sz`. Returns -1 if
//    an error occurred before any characters were written.

ssize_t io61_write(io61_file* f, const char* buf, size_t sz) {
    size_t nwritten = 0;

    // printf("      Size (W) %i\n", sz);
    while (nwritten != sz) {
        if (sz >= BLOCKSIZE) {
            // printf("      System (W) %i\n", f->cache_slot.data_sz);
            size_t r = write(f->fd, buf, sz);
            if (r != sz)
                break;
            else
                nwritten += r;
        } else if (sz == 1) {
            if (io61_writec(f, buf[nwritten]) == -1)
                break;
            ++nwritten;
        } else {
            if (f->cache_slot.data_sz >= BLOCKSIZE) {
                // printf("      System (W) %i\n", f->cache_slot.data_sz);
                ssize_t n = write(f->fd, f->cache_slot.data, f->cache_slot.data_sz);
                if (n > 0) {
                    f->cache_slot.offset += BLOCKSIZE;
                    f->cache_slot.data_sz = 0;
                    f->seek = 0;
                } else
                    return nwritten ? nwritten : n;
            }

            size_t n = sz - nwritten;

            if (n > BLOCKSIZE - f->cache_slot.data_sz)
                n = BLOCKSIZE - f->cache_slot.data_sz;

            // printf("\n      Cache (W) %i\n", n);
            memcpy(&f->cache_slot.data[f->seek], buf, n);
            f->seek += n;
            // printf("      Cache (W) - Position %i\n", f->seek);
            f->cache_slot.data_sz += n;
            nwritten += n;
        }
    }

    if (nwritten != 0 || sz == 0)
        return nwritten;
    else
        return -1;
}

// io61_flush(f)
//    Forces a write of all buffered data written to `f`.
//    If `f` was opened read-only, io61_flush(f) may either drop all
//    data buffered for reading, or do nothing.

int io61_flush(io61_file* f) {
    (void) f;
    if (f->cache_slot.data_sz > 0) {
        size_t written = write(f->fd, f->cache_slot.data, f->cache_slot.data_sz);
        if (written > 0) {
            f->cache_slot.offset += BLOCKSIZE;
            f->cache_slot.data_sz = 0;
            f->seek = 0;
        } else
            return -1;
    }
    return 0;
}



// io61_seek(f, pos)
//    Change the file pointer for file `f` to `pos` bytes into the file.
//    Returns 0 on success and -1 on failure.

int io61_seek(io61_file* f, off_t pos) {
    // if (f->mode == O_WRONLY)
    //     io61_flush(f);

    if (pos >= (off_t) f->cache_slot.offset && pos <= (off_t) f->cache_slot.offset + (off_t) f->cache_slot.data_sz)
    {
        f->seek = pos - f->cache_slot.offset;
        return 0;
    } else {
        off_t r = lseek(f->fd, (off_t) pos, SEEK_SET);
        if (r == (off_t) pos) {
            f->seek = f->cache_slot.data_sz = 0;
            f->cache_slot.offset = pos;
            return 0;
        } else
            return -1;
    }
}


// You shouldn't need to change these functions.

// io61_open_check(filename, mode)
//    Open the file corresponding to `filename` and return its io61_file.
//    If `filename == NULL`, returns either the standard input or the
//    standard output, depending on `mode`. Exits with an error message if
//    `filename != NULL` and the named file cannot be opened.

io61_file* io61_open_check(const char* filename, int mode) {
    int fd;
    if (filename)
        fd = open(filename, mode, 0666);
    else if ((mode & O_ACCMODE) == O_RDONLY)
        fd = STDIN_FILENO;
    else
        fd = STDOUT_FILENO;
    if (fd < 0) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        exit(1);
    }
    return io61_fdopen(fd, mode & O_ACCMODE);
}


// io61_filesize(f)
//    Return the size of `f` in bytes. Returns -1 if `f` does not have a
//    well-defined size (for instance, if it is a pipe).

off_t io61_filesize(io61_file* f) {
    struct stat s;
    int r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode))
        return s.st_size;
    else
        return -1;
}


// io61_eof(f)
//    Test if readable file `f` is at end-of-file. Should only be called
//    immediately after a `read` call that returned 0 or -1.

int io61_eof(io61_file* f) {
    char x;
    ssize_t nread = read(f->fd, &x, 1);
    if (nread == 1) {
        fprintf(stderr, "Error: io61_eof called improperly\n\
  (Only call immediately after a read() that returned 0 or -1.)\n");
        abort();
    }
    return nread == 0;
}
