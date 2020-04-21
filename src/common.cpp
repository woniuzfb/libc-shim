#include <libc_shim.h>

#include <stdexcept>
#include "common.h"
#include "pthreads.h"
#include "semaphore.h"
#include "network.h"
#include "dirent.h"
#include "cstdio.h"
#include "errno.h"
#include "ctype_data.h"
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#ifndef __APPLE__
#include <sys/auxv.h>
#endif

using namespace shim;

extern "C" unsigned long __umoddi3(unsigned long a, unsigned long b);
extern "C" unsigned long __udivdi3(unsigned long a, unsigned long b);
extern "C" long __divdi3(long a, long b);

extern "C" int __cxa_atexit(void (*)(void*), void*, void*);
extern "C" void __cxa_finalize(void * d);

uintptr_t bionic::stack_chk_guard = []() {
#ifndef __APPLE__
    return *((uintptr_t*) getauxval(AT_RANDOM));
#else
    return 0;
#endif
}();

int bionic::to_host_clock_type(bionic::clock_type type) {
    switch (type) {
        case clock_type::REALTIME: return CLOCK_REALTIME;
        case clock_type::MONOTONIC: return CLOCK_MONOTONIC;
        default: throw std::runtime_error("Unexpected clock type");
    }
}

void bionic::on_stack_chk_fail() {
    fprintf(stderr, "stack corruption has been detected\n");
    abort();
}

void shim::assert(const char *file, int line, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i: %s\n", file, line, msg);
    abort();
}

void shim::assert2(const char *file, int line, const char *function, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i %s: %s\n", file, line, function, msg);
    abort();
}

#ifndef __LP64__
int shim::ftruncate(int fd, bionic::off_t len) {
    return ::ftruncate(fd, (::off_t) len);
}

ssize_t shim::pread(int fd, void *buf, size_t len, bionic::off_t off) {
    return ::pread(fd, buf, len, (::off_t) off);
}

ssize_t shim::pwrite(int fd, const void *buf, size_t len, bionic::off_t off) {
    return ::pwrite(fd, buf, len, (::off_t) off);
}
#endif

int shim::clock_gettime(bionic::clock_type clock, struct timespec *ts) {
    return ::clock_gettime(bionic::to_host_clock_type(clock), ts);
}

void shim::add_common_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
#ifdef __APPLE__
        {"__errno", bionic::errno_with_translation},
        {"__set_errno", bionic::set_errno_with_translation},
#else
        {"__errno", __errno_location},
        {"__set_errno", bionic::set_errno_without_translation},
#endif

        {"__stack_chk_fail", &bionic::on_stack_chk_fail},
        {"__stack_chk_guard", &bionic::stack_chk_guard},

        {"__assert", assert},
        {"__assert2", assert2},

        {"__cxa_atexit", ::__cxa_atexit},
        {"__cxa_finalize", ::__cxa_finalize}
    });
}

void shim::add_stdlib_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"abort", abort},
        {"atexit", atexit},
        {"exit", exit},
        {"_Exit", _Exit},

        {"system", system},

        {"getenv", getenv},
        {"putenv", putenv},
        {"setenv", setenv},
        {"unsetenv", unsetenv},


        {"random", random},
        {"srandom", srandom},
        {"initstate", initstate},
        {"setstate", setstate},

        {"rand", rand},
        {"srand", srand},
        {"rand_r", rand_r},
        {"drand48", drand48},
        {"erand48", erand48},
        {"lrand48", lrand48},
        {"nrand48", nrand48},
        {"mrand48", mrand48},
        {"jrand48", jrand48},
        {"srand48", srand48},
        {"seed48", seed48},
        {"lcong48", lcong48},

        {"atof", atof},
        {"atoi", atoi},
        {"atol", atol},
        {"atoll", atoll},
        {"strtod", strtod},
        {"strtof", strtof},
        {"strtold", strtold},
        {"strtol", strtol},
        {"strtoul", strtoul},
        {"strtoq", strtoq},
        {"strtouq", strtouq},
        {"strtoll", strtoll},
        {"strtoull", strtoull},
        {"strtoul_l", strtoul_l},
        {"strtof_l", strtof_l},
        {"strtold_l", strtold_l},

        {"realpath", realpath},
        {"bsearch", bsearch},
        {"qsort", qsort},
        {"mblen", mblen},
        {"mbtowc", mbtowc},
        {"wctomb", wctomb},
        {"mbstowcs", mbstowcs},
        {"wcstombs", wcstombs},
        {"wcsrtombs", wcsrtombs},
    });
}

