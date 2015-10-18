#include "io61.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <errno.h>

#define BLOCKSIZE 4096
#define MMAP_MAX 3145728

struct io61_file {
    int fd;
    int mode;
    size_t pos;
    size_t buffer_sz;
    size_t block_sz;
    off_t file_sz;

    char* mmap_ptr;
    off_t mmap_offset;

    struct {
        size_t offset;
        unsigned char* data;
        void* data_ptr;
        size_t data_sz;
    } read_cache_slot;

    struct {
        size_t offset;
        unsigned char* data;
        void* data_ptr;
        size_t data_sz;
    } write_cache_slot;
};


// io61_fdopen(fd, mode)
//    Return a new io61_file that reads from and/or writes to the given
//    file descriptor `fd`. `mode` is either O_RDONLY for a read-only file
//    or O_WRONLY for a write-only file. You need not support read/write
//    files.

io61_file* io61_fdopen(int fd, int mode) {
    
    io61_file* f = io61_mmap(fd, mode);

    if (f != NULL) {
        return f;
    } else {
        io61_file* f = io61_initb(fd, mode);
        return f;
    }
}

io61_file* io61_initb(int fd, int mode) {
    io61_file* f = (io61_file*) malloc(sizeof(io61_file));

    f->fd = fd;  
    f->mode = mode;
    f->pos = 0;

    off_t file_sz = io61_filesize(f);
    if (file_sz >= 0)
        f->file_sz = file_sz;

    off_t block_sz = io61_blocksize(f);
    if (block_sz > 0) {
        f->block_sz = block_sz;
        f->buffer_sz = block_sz;
    } else {
        f->buffer_sz = BLOCKSIZE;
    }
    
    if (mode == O_WRONLY) {
        f->write_cache_slot.data = (unsigned char *) malloc(f->buffer_sz * sizeof (char));
        f->write_cache_slot.offset = 0;
        f->pos = 0;
        f->write_cache_slot.data_ptr = f->write_cache_slot.data;
    } else {
        f->read_cache_slot.data = (unsigned char *) malloc(f->buffer_sz * sizeof (char));
        f->read_cache_slot.offset = 0;
    }

    return f;
}

io61_file* io61_mmap(int fd, int mode) {
    if (mode == O_RDONLY) 
    {
        io61_file* f = io61_initb(fd,mode);
        f->read_cache_slot.data = NULL;

        if (f->file_sz <= MMAP_MAX) {
            free(f);
            return NULL;
        }

        f->mmap_ptr = mmap(NULL, f->file_sz, PROT_READ, MAP_PRIVATE | MAP_POPULATE, f->fd, 0);
        f->mmap_offset = 0;

        return f;
    }
    else
        return NULL;
}

// io61_close(f)
//    Close the io61_file `f` and release all its resources, including
//    any buffers.

int io61_close(io61_file* f) {
    if (f->mode == O_WRONLY)
        io61_flush(f);
    int r = close(f->fd);
    free(f);
    return r;
}

size_t io61_blocksize(io61_file* f) {
    struct stat file_info;
    if (fstat(f->fd, &file_info) != 0)
        return 0;
    else
        return file_info.st_blksize;
}
// io61_readc(f)
//    Read a single (unsigned) character from `f` and return it. Returns EOF
//    (which is -1) on error or end-of-file.

int io61_readc(io61_file* f) {

    if(f->mmap_ptr != NULL)
        return mmap_readc(f);

    size_t pos = f->pos;
    if (pos < f->read_cache_slot.data_sz) 
    {
        f->pos += 1;
        return f->read_cache_slot.data[pos];
    } else {
        f->read_cache_slot.data_sz = read(f->fd, f->read_cache_slot.data, f->buffer_sz);
        if (f->read_cache_slot.data_sz < 1)
            return EOF;
        else {
            f->read_cache_slot.offset += f->read_cache_slot.data_sz;
            f->pos = 1;
            return f->read_cache_slot.data[0];            
        }
    }
}

int mmap_readc(io61_file* f) {
    if(f->mmap_offset == f->file_sz) {
        return EOF;
    }

    f->mmap_offset++;
    return (unsigned char)*f->mmap_ptr++;
}

ssize_t mmap_read(io61_file* f, char* buf, size_t sz) 
{
    size_t n = sz;
    if ((size_t) f->file_sz < (size_t) f->mmap_offset + sz)
    {
        n = f->file_sz - f->mmap_offset;
    }

    memcpy(buf, f->mmap_ptr, n);

    f->mmap_ptr += n;
    f->mmap_offset += n;

    return n;
}

// io61_read(f, buf, sz)
//    Read up to `sz` characters from `f` into `buf`. Returns the number of
//    characters read on success; normally this is `sz`. Returns a short
//    count if the file ended before `sz` characters could be read. Returns
//    -1 an error occurred before any characters were read.

