#pragma once

#include <time.h>
#include <sys/stat.h>
#include <libc_shim.h>

namespace shim {

    namespace bionic {

#ifdef __x86_64__
        struct stat {
            unsigned long st_dev;
            unsigned long st_ino;
            unsigned long st_nlink;
            unsigned int st_mode;
            unsigned int st_uid;
            unsigned int st_gid;
            unsigned int __pad0;
            unsigned long st_rdev;
            long st_size;
            long st_blksize;
            long st_blocks;
            timespec st_atim;
            timespec st_mtim;
            timespec st_ctim;
            long __pad3[3];
        };
#else
        struct stat {
            unsigned long long st_dev;
            unsigned char __pad0[4];
            unsigned long __st_ino;
            unsigned int st_mode;
            unsigned int st_nlink;
            unsigned int st_uid;
            unsigned int st_gid;
            unsigned long long st_rdev;
            unsigned char __pad3[4];
            long long st_size;
            unsigned long st_blksize;
            unsigned long long st_blocks;
            timespec st_atim;
            timespec st_mtim;
            timespec st_ctim;
            unsigned long long st_ino;
        };
#endif

#ifndef __APPLE__
        void from_host(struct ::stat64 const &info, stat &result);
#else
        void from_host(struct ::stat const &info, stat &result);
#endif

    }

    int stat(const char *path, bionic::stat *s);
    int fstat(int fd, bionic::stat *s);

    void add_stat_shimmed_symbols(std::vector<shimmed_symbol> &list);

}