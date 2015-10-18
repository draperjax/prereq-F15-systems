#include "io61.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <errno.h>

/* Default Buffer Size & Maximum MMAP Size - selected after 
    performance testing */
#define BLOCKSIZE 4096
#define MMAP_MAX 3145728

/* IO61 Data Structure */
struct io61_file {
    int fd;
    int mode;

    /* Current Position in Buffer */
    size_t pos;
    
    /* Buffer Size, adjustable if blocks passed in */
    size_t buffer_sz;

    /* User input block size & detected file size */
    size_t block_sz;
    off_t file_sz;

    /* MMap Pointer & Offset */
    char* mmap_ptr;
    off_t mmap_offset;

    /* Cache Data Stucture */
    struct {
        size_t offset;
        unsigned char* data;
        size_t data_sz;
    } cache_slot;
};


// io61_fdopen(fd, mode)
//    Return a new io61_file that reads from and/or writes to the given
//    file descriptor `fd`. `mode` is either O_RDONLY for a read-only file
//    or O_WRONLY for a write-only file. You need not support read/write
//    files.

io61_file* io61_fdopen(int fd, int mode) {
    
    /* Initialize Caching Structure via MMAP Init */
    io61_file* f = io61_mmap(fd, mode);

    /* If MMAP Init fails, use IO61 Init */
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

    /* Store File size */
    off_t file_sz = io61_filesize(f);
    if (file_sz >= 0)
        f->file_sz = file_sz;

    /* Store Block size */
    off_t block_sz = io61_blocksize(f);
    if (block_sz > 0) {
        f->block_sz = block_sz;
        f->buffer_sz = block_sz;
    } else {
        f->buffer_sz = BLOCKSIZE;
    }
    
    /* Allocate memory for buffer */
    f->cache_slot.data = (unsigned char *) malloc(f->buffer_sz * sizeof (char));
    f->cache_slot.offset = 0;

    return f;
}

io61_file* io61_mmap(int fd, int mode) {
    if (mode == O_RDONLY) {
        io61_file* f = io61_initb(fd,mode);
        f->cache_slot.data = NULL;

        /* Check if file size is less than MMAP Max, and if so free structure & 
            return NULL to force use of IO61 Init. */
        if (f->file_sz <= MMAP_MAX) {
            free(f);
            return NULL;
        }

        /* Setup MMAP - 
            Desired Memory Protection: PROT_READ - Pages may be read
            Flags: 
                MMAP_PRIVATE: Protects mapping from other processes.
                MMAP_POPULATE: Reads ahead in file. */
        f->mmap_ptr = mmap(NULL, f->file_sz, PROT_READ, 
            MAP_PRIVATE | MAP_POPULATE, f->fd, 0);

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

    /* If open for write, flush cache before closing. */
    if (f->mode == O_WRONLY)
        io61_flush(f);

    int r = close(f->fd);
    free(f);
    return r;
}

size_t io61_blocksize(io61_file* f) {
    /* Per guidelines to avoid adjustment to io61_filesize, this function 
        performs similarly to capture blocksize. */
    struct stat file_info;
    int r = fstat(f->fd, &file_info);
    if (r >= 0 && S_ISREG(file_info.st_mode))
        return file_info.st_blksize;
    else
        return 0;
}

// io61_readc(f)
//    Read a single (unsigned) character from `f` and return it. Returns EOF
//    (which is -1) on error or end-of-file.
int io61_readc(io61_file* f) {

    /* Always check mmap first -- speeds up time */
    if(f->mmap_ptr != NULL) {
        if(f->mmap_offset == f->file_sz)
            return EOF;
        f->mmap_offset++;
        return *f->mmap_ptr++;
    }

    /* If Mapping does not exist, then leverage standard caching method */
    size_t pos = f->pos;

    /* Check if position is less than the size of the data in the cache */
    if (pos < f->cache_slot.data_sz) {
        f->pos += 1;
        return f->cache_slot.data[pos];
    } else {
        /* Attempt a system read to refill the buffer */
        f->cache_slot.data_sz = read(f->fd, f->cache_slot.data, f->buffer_sz);
        
        /* If nothing is returned, signal end of file */
        if (f->cache_slot.data_sz == 0)
            return EOF;
        else {
            /* Else, incrememnt the offset of the cache, reset the position 
                and return the first character. */
            f->cache_slot.offset += f->cache_slot.data_sz;
            f->pos = 1;
            return f->cache_slot.data[0];            
        }
    }
}

ssize_t mmap_read(io61_file* f, char* buf, size_t sz) 
{
    size_t n = sz;

    /* Check if size requested is more than file size and if so, adjust. */
    if ((size_t) f->file_sz < (size_t) f->mmap_offset + sz) {
        n = f->file_sz - f->mmap_offset;
    }

    /* Copy n bytes from MMAP Pointer position to User's Buffer. */
    memcpy(buf, f->mmap_ptr, n);

    /* Increment the MMAP Pointer and Offset. */
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
    
    /* Always check mmap first -- speeds up time */
    if(f->mmap_ptr != NULL) {
        return mmap_read(f, buf, sz);
    }

    size_t nread = 0;

    /* Loop through until the amount read equals size. */
    while (nread != sz) {
        if (sz == 1) {
            /* If requested a single character, use io61_readc */
            int ch = io61_readc(f);

            if (ch == EOF)
                break;
            
            /* Set value of current position in user buffer to 
                character returned from cache. */
            buf[nread] = ch;

            ++nread;
        } else {
            /* If we haven't reached the end of the buffer, then read */
            if (f->pos != f->cache_slot.data_sz) {
                size_t n = sz - nread;

                if (n > f->cache_slot.data_sz - f->pos)
                    n = f->cache_slot.data_sz - f->pos;

                memcpy(&buf[nread], &f->cache_slot.data[f->pos], n);
                f->pos += n;
                nread += n;
            } else {
                /* At end of buffer, so make another read & shift offset */
                f->cache_slot.offset += f->cache_slot.data_sz;
                f->pos = f->cache_slot.data_sz = 0;
                
                ssize_t n = read(f->fd, f->cache_slot.data, f->buffer_sz);
                
                if (n > 0) {
                    f->cache_slot.data_sz = n;
                } else {
                    if (nread > 0)
                        return nread;
                    else
                        return n;
                }
            }
        }
    }
    return nread;
}

// io61_writec(f)
//    Write a single character `ch` to `f`. Returns 0 on success or
//    -1 on error.

int io61_writec(io61_file* f, int ch) {
    size_t written = 0;

    /* If at buffer end, flush/reset cache and shift offset */
    if (f->pos >= f->buffer_sz) {
        written = write(f->fd, f->cache_slot.data, f->buffer_sz);

        if (written == f->buffer_sz) {
            f->cache_slot.offset += written;
            f->cache_slot.data_sz = 0;
            f->pos = 0;
        } else
            return -1;
    }

    /* Write the next character to cache & increment cache size. */
    f->cache_slot.data[f->pos] = ch;
    f->pos += 1;
    f->cache_slot.data_sz += 1;
    return 0;
}

// io61_write(f, buf, sz)
//    Write `sz` characters from `buf` to `f`. Returns the number of
//    characters written on success; normally this is `sz`. Returns -1 if
//    an error occurred before any characters were written.

ssize_t io61_write(io61_file* f, const char* buf, size_t sz) {
    size_t nwritten = 0;

    /* Loop through until the amount written equals size. */
    while (nwritten != sz) {
        if (sz == 1) {
            if (io61_writec(f, buf[nwritten]) == -1)
                break;
            ++nwritten;
        } else {
            /* If at buffer end, flush/reset cache and shift offset */
            if (f->cache_slot.data_sz >= f->buffer_sz) {
                ssize_t n = io61_flush(f);
                if (n < 0)
                    return nwritten;
            }

            /* Adjust to only write size not yet written. */
            size_t n = sz - nwritten;

            /* Adjust to only write size remaining in buffer. */
            if (n > BLOCKSIZE - f->cache_slot.data_sz)
                n = BLOCKSIZE - f->cache_slot.data_sz;

            /* Copy n bytes from Cache to User's Buffer. */
            memcpy(&f->cache_slot.data[f->pos], &buf[nwritten], n);
            f->pos += n;
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

    /* If file open for write, make system write, 
        reset buffer & buffer position. */
    if (f->mode == O_WRONLY) {
        if (f->cache_slot.data_sz > 0) {
            size_t written = write(f->fd, f->cache_slot.data, f->cache_slot.data_sz);
            if (written > 0) {
                f->cache_slot.offset += written;
                f->cache_slot.data_sz = 0;
                f->pos = 0;
                return written;
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

    /* If using MMAP, leverage MMAP Seek. */
    if (f->mmap_ptr != NULL && f->mode == O_RDONLY)
        return io61_mmap_seek(f, pos);

    /* If file open for read, and within cache bounds, seek. */
    if (f->mode == O_RDONLY) {
        if (pos >= (off_t) f->cache_slot.offset && 
            pos <= (off_t) f->cache_slot.offset + (off_t) f->cache_slot.data_sz) {
            f->pos = pos - f->cache_slot.offset;
            return 0;
        } else {
            r = lseek(f->fd, (off_t) pos, SEEK_SET);
            if (r == (off_t) pos) {
                f->pos = f->cache_slot.data_sz = 0;
                f->cache_slot.offset = pos;
                return 0;
            } else
                return -1;
        }
    } else if (f->mode == O_WRONLY) {
        /* If file open for write, flush then seek. */
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
    /* If position is out of range, return -1. */
    if(pos < 0 || pos > f->file_sz) {
        return -1;
    }

    /* Otherwise, adjust the MMAP pointer & Offset. */
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