ssize_t io61_read(io61_file* f, char* buf, size_t sz) {
    
    if(f->mmap_ptr != NULL) 
    {
        return mmap_read(f, buf, sz);
    }

    size_t nread = 0;

    while (nread != sz) {
        if (sz == 1) {
            int ch = io61_readc(f);
            if (ch == EOF)
                break;
            buf[nread] = ch;
            ++nread;
        } else {
            if (f->pos != f->read_cache_slot.data_sz) {
                size_t n = sz - nread;

                if (n > f->read_cache_slot.data_sz - f->pos)
                    n = f->read_cache_slot.data_sz - f->pos;

                memcpy(&buf[nread], &f->read_cache_slot.data[f->pos], n);
                // printf("      Cache (R) %i\n", n);

                f->pos += n;

                // printf("      Cache (R) - Position %i\n", f->pos);
                nread += n;

            } else {
                f->read_cache_slot.offset += f->read_cache_slot.data_sz;
                f->pos = f->read_cache_slot.data_sz = 0;
                
                ssize_t n = read(f->fd, f->read_cache_slot.data, f->buffer_sz);
                
                if (n > 0) {
                    // printf("      System (R) %i\n", n);
                    f->read_cache_slot.data_sz = n;
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

    size_t pos = f->pos;
    size_t written = 0;

    if (pos >= f->buffer_sz) {
        written = write(f->fd, f->write_cache_slot.data, f->buffer_sz);

        if (written == f->buffer_sz) {
            f->write_cache_slot.offset += written;
            pos = 0;

            f->write_cache_slot.data[pos] = ch;
            f->pos = 1;
            f->write_cache_slot.data_sz = 1;
            return 0;
        } else
            return -1;
    } else {
        f->write_cache_slot.data[pos] = ch;
        f->pos += 1;
        f->write_cache_slot.data_sz += 1;
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
        if (sz == 1) {
            if (io61_writec(f, buf[nwritten]) == -1)
                break;
            ++nwritten;
        } else {
            if (f->write_cache_slot.data_sz >= f->buffer_sz) {
                // printf("      System (W) %i\n", f->cache_slot.data_sz);

                // Correct for the offset position if the seek is out of range
                ssize_t n = write(f->fd, f->write_cache_slot.data, f->write_cache_slot.data_sz);
                if (n > 0) {
                    f->write_cache_slot.offset += n;
                    f->write_cache_slot.data_sz = 0;
                    f->pos = 0;
                } else
                    return nwritten ? nwritten : n;
            }

            size_t n = sz - nwritten;

            if (n > BLOCKSIZE - f->write_cache_slot.data_sz)
                n = BLOCKSIZE - f->write_cache_slot.data_sz;

            // printf("\n      Cache (W) %i\n", n);
            memcpy(&f->write_cache_slot.data[f->pos], &buf[nwritten], n);
            f->pos += n;
            // printf("      Cache (W) - Position %i\n", f->pos);
            f->write_cache_slot.data_sz += n;
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
    if (f->mode == O_WRONLY) {
        if (f->write_cache_slot.data_sz > 0) {
            size_t written = write(f->fd, f->write_cache_slot.data, f->write_cache_slot.data_sz);
            if (written > 0) {
                f->write_cache_slot.offset += written;
                f->write_cache_slot.data_sz = 0;
                f->pos = 0;
            } else
                return -1;
        }
    }
    return 0;
}



// io61_seek(f, pos)
//    Change the file pointer for file `f` to `pos` bytes into the file.
//    Returns 0 on success and -1 on failure.

int io61_seek(io61_file* f, off_t pos) {
    off_t r = 0;

    if (f->mmap_ptr != NULL && f->mode == O_RDONLY)
        return io61_mmap_seek(f, pos);

    if (f->mode == O_RDONLY) {
        if (pos >= (off_t) f->read_cache_slot.offset && 
            pos <= (off_t) f->read_cache_slot.offset + (off_t) f->read_cache_slot.data_sz) {
            f->pos = pos - f->read_cache_slot.offset;
            return 0;
        } else {
            r = lseek(f->fd, (off_t) pos, SEEK_SET);
            if (r == (off_t) pos) {
                f->pos = f->read_cache_slot.data_sz = 0;
                f->read_cache_slot.offset = pos;
                return 0;
            } else
                return -1;
        }
    } else if (f->mode == O_WRONLY) {
        io61_flush(f);

        r = lseek(f->fd, (off_t) pos, SEEK_SET);

        if (r == (off_t) pos)
            return 0;
        else
            return -1;
    }

    return 0;
}

int io61_mmap_seek(io61_file* f, off_t pos) 
{
    if(pos < 0 || pos > f->file_sz) {
        return -1;
    }

    f->mmap_ptr += (pos - f->mmap_offset);
    f->mmap_offset = pos;

    return 0;
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