void shim::add_malloc_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"malloc", ::malloc},
        {"free", ::free},
        {"calloc", ::calloc},
        {"realloc", ::realloc},
        {"valloc", ::valloc},
        {"posix_memalign", ::posix_memalign},
        {"_Znwj", (void *(*)(size_t)) ::operator new},
        {"_ZdlPv", (void (*)(void *)) ::operator delete},
    });
}

void shim::add_ctype_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"_tolower_tab_", &_tolower_tab_},
        {"_toupper_tab_", &_toupper_tab_},
        {"_ctype_", &_ctype_},
        {"isalnum", isalnum},
        {"isalpha", isalpha},
        {"isblank", isblank},
        {"iscntrl", iscntrl},
        {"isdigit", isdigit},
        {"isgraph", isgraph},
        {"islower", islower},
        {"isprint", isprint},
        {"ispunct", ispunct},
        {"isspace", isspace},
        {"isupper", isupper},
        {"isxdigit", isxdigit},

        {"tolower", ::tolower},
        {"toupper", ::toupper},
    });
}

void shim::add_math_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"__umoddi3", __umoddi3},
        {"__udivdi3", __udivdi3},
        {"__divdi3", __divdi3},
        {"ldexp", (double (*)(double, int)) ::ldexp},
    });
}

void shim::add_time_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* sys/time.h */
        {"gettimeofday", ::gettimeofday},

        /* time.h */
        {"clock", ::clock},
        {"time", ::time},
        {"difftime", ::difftime},
        {"mktime", ::mktime},
        {"strftime", ::strftime},
        {"strptime", ::strptime},
        {"strftime_l", ::strftime_l},
        {"strptime_l", ::strptime_l},
        {"gmtime", ::gmtime},
        {"gmtime_r", ::gmtime_r},
        {"localtime", ::localtime},
        {"localtime_r", ::localtime_r},
        {"asctime", ::asctime},
        {"ctime", ::ctime},
        {"asctime_r", ::asctime_r},
        {"ctime_r", ::ctime_r},
        {"tzname", ::tzname},
        {"tzset", ::tzset},
        {"daylight", &::daylight},
        {"timezone", &::timezone},
        {"nanosleep", ::nanosleep},
        {"clock_gettime", clock_gettime},
    });
}
void shim::add_sched_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"sched_yield", ::sched_yield},
    });
}

void shim::add_unistd_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"access", ::access},
        {"lseek", ::lseek},
        {"close", ::close},
        {"read", ::read},
        {"write", ::write},
        {"pipe", ::pipe},
        {"alarm", ::alarm},
        {"sleep", ::sleep},
        {"usleep", ::usleep},
        {"pause", ::pause},
        {"chown", ::chown},
        {"fchown", ::fchown},
        {"lchown", ::lchown},
        {"chdir", ::chdir},
        {"fchdir", ::fchdir},
        {"getcwd", ::getcwd},
        {"dup", ::dup},
        {"dup2", ::dup2},
        {"execv", ::execv},
        {"execle", ::execle},
        {"execl", ::execl},
        {"execvp", ::execvp},
        {"execlp", ::execlp},
        {"nice", ::nice},
        {"_exit", ::_exit},
        {"getuid", ::getuid},
        {"getpid", ::getpid},
        {"getppid", ::getppid},
        {"getpgrp", ::getpgrp},
        {"fork", ::fork},
        {"vfork", ::vfork},
        {"isatty", ::isatty},
        {"link", ::link},
        {"symlink", ::symlink},
        {"readlink", ::readlink},
        {"unlink", ::unlink},
        {"rmdir", ::rmdir},
        {"gethostname", ::gethostname},
        {"fsync", ::fsync},
        {"sync", ::sync},
        {"getpagesize", ::getpagesize},
        {"getdtablesize", ::getdtablesize},
        {"syscall", ::syscall},
        {"lockf", ::lockf},
        {"swab", ::swab},

        /* Use our impl or fallback to system */
        {"ftruncate", ftruncate},
        {"pread", pread},
        {"pwrite", pwrite},
    });
}

std::vector<shimmed_symbol> shim::get_shimmed_symbols() {
    std::vector<shimmed_symbol> ret;
    add_common_shimmed_symbols(ret);
    add_stdlib_shimmed_symbols(ret);
    add_malloc_shimmed_symbols(ret);
    add_ctype_shimmed_symbols(ret);
    add_math_shimmed_symbols(ret);
    add_time_shimmed_symbols(ret);
    add_sched_shimmed_symbols(ret);
    add_unistd_shimmed_symbols(ret);
    add_pthread_shimmed_symbols(ret);
    add_sem_shimmed_symbols(ret);
    add_network_shimmed_symbols(ret);
    add_dirent_shimmed_symbols(ret);
    add_cstdio_shimmed_symbols(ret);
    return ret;
}