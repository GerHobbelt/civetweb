/* Copyright (c) 2013-2024 the Civetweb developers
 * Copyright (c) 2004-2013 Sergey Lyubka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CIVETWEB_SYS_PORTING_INCLUDE
#define CIVETWEB_SYS_PORTING_INCLUDE

#if defined(__GNUC__) || defined(__MINGW32__)
#ifndef GCC_VERSION
#define GCC_VERSION                                                            \
	(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif
#if GCC_VERSION >= 40500
/* gcc diagnostic pragmas available */
#define GCC_DIAGNOSTIC
#endif
#endif

#if defined(GCC_DIAGNOSTIC)
/* Disable unused macros warnings - not all defines are required
 * for all systems and all compilers. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* A padding warning is just plain useless */
#pragma GCC diagnostic ignored "-Wpadded"
#endif

#if defined(__clang__) /* GCC does not (yet) support this pragma */
/* We must set some flags for the headers we include. These flags
 * are reserved ids according to C99, so we need to disable a
 * warning for that. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#endif

#if defined(_WIN32)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS /* Disable deprecation warning in VS2005 */
#endif
#if !defined(_WIN32_WINNT) /* Minimum API version */
#define _WIN32_WINNT 0x0601
#endif
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN      // Disable WIN32_LEAN_AND_MEAN, if necessary
#endif
#else
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* for setgroups(), pthread_setname_np() */
#endif
#if defined(__linux__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600 /* For flockfile() on Linux */
#endif
#if defined(__LSB_VERSION__) || defined(__sun)
#define NEED_TIMEGM
#define NO_THREAD_NAME
#endif
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE /* For fseeko(), ftello() */
#endif
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64 /* Use 64-bit file offsets by default */
#endif
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS /* <inttypes.h> wants this for C++ */
#endif
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS /* C++ wants that for INT64_MAX */
#endif
#if !defined(_DARWIN_UNLIMITED_SELECT)
#define _DARWIN_UNLIMITED_SELECT
#endif
#if defined(__sun)
#define __EXTENSIONS__  /* to expose flockfile and friends in stdio.h */
#define __inline inline /* not recognized on older compiler versions */
#endif
#endif

#if defined(__clang__)
/* Enable reserved-id-macro warning again. */
#pragma GCC diagnostic pop
#endif


#if defined(USE_LUA)
#define USE_TIMERS
#endif

#if defined(_MSC_VER)
/* 'type cast' : conversion from 'int' to 'HANDLE' of greater size */
#pragma warning(disable : 4306)
/* conditional expression is constant: introduced by FD_SET(..) */
#pragma warning(disable : 4127)
/* expression before comma has no effect; expected expression with side-effect: introduced by FD_SET in VC2017 */
#pragma warning(disable : 4548)
/* non-constant aggregate initializer: issued due to missing C99 support */
#pragma warning(disable : 4204)
/* padding added after data member */
#pragma warning(disable : 4820)
/* not defined as a preprocessor macro, replacing with '0' for '#if/#elif' */
#pragma warning(disable : 4668)
/* no function prototype given: converting '()' to '(void)' */
#pragma warning(disable : 4255)
/* function has been selected for automatic inline expansion */
#pragma warning(disable : 4711)
#endif


/*
 * improve memory debugging on WIN32 by using crtdbg.h (only MSVC
 * compiler and debug builds!)
 *
 * make sure crtdbg.h is loaded before malloc.h!
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// Studio 2008+
# if (defined(WIN32) || defined(__WIN32)) && !defined(UNDER_CE)
#  if defined(DEBUG) || defined(_DEBUG)
#   ifndef _CRTDBG_MAP_ALLOC
#    define _CRTDBG_MAP_ALLOC 1
#   endif
#  endif
#  include <crtdbg.h>
#  include <malloc.h>
# endif
#endif


/* This code uses static_assert to check some conditions.
 * Unfortunately some compilers still do not support it, so we have a
 * replacement function here. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201100L
#define mg_static_assert _Static_assert
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define mg_static_assert static_assert
#else
char static_assert_replacement[1];
#define mg_static_assert(cond, txt)                                            \
	extern char static_assert_replacement[(cond) ? 1 : -1]
#endif

mg_static_assert(sizeof(int) == 4 || sizeof(int) == 8,
                 "int data type size check");
mg_static_assert(sizeof(void *) == 4 || sizeof(void *) == 8,
                 "pointer data type size check");
mg_static_assert(sizeof(void *) >= sizeof(int), "data type size check");


/* Select queue implementation. Diagnosis features originally only implemented
 * for the "ALTERNATIVE_QUEUE" have been ported to the previous queue
 * implementation (NO_ALTERNATIVE_QUEUE) as well. The new configuration value
 * "CONNECTION_QUEUE_SIZE" is only available for the previous queue
 * implementation, since the queue length is independent from the number of
 * worker threads there, while the new queue is one element per worker thread.
 *
 */
#if defined(NO_ALTERNATIVE_QUEUE) && defined(ALTERNATIVE_QUEUE)
/* The queues are exclusive or - only one can be used. */
#error                                                                         \
    "Define ALTERNATIVE_QUEUE or NO_ALTERNATIVE_QUEUE (or none of them), but not both"
#endif
#if !defined(NO_ALTERNATIVE_QUEUE) && !defined(ALTERNATIVE_QUEUE)
/* Use a default implementation */
#define NO_ALTERNATIVE_QUEUE
#endif

#if defined(NO_FILESYSTEMS) && !defined(NO_FILES)
/* File system access:
 * NO_FILES = do not serve any files from the file system automatically.
 * However, with NO_FILES CivetWeb may still write log files, read access
 * control files, default error page files or use API functions like
 * mg_send_file in callbacks to send files from the server local
 * file system.
 * NO_FILES only disables the automatic mapping between URLs and local
 * file names.
 * NO_FILESYSTEM = do not access any file at all. Useful for embedded
 * devices without file system. Logging to files in not available
 * (use callbacks instead) and API functions like mg_send_file are not
 * available.
 * If NO_FILESYSTEM is set, NO_FILES must be set as well.
 */
#error "Inconsistent build flags, NO_FILESYSTEMS requires NO_FILES"
#endif

#if defined(__SYMBIAN32__)
/* According to https://en.wikipedia.org/wiki/Symbian#History,
 * Symbian is no longer maintained since 2014-01-01.
 * Support for Symbian has been removed from CivetWeb
 */
#error "Symbian is no longer maintained. CivetWeb no longer supports Symbian."
#endif /* __SYMBIAN32__ */

#if defined(__rtems__)
#include <rtems/version.h>
#endif

#if defined(__ZEPHYR__)
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include <zephyr/kernel.h>

/* Max worker threads is the max of pthreads minus the main application thread
 * and minus the main civetweb thread, thus -2
 */
#define MAX_WORKER_THREADS (CONFIG_MAX_PTHREAD_COUNT - 2)

#if defined(USE_STACK_SIZE) && (USE_STACK_SIZE > 1)
#define ZEPHYR_STACK_SIZE USE_STACK_SIZE
#else
#define ZEPHYR_STACK_SIZE (1024 * 16)
#endif

K_THREAD_STACK_DEFINE(civetweb_main_stack, ZEPHYR_STACK_SIZE);
K_THREAD_STACK_ARRAY_DEFINE(civetweb_worker_stacks,
                            MAX_WORKER_THREADS,
                            ZEPHYR_STACK_SIZE);

static int zephyr_worker_stack_index;

#endif

/* Include the header file here, so the CivetWeb interface is defined for the
 * entire implementation, including the following forward definitions. */
#include "civetweb.h"

#if !defined(DEBUG_TRACE)
#if defined(DEBUG)
static void DEBUG_TRACE_FUNC(const char *func,
                             unsigned line,
                             PRINTF_FORMAT_STRING(const char *fmt),
                             ...) PRINTF_ARGS(3, 4);

#define DEBUG_TRACE(fmt, ...)                                                  \
	DEBUG_TRACE_FUNC(__func__, __LINE__, fmt, __VA_ARGS__)

#define NEED_DEBUG_TRACE_FUNC
#if !defined(DEBUG_TRACE_STREAM)
#define DEBUG_TRACE_STREAM stdout
#endif

#else
#define DEBUG_TRACE(fmt, ...)                                                  \
	do {                                                                       \
	} while (0)
#endif /* DEBUG */
#endif /* DEBUG_TRACE */


#if !defined(DEBUG_ASSERT)
#if defined(DEBUG)
#include <stdlib.h>
#define DEBUG_ASSERT(cond)                                                     \
	do {                                                                       \
		if (!(cond)) {                                                         \
			DEBUG_TRACE("ASSERTION FAILED: %s", #cond);                        \
			exit(2); /* Exit with error */                                     \
		}                                                                      \
	} while (0)
#else
#define DEBUG_ASSERT(cond)
#endif /* DEBUG */
#endif


#if defined(__GNUC__) && defined(GCC_INSTRUMENTATION)
void __cyg_profile_func_enter(void *this_fn, void *call_site)
    __attribute__((no_instrument_function));

void __cyg_profile_func_exit(void *this_fn, void *call_site)
    __attribute__((no_instrument_function));

void
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
	if ((void *)this_fn != (void *)printf) {
		printf("E %p %p\n", this_fn, call_site);
	}
}

void
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
	if ((void *)this_fn != (void *)printf) {
		printf("X %p %p\n", this_fn, call_site);
	}
}
#endif


#if !defined(IGNORE_UNUSED_RESULT)
#define IGNORE_UNUSED_RESULT(a) ((void)((a) && 1))
#endif


#if defined(__GNUC__) || defined(__MINGW32__)

/* GCC unused function attribute seems fundamentally broken.
 * Several attempts to tell the compiler "THIS FUNCTION MAY BE USED
 * OR UNUSED" for individual functions failed.
 * Either the compiler creates an "unused-function" warning if a
 * function is not marked with __attribute__((unused)).
 * On the other hand, if the function is marked with this attribute,
 * but is used, the compiler raises a completely idiotic
 * "used-but-marked-unused" warning - and
 *   #pragma GCC diagnostic ignored "-Wused-but-marked-unused"
 * raises error: unknown option after "#pragma GCC diagnostic".
 * Disable this warning completely, until the GCC guys sober up
 * again.
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#define FUNCTION_MAY_BE_UNUSED /* __attribute__((unused)) */

#else
#define FUNCTION_MAY_BE_UNUSED
#endif


/* Some ANSI #includes are not available on Windows CE and Zephyr */
#if !defined(_WIN32_WCE) && !defined(__ZEPHYR__)
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif /* !_WIN32_WCE */


#if defined(__clang__)
/* When using -Weverything, clang does not accept it's own headers
 * in a release build configuration. Disable what is too much in
 * -Weverything. */
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

#if defined(__GNUC__) || defined(__MINGW32__)
/* Who on earth came to the conclusion, using __DATE__ should rise
 * an "expansion of date or time macro is not reproducible"
 * warning. That's exactly what was intended by using this macro.
 * Just disable this nonsense warning. */

/* And disabling them does not work either:
 * #pragma clang diagnostic ignored "-Wno-error=date-time"
 * #pragma clang diagnostic ignored "-Wdate-time"
 * So we just have to disable ALL warnings for some lines
 * of code.
 * This seems to be a known GCC bug, not resolved since 2012:
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431
 */
#endif


#if defined(__MACH__) && defined(__APPLE__) /* Apple OSX section */

#if defined(__clang__)
#if (__clang_major__ == 3) && ((__clang_minor__ == 7) || (__clang_minor__ == 8))
/* Avoid warnings for Xcode 7. It seems it does no longer exist in Xcode 8 */
#pragma clang diagnostic ignored "-Wno-reserved-id-macro"
#pragma clang diagnostic ignored "-Wno-keyword-macro"
#endif
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC (1)
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME (2)
#endif

#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/errno.h>
#include <sys/time.h>

/* clock_gettime is not implemented on OSX prior to 10.12 */
static int
_civet_clock_gettime(int clk_id, struct timespec *t)
{
	memset(t, 0, sizeof(*t));
	if (clk_id == CLOCK_REALTIME) {
		struct timeval now;
		int rv = gettimeofday(&now, NULL);
		if (rv) {
			return rv;
		}
		t->tv_sec = now.tv_sec;
		t->tv_nsec = now.tv_usec * 1000;
		return 0;

	} else if (clk_id == CLOCK_MONOTONIC) {
		static uint64_t clock_start_time = 0;
		static mach_timebase_info_data_t timebase_ifo = {0, 0};

		uint64_t now = mach_absolute_time();

		if (clock_start_time == 0) {
			kern_return_t mach_status = mach_timebase_info(&timebase_ifo);
			DEBUG_ASSERT(mach_status == KERN_SUCCESS);

			/* appease "unused variable" warning for release builds */
			(void)mach_status;

			clock_start_time = now;
		}

		now = (uint64_t)((double)(now - clock_start_time)
		                 * (double)timebase_ifo.numer
		                 / (double)timebase_ifo.denom);

		t->tv_sec = now / 1000000000;
		t->tv_nsec = now % 1000000000;
		return 0;
	}
	return -1; /* EINVAL - Clock ID is unknown */
}

/* if clock_gettime is declared, then __CLOCK_AVAILABILITY will be defined */
#if defined(__CLOCK_AVAILABILITY)
/* If we compiled with Mac OSX 10.12 or later, then clock_gettime will be
 * declared but it may be NULL at runtime. So we need to check before using
 * it. */
static int
_civet_safe_clock_gettime(int clk_id, struct timespec *t)
{
	if (clock_gettime) {
		return clock_gettime(clk_id, t);
	}
	return _civet_clock_gettime(clk_id, t);
}
#define clock_gettime _civet_safe_clock_gettime
#else
#define clock_gettime _civet_clock_gettime
#endif

#endif


#if defined(_WIN32)
#define ERROR_TRY_AGAIN(err) ((err) == WSAEWOULDBLOCK)
#else
/* Unix might return different error codes indicating to try again.
 * For Linux EAGAIN==EWOULDBLOCK, maybe EAGAIN!=EWOULDBLOCK is history from
 * decades ago, but better check both and let the compiler optimize it. */
#define ERROR_TRY_AGAIN(err)                                                   \
	(((err) == EAGAIN) || ((err) == EWOULDBLOCK) || ((err) == EINTR))
#endif

#if defined(USE_ZLIB)
#include "zconf.h"
#include "zlib.h"
#endif

#if defined(USE_ZLIB_NG)
#include "zconf-ng.h"
#include "zlib-ng.h"
#endif

/********************************************************************/
/* CivetWeb configuration defines */
/********************************************************************/

/* Maximum number of threads that can be configured.
 * The number of threads actually created depends on the "num_threads"
 * configuration parameter, but this is the upper limit. */
#if !defined(MAX_WORKER_THREADS)
#define MAX_WORKER_THREADS (1024 * 64) /* in threads (count) */
#endif

/* Timeout interval for select/poll calls.
 * The timeouts depend on "*_timeout_ms" configuration values, but long
 * timeouts are split into timouts as small as SOCKET_TIMEOUT_QUANTUM.
 * This reduces the time required to stop the server. */
#if !defined(SOCKET_TIMEOUT_QUANTUM)
#define SOCKET_TIMEOUT_QUANTUM (2000) /* in ms */
#endif

/* Do not try to compress files smaller than this limit. */
#if !defined(MG_FILE_COMPRESSION_SIZE_LIMIT)
#define MG_FILE_COMPRESSION_SIZE_LIMIT (1024) /* in bytes */
#endif

#if !defined(PASSWORDS_FILE_NAME)
#define PASSWORDS_FILE_NAME ".htpasswd"
#endif

/* Initial buffer size for all CGI environment variables. In case there is
 * not enough space, another block is allocated. */
#if !defined(CGI_ENVIRONMENT_SIZE)
#define CGI_ENVIRONMENT_SIZE (4096) /* in bytes */
#endif

/* Maximum number of environment variables. */
#if !defined(MAX_CGI_ENVIR_VARS)
#define MAX_CGI_ENVIR_VARS (256) /* in variables (count) */
#endif

/* General purpose buffer size. */
#if !defined(MG_BUF_LEN) /* in bytes */
#define MG_BUF_LEN (1024 * 8)
#endif


/********************************************************************/

/* Helper macros */
#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#include <stdint.h>

/* Standard defines */
#if !defined(INT64_MAX)
#define INT64_MAX (9223372036854775807)
#endif

#define SHUTDOWN_RD (0)
#define SHUTDOWN_WR (1)
#define SHUTDOWN_BOTH (2)

mg_static_assert(MAX_WORKER_THREADS >= 1,
                 "worker threads must be a positive number");

mg_static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8,
                 "size_t data type size check");




#if defined(_MSC_VER)
#ifndef _WIN32_WCE // Some ANSI #includes are not available on Windows CE
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#endif // !_WIN32_WCE
#endif

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) /* WINDOWS include block */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // To make it link in VS200x (with IPv6 support from Win2K onwards, with improved support from Vista onwards)
#pragma message("Warning: _WIN32_WINNT is not set explicitly. Default to support Windows Vista and newer.")
#endif
// load winSock2 before windows.h or you won't be able to access to IPv6 goodness due to windows.h loading winsock.h (v1):
#include <ws2tcpip.h>
#include <winsock2.h> /* DTL add for SO_EXCLUSIVE */
#include <windows.h>
#include <mswsock.h>
#include <winsvc.h>

//
// _WIN32_WINNT version constants           <sdkddkver.h>
//
#ifndef _WIN32_WINNT_NT4
#define _WIN32_WINNT_NT4                    0x0400
#endif
#ifndef _WIN32_WINNT_NT4_SP3
#define _WIN32_WINNT_NT4_SP3                0x0403
#endif
#ifndef _WIN32_WINNT_WIN2K
#define _WIN32_WINNT_WIN2K                  0x0500
#endif
#ifndef _WIN32_WINNT_WINXP
#define _WIN32_WINNT_WINXP                  0x0501
#endif
#ifndef _WIN32_WINNT_WS03
#define _WIN32_WINNT_WS03                   0x0502
#endif
#ifndef _WIN32_WINNT_WIN6
#define _WIN32_WINNT_WIN6                   0x0600
#endif
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA                  0x0600
#endif
#ifndef _WIN32_WINNT_WS08
#define _WIN32_WINNT_WS08                   0x0600
#endif
#ifndef _WIN32_WINNT_LONGHORN
#define _WIN32_WINNT_LONGHORN               0x0600
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7                   0x0601
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif


typedef const char *SOCK_OPT_TYPE;

/* For a detailed description of these *_PATH_MAX defines, see
 * https://github.com/civetweb/civetweb/issues/937. */

/* UTF8_PATH_MAX is a char buffer size for 259 BMP characters in UTF-8 plus
 * null termination, rounded up to the next 4 bytes boundary */
#define UTF8_PATH_MAX (3 * 260)
/* UTF16_PATH_MAX is the 16-bit wchar_t buffer size required for 259 BMP
 * characters plus termination. (Note: wchar_t is 16 bit on Windows) */
#define UTF16_PATH_MAX (260)

#if !defined(_IN_PORT_T)
#if !defined(in_port_t)
#define in_port_t u_short
#endif
#endif

#if defined(_WIN32_WCE)
#error "WinCE support has ended"
#endif

#include <direct.h>
#include <io.h>
#include <process.h>


#define MAKEUQUAD(lo, hi)                                                      \
	((uint64_t)(((uint32_t)(lo)) | ((uint64_t)((uint32_t)(hi))) << 32))
#define RATE_DIFF (10000000) /* 100 nsecs */
#define EPOCH_DIFF (MAKEUQUAD(0xd53e8000, 0x019db1de))
#define SYS2UNIX_TIME(lo, hi)                                                  \
	((time_t)((MAKEUQUAD((lo), (hi)) - EPOCH_DIFF) / RATE_DIFF))

/* Visual Studio 6 does not know __func__ or __FUNCTION__
 * The rest of MS compilers use __FUNCTION__, not C99 __func__
 * Also use _strtoui64 on modern M$ compilers */
#if defined(_MSC_VER)
#if (_MSC_VER < 1300)
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ __FILE__ ":" STR(__LINE__)
#define strtoull(x, y, z) ((unsigned __int64)_atoi64(x))
#define strtoll(x, y, z) (_atoi64(x))
#else
#define __func__ __FUNCTION__
#define strtoull(x, y, z) (_strtoui64(x, y, z))
#define strtoll(x, y, z) (_strtoi64(x, y, z))
#endif
#endif /* _MSC_VER */


#define ERRNO   mgW32_get_errno()

#if !(defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(_WINSOCK2API_)) // Microsoft: socklen_t exists in ws2tcpip.h in Windows SDK 7.0A+
#define NO_SOCKLEN_T
#endif


#if defined(_WIN64) || defined(__MINGW64__)
#if !defined(SSL_LIB)

#if defined(OPENSSL_API_3_0)
#define SSL_LIB "libssl-3-x64.dll"
#define CRYPTO_LIB "libcrypto-3-x64.dll"
#endif

#if defined(OPENSSL_API_1_1)
#define SSL_LIB "libssl-1_1-x64.dll"
#define CRYPTO_LIB "libcrypto-1_1-x64.dll"
#endif /* OPENSSL_API_1_1 */

#if defined(OPENSSL_API_1_0)
#define SSL_LIB "ssleay64.dll"
#define CRYPTO_LIB "libeay64.dll"
#endif /* OPENSSL_API_1_0 */

#endif
#else /* defined(_WIN64) || defined(__MINGW64__) */
#if !defined(SSL_LIB)

#if defined(OPENSSL_API_3_0)
#define SSL_LIB "libssl-3.dll"
#define CRYPTO_LIB "libcrypto-3.dll"
#endif

#if defined(OPENSSL_API_1_1)
#define SSL_LIB "libssl-1_1.dll"
#define CRYPTO_LIB "libcrypto-1_1.dll"
#endif /* OPENSSL_API_1_1 */

#if defined(OPENSSL_API_1_0)
#define SSL_LIB "ssleay32.dll"
#define CRYPTO_LIB "libeay32.dll"
#endif /* OPENSSL_API_1_0 */

#endif /* SSL_LIB */
#endif /* defined(_WIN64) || defined(__MINGW64__) */


#define DIRSEP '\\'
#define IS_DIRSEP_CHAR(c) ((c) == '/' || (c) == '\\')

#ifndef PATH_MAX // MingW fix
#define PATH_MAX MAX_PATH
#endif

#ifndef S_ISDIR // MingW fix
#define S_ISDIR(x) ((x) & _S_IFDIR)
#endif

#define O_NONBLOCK (0)

#if !defined(W_OK)
#define W_OK (2) /* http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx */
#endif

#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK  WSAEWOULDBLOCK
#endif // !EWOULDBLOCK

#ifndef ETIMEOUT
#define ETIMEOUT    WSAETIMEDOUT
#endif













#define _POSIX_

// make sure code which hasn't been properly migrated, crashes at compile time:
#define INT64_FMT   ++++"do not use; use the C99 format define(s) instead, e.g. PRId64"++++
#define UINT64_FMT  ++++"do not use; use the C99 format define(s) instead, e.g. PRIu64"++++

/*
Since MSVC and MingW GCC don't have inttypes.h, we provide those printf/scanf formats here:

GCC inttypes.h says:
   "The ISO C99 standard specifies that these macros must only be
   defined if explicitly requested."

We also check whether someone else has gone before us setting up these C99 defines.
*/
#if (!defined __cplusplus || defined __STDC_FORMAT_MACROS) && !defined(PRId64)

#define __PRI64_PREFIX  "I64"

/* Macros for printing format specifiers.  */

/* Decimal notation.  */
#define PRId8       "d"
#define PRId16      "d"
#define PRId32      "d"
#define PRId64      __PRI64_PREFIX "d"

#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 __PRI64_PREFIX "d"

#define PRIdFAST8   "d"
#define PRIdFAST16  "d"
#define PRIdFAST32  "d"
#define PRIdFAST64  __PRI64_PREFIX "d"


#define PRIi8       "i"
#define PRIi16      "i"
#define PRIi32      "i"
#define PRIi64      __PRI64_PREFIX "i"

#define PRIiLEAST8  "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 __PRI64_PREFIX "i"

#define PRIiFAST8   "i"
#define PRIiFAST16  "i"
#define PRIiFAST32  "i"
#define PRIiFAST64  __PRI64_PREFIX "i"

/* Octal notation.  */
#define PRIo8       "o"
#define PRIo16      "o"
#define PRIo32      "o"
#define PRIo64      __PRI64_PREFIX "o"

#define PRIoLEAST8  "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 __PRI64_PREFIX "o"

#define PRIoFAST8   "o"
#define PRIoFAST16  "o"
#define PRIoFAST32  "o"
#define PRIoFAST64  __PRI64_PREFIX "o"

/* Unsigned integers.  */
#define PRIu8       "u"
#define PRIu16      "u"
#define PRIu32      "u"
#define PRIu64      __PRI64_PREFIX "u"

#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 __PRI64_PREFIX "u"

#define PRIuFAST8   "u"
#define PRIuFAST16  "u"
#define PRIuFAST32  "u"
#define PRIuFAST64  __PRI64_PREFIX "u"

/* lowercase hexadecimal notation.  */
#define PRIx8       "x"
#define PRIx16      "x"
#define PRIx32      "x"
#define PRIx64      __PRI64_PREFIX "x"

#define PRIxLEAST8  "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 __PRI64_PREFIX "x"

#define PRIxFAST8   "x"
#define PRIxFAST16  "x"
#define PRIxFAST32  "x"
#define PRIxFAST64  __PRI64_PREFIX "x"

/* UPPERCASE hexadecimal notation.  */
#define PRIX8       "X"
#define PRIX16      "X"
#define PRIX32      "X"
#define PRIX64      __PRI64_PREFIX "X"

#define PRIXLEAST8  "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 __PRI64_PREFIX "X"

#define PRIXFAST8   "X"
#define PRIXFAST16  "X"
#define PRIXFAST32  "X"
#define PRIXFAST64  __PRI64_PREFIX "X"


/* Macros for printing `intmax_t' and `uintmax_t'.  */
#define PRIdMAX __PRI64_PREFIX "d"
#define PRIiMAX __PRI64_PREFIX "i"
#define PRIoMAX __PRI64_PREFIX "o"
#define PRIuMAX __PRI64_PREFIX "u"
#define PRIxMAX __PRI64_PREFIX "x"
#define PRIXMAX __PRI64_PREFIX "X"

/* Macros for printing `intptr_t' and `uintptr_t'.  */
#define PRIdPTR __PRI64_PREFIX "d"
#define PRIiPTR __PRI64_PREFIX "i"
#define PRIoPTR __PRI64_PREFIX "o"
#define PRIuPTR __PRI64_PREFIX "u"
#define PRIxPTR __PRI64_PREFIX "x"
#define PRIXPTR __PRI64_PREFIX "X"


/* Macros for scanning format specifiers.  */

/* Signed decimal notation.  */
//#define SCNd8     "hhd"
#define SCNd16      "hd"
#define SCNd32      "d"
#define SCNd64      __PRI64_PREFIX "d"

//#define SCNdLEAST8    "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 __PRI64_PREFIX "d"

//#define SCNdFAST8 "hhd"
#define SCNdFAST16  "d"
#define SCNdFAST32  "d"
#define SCNdFAST64  __PRI64_PREFIX "d"

/* Signed decimal notation.  */
//#define SCNi8     "hhi"
#define SCNi16      "hi"
#define SCNi32      "i"
#define SCNi64      __PRI64_PREFIX "i"

//#define SCNiLEAST8    "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 __PRI64_PREFIX "i"

//#define SCNiFAST8 "hhi"
#define SCNiFAST16  "i"
#define SCNiFAST32  "i"
#define SCNiFAST64  __PRI64_PREFIX "i"

/* Unsigned decimal notation.  */
//#define SCNu8     "hhu"
#define SCNu16      "hu"
#define SCNu32      "u"
#define SCNu64      __PRI64_PREFIX "u"

//#define SCNuLEAST8    "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 __PRI64_PREFIX "u"

//#define SCNuFAST8 "hhu"
#define SCNuFAST16  "u"
#define SCNuFAST32  "u"
#define SCNuFAST64  __PRI64_PREFIX "u"

/* Octal notation.  */
//#define SCNo8     "hho"
#define SCNo16      "ho"
#define SCNo32      "o"
#define SCNo64      __PRI64_PREFIX "o"

//#define SCNoLEAST8    "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 __PRI64_PREFIX "o"

//#define SCNoFAST8 "hho"
#define SCNoFAST16  "o"
#define SCNoFAST32  "o"
#define SCNoFAST64  __PRI64_PREFIX "o"

/* Hexadecimal notation.  */
//#define SCNx8     "hhx"
#define SCNx16      "hx"
#define SCNx32      "x"
#define SCNx64      __PRI64_PREFIX "x"

//#define SCNxLEAST8    "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 __PRI64_PREFIX "x"

//#define SCNxFAST8 "hhx"
#define SCNxFAST16  "x"
#define SCNxFAST32  "x"
#define SCNxFAST64  __PRI64_PREFIX "x"


/* Macros for scanning `intmax_t' and `uintmax_t'.  */
#define SCNdMAX __PRI64_PREFIX "d"
#define SCNiMAX __PRI64_PREFIX "i"
#define SCNoMAX __PRI64_PREFIX "o"
#define SCNuMAX __PRI64_PREFIX "u"
#define SCNxMAX __PRI64_PREFIX "x"

/* Macros for scanning `intptr_t' and `uintptr_t'.  */
#define SCNdPTR __PRI64_PREFIX "d"
#define SCNiPTR __PRI64_PREFIX "i"
#define SCNoPTR __PRI64_PREFIX "o"
#define SCNuPTR __PRI64_PREFIX "u"
#define SCNxPTR __PRI64_PREFIX "x"

#endif  /* C++ && format macros */

// Below are the macros for enabling compiler-specific checks for
// printf-like arguments.
//
// See also: http://stackoverflow.com/questions/2354784/attribute-formatprintf-1-2-for-msvc/6849629#6849629
#undef PRINTF_FORMAT_STRING
#if _MSC_VER >= 1400
#include <sal.h>
#if _MSC_VER > 1400
#define PRINTF_FORMAT_STRING(s) _Printf_format_string_ s
#else
#define PRINTF_FORMAT_STRING(s) __format_string s
#endif
#else
#define PRINTF_FORMAT_STRING(s) s
#endif

#ifdef __GNUC__
#define PRINTF_ARGS(x, y) __attribute__((format(printf, x, y)))
#else
#define PRINTF_ARGS(x, y)
#endif
// -------------

#define WINCDECL __cdecl

#ifndef SHUT_WR
#define SHUT_WR   SD_SEND
#define SHUT_RD   SD_RECEIVE
#define SHUT_RDWR SD_BOTH
#endif

#if !defined(IPV6_V6ONLY) && (_WIN32_WINNT >= 0x0501)
#define IPV6_V6ONLY           27 // Treat wildcard bind as AF_INET6-only.
#endif

#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define vsnprintf_impl _vsnprintf

#define access _access

#define mg_sleep(x) (Sleep(x))

#define pipe(x) _pipe(x, MG_BUF_LEN, _O_BINARY | _O_NOINHERIT)
#if !defined(popen)
#define popen(x, y) (_popen(x, y))
#endif
#if !defined(pclose)
#define pclose(x) (_pclose(x))
#endif
#define close(x) (_close(x))
#define dlsym(x, y) (GetProcAddress((HINSTANCE)(x), (y)))
#define RTLD_LAZY (0)
#if !defined(_POSIX_)
#define fseeko(x, y, z) ((_lseeki64(_fileno(x), (y), (z)) == -1) ? -1 : 0)
#define fdopen(x, y) (_fdopen((x), (y)))
#else
#define fseeko(x, y, z) ((_lseeki64(fileno(x), (y), (z)) == -1) ? -1 : 0)
#endif
#define write(x, y, z) (_write((x), (y), (unsigned)z))
#define read(x, y, z) (_read((x), (y), (unsigned)z))
#define flockfile(x) ((void)pthread_mutex_lock(&global_log_file_lock))
#define funlockfile(x) ((void)pthread_mutex_unlock(&global_log_file_lock))
#define sleep(x) (Sleep((x)*1000))
#define rmdir(x) (_rmdir(x))
#if defined(_WIN64) || !defined(__MINGW32__)
/* Only MinGW 32 bit is missing this function */
#define timegm(x) (_mkgmtime(x))
#else
time_t timegm(struct tm *tm);
#define NEED_TIMEGM
#endif


#if !defined(fileno)
#define fileno(x) (_fileno(x))
#endif /* !fileno MINGW #defines fileno */

// prevent collisions / odd replacements outside civetweb.c + civetweb_ex.c:
#if defined(INSIDE_CIVETWEB_C)
#define close _close
#define write _write
#define read  _read
#endif

#if !defined(HAVE_PTHREAD)
typedef struct {
	CRITICAL_SECTION sec; /* Immovable */
} pthread_mutex_t;
typedef DWORD pthread_key_t;
typedef HANDLE pthread_t;
typedef struct {
	pthread_mutex_t threadIdSec;
	struct mg_workerTLS *waiting_thread; /* The chain of threads */
} pthread_cond_t;
#endif

#if !defined(__clockid_t_defined)
typedef DWORD clockid_t;
#endif
#if !defined(CLOCK_MONOTONIC)
#define CLOCK_MONOTONIC (1)
#endif
#if !defined(CLOCK_REALTIME)
#define CLOCK_REALTIME (2)
#endif
#if !defined(CLOCK_THREAD)
#define CLOCK_THREAD (3)
#endif
#if !defined(CLOCK_PROCESS)
#define CLOCK_PROCESS (4)
#endif


#if (NTDDI_VERSION >= NTDDI_VISTA)
  // Only Windoze Vista (and newer) have inet_ntop(); MingW doesn't seem to provide it though
  #if !defined(__MINGW32__) && !defined(__MINGW64__) // http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers#MinGW_and_MinGW-w64
    #define HAVE_INET_NTOP
  #endif
#endif
#if defined(_WIN32_WINNT) && defined(_WIN32_WINNT_WINXP) && (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
  #define HAVE_GETNAMEINFO
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int mgW32_get_errno(void);

void mgW32_flockfile(FILE *x);
void mgW32_funlockfile(FILE *x);

#ifdef __cplusplus
}
#endif // __cplusplus

#define flockfile mgW32_flockfile
#define funlockfile mgW32_funlockfile

#if !defined(fileno) && (!defined(_POSIX_) || defined(_fileno))
#define fileno(x) _fileno(x)
#endif // !fileno MINGW #defines fileno; so does MSVC when in _POSIX_ mode, but _fileno is a faster(?) macro there.

#if !defined(HAVE_PTHREAD)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef HANDLE pthread_mutex_t;
typedef struct {HANDLE signal, broadcast;} pthread_cond_t;
typedef DWORD pthread_t;
typedef struct {WORD dummy;} pthread_attr_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#else

#include <pthread.h>

#endif


#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#define _TIMESPEC_DEFINED
#endif
#if !defined(_TIMESPEC_DEFINED)
struct timespec {
	time_t tv_sec; /* seconds */
	long tv_nsec;  /* nanoseconds */
};
#endif

#if !defined(WIN_PTHREADS_TIME_H)
#define MUST_IMPLEMENT_CLOCK_GETTIME
#endif

#if defined(MUST_IMPLEMENT_CLOCK_GETTIME)
#define clock_gettime mg_clock_gettime
static int
clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	FILETIME ft;
	ULARGE_INTEGER li, li2;
	BOOL ok = FALSE;
	double d;
	static double perfcnt_per_sec = 0.0;
	static BOOL initialized = FALSE;

	if (!initialized) {
		QueryPerformanceFrequency((LARGE_INTEGER *)&li);
		perfcnt_per_sec = 1.0 / li.QuadPart;
		initialized = TRUE;
	}

	if (tp) {
		memset(tp, 0, sizeof(*tp));

		if (clk_id == CLOCK_REALTIME) {

			/* BEGIN: CLOCK_REALTIME = wall clock (date and time) */
			GetSystemTimeAsFileTime(&ft);
			li.LowPart = ft.dwLowDateTime;
			li.HighPart = ft.dwHighDateTime;
			li.QuadPart -= 116444736000000000; /* 1.1.1970 in filedate */
			tp->tv_sec = (time_t)(li.QuadPart / 10000000);
			tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
			ok = TRUE;
			/* END: CLOCK_REALTIME */

		} else if (clk_id == CLOCK_MONOTONIC) {

			/* BEGIN: CLOCK_MONOTONIC = stopwatch (time differences) */
			QueryPerformanceCounter((LARGE_INTEGER *)&li);
			d = li.QuadPart * perfcnt_per_sec;
			tp->tv_sec = (time_t)d;
			d -= (double)tp->tv_sec;
			tp->tv_nsec = (long)(d * 1.0E9);
			ok = TRUE;
			/* END: CLOCK_MONOTONIC */

		} else if (clk_id == CLOCK_THREAD) {

			/* BEGIN: CLOCK_THREAD = CPU usage of thread */
			FILETIME t_create, t_exit, t_kernel, t_user;
			if (GetThreadTimes(GetCurrentThread(),
			                   &t_create,
			                   &t_exit,
			                   &t_kernel,
			                   &t_user)) {
				li.LowPart = t_user.dwLowDateTime;
				li.HighPart = t_user.dwHighDateTime;
				li2.LowPart = t_kernel.dwLowDateTime;
				li2.HighPart = t_kernel.dwHighDateTime;
				li.QuadPart += li2.QuadPart;
				tp->tv_sec = (time_t)(li.QuadPart / 10000000);
				tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
				ok = TRUE;
			}
			/* END: CLOCK_THREAD */

		} else if (clk_id == CLOCK_PROCESS) {

			/* BEGIN: CLOCK_PROCESS = CPU usage of process */
			FILETIME t_create, t_exit, t_kernel, t_user;
			if (GetProcessTimes(GetCurrentProcess(),
			                    &t_create,
			                    &t_exit,
			                    &t_kernel,
			                    &t_user)) {
				li.LowPart = t_user.dwLowDateTime;
				li.HighPart = t_user.dwHighDateTime;
				li2.LowPart = t_kernel.dwLowDateTime;
				li2.HighPart = t_kernel.dwHighDateTime;
				li.QuadPart += li2.QuadPart;
				tp->tv_sec = (time_t)(li.QuadPart / 10000000);
				tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
				ok = TRUE;
			}
			/* END: CLOCK_PROCESS */

		} else {

			/* BEGIN: unknown clock */
			/* ok = FALSE; already set by init */
			/* END: unknown clock */
		}
	}

	return ok ? 0 : -1;
}
#endif


#define pid_t HANDLE // MINGW typedefs pid_t to int. Using #define here.  It also overrides the pid_t typedef in pthread.h (--> sched.h) for pthread-win32

static int pthread_mutex_lock(pthread_mutex_t *);
static int pthread_mutex_unlock(pthread_mutex_t *);
static void path_to_unicode(const struct mg_connection *conn,
                            const char *path,
                            wchar_t *wbuf,
                            size_t wbuf_len);

/* All file operations need to be rewritten to solve #246. */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct mg_file;

static const char *mg_fgets(char *buf, size_t size, struct mg_file *filep);


/* POSIX dirent interface */
struct dirent {
	char d_name[UTF8_PATH_MAX];
};

typedef struct DIR {
	HANDLE handle;
	WIN32_FIND_DATAW info;
	struct dirent result;
} DIR;

#if defined(HAVE_POLL)
#define mg_pollfd pollfd
#else
struct mg_pollfd {
	SOCKET fd;
	short events;
	short revents;
};
#endif

int mg_rename(const char* oldname, const char* newname);
int mg_remove(const char *path);
int mg_mkdir(const char *path, int mode);

#if defined(__MINGW32__) || defined(__MINGW64__)
// fixing the 'implicit declaration' warnings as the MingW headers aren't up to snuff:
#if defined(_DLL) && !defined(_CRTIMP)
#define _CRTIMP __declspec(dllimport)
#endif
_CRTIMP int __cdecl fileno(FILE * _File);
_CRTIMP FILE * __cdecl fdopen(int _FileHandle, const char * _Mode);
_CRTIMP int __cdecl _pclose(FILE * _File);
_CRTIMP FILE * __cdecl _popen(const char * _Command, const char * _Mode);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

/* Mark required libraries */
#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#endif

#else /* defined(_WIN32) - WINDOWS vs UNIX include block */

#include <inttypes.h>

/* Linux & co. internally use UTF8 */
#define UTF8_PATH_MAX (PATH_MAX)

typedef const void *SOCK_OPT_TYPE;

#if defined(ANDROID)
typedef unsigned short int in_port_t;
#endif

#if !defined(__ZEPHYR__)
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#if !defined(__rtems__)
#include <sys/utsname.h>
#endif
#include <sys/wait.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#if defined(USE_X_DOM_SOCKET)
#include <sys/un.h>
#endif
#endif

#define vsnprintf_impl vsnprintf

#if !defined(NO_SSL_DL) && !defined(NO_SSL)
#include <dlfcn.h>
#endif

#if defined(__MACH__) && defined(__APPLE__)

#if defined(OPENSSL_API_3_0)
#define SSL_LIB "libssl.3.dylib"
#define CRYPTO_LIB "libcrypto.3.dylib"
#endif

#if defined(OPENSSL_API_1_1)
#define SSL_LIB "libssl.1.1.dylib"
#define CRYPTO_LIB "libcrypto.1.1.dylib"
#endif /* OPENSSL_API_1_1 */

#if defined(OPENSSL_API_1_0)
#define SSL_LIB "libssl.1.0.dylib"
#define CRYPTO_LIB "libcrypto.1.0.dylib"
#endif /* OPENSSL_API_1_0 */

#else
#if !defined(SSL_LIB)
#define SSL_LIB   "libssl.so"
#endif
#if !defined(CRYPTO_LIB)
#define CRYPTO_LIB  "libcrypto.so"
#endif
#endif
#ifndef DIRSEP
#define DIRSEP   '/'
#endif
#ifndef IS_DIRSEP_CHAR
#define IS_DIRSEP_CHAR(c) ((c) == '/')
#endif
#if !defined(O_BINARY)
#define O_BINARY (0)
#endif /* O_BINARY */
#define closesocket(a) (close(a))
//#define mg_fopen(x, y) fopen(x, y)
#define mg_mkdir(conn, path, mode) (mkdir(path, mode))
#define mg_remove(conn, x) (remove(x))
#define mg_sleep(x) (usleep((x)*1000))
#define mg_opendir(conn, x) (opendir(x))
#define mg_closedir(x) (closedir(x))
#define mg_readdir(x) (readdir(x))
#define mg_rename(x, y) rename(x, y)
#define ERRNO (errno)
#define INVALID_SOCKET (-1)

// make sure code which hasn't been properly migrated, crashes at compile time:
#define INT64_FMT   ++++"do not use; use the C99 format define(s) instead, e.g. PRId64"++++
#define UINT64_FMT  ++++"do not use; use the C99 format define(s) instead, e.g. PRIu64"++++

typedef int SOCKET;
#define WINCDECL

#if defined(__hpux)
/* HPUX 11 does not have monotonic, fall back to realtime */
#if !defined(CLOCK_MONOTONIC)
#define CLOCK_MONOTONIC CLOCK_REALTIME
#endif

/* HPUX defines socklen_t incorrectly as size_t which is 64bit on
 * Itanium.  Without defining _XOPEN_SOURCE or _XOPEN_SOURCE_EXTENDED
 * the prototypes use int* rather than socklen_t* which matches the
 * actual library expectation.  When called with the wrong size arg
 * accept() returns a zero client inet addr and check_acl() always
 * fails.  Since socklen_t is widely used below, just force replace
 * their typedef with int. - DTL
 */
#define socklen_t int
#endif /* hpux */

#define mg_pollfd pollfd

// for now assume all UNIXes have inet_ntop / inet_pton when they have IPv6, otherwise they always have getnameinfo()
#if defined(USE_IPV6)
  #define HAVE_INET_NTOP
#endif
#define HAVE_GETNAMEINFO

#endif /* defined(_WIN32) - WINDOWS vs UNIX include block */

#ifndef FORMAT_STRING
# define FORMAT_STRING(p) p
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#if !defined(MG_BUF_LEN)
#define MG_BUF_LEN        8192
#endif
#define MG_MAX(a, b)      ((a) >= (b) ? (a) : (b))
#define MG_MIN(a, b)      ((a) <= (b) ? (a) : (b))

/*
 * The following VA_COPY was coded following an example in
 * the Samba project.  It may not be sufficient for some
 * esoteric implementations of va_list (i.e. it may need
 * something involving a memcpy) but (hopefully) will be
 * sufficient for civetweb (code taken from libxml2 and augmented).
 */
#ifndef VA_COPY
  #if defined(HAVE_VA_COPY) || defined(va_copy) /* Linux stdarg.h 'regular' flavor */
    #define VA_COPY(dest, src) va_copy(dest, src)
  #else
    #if defined(HAVE___VA_COPY) || defined(__va_copy)  /* Linux stdarg.h 'strict ANSI' flavor */
      #define VA_COPY(dest,src) __va_copy(dest, src)
    #else
      #define VA_COPY(dest,src) do { (dest) = (src); } while (0) /* MSVC: doesn't offer va_copy at all. */
    #endif
  #endif
#endif


/* <bel>: Local fix for some linux SDK headers that do not know these options */
#ifndef SOMAXCONN
#define SOMAXCONN 128
#endif
/* <bel>: end fix */

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif


#if (defined(DEBUG) || defined(_DEBUG)) && !MG_DEBUG_TRACING
#if defined(PTW32_VERSION)
#define MG_PTHREAD_SELF()   pthread_self().p
#else
#define MG_PTHREAD_SELF()   (void *)pthread_self()
#endif
#define MG_DEBUG_TRACING                                    1
/*
  mg_trace_level bits:

      0x0001:          general socket I/O
      0x0002:          keep alive (HTTP) specifics (queueing and headers)
      0x0004:          chunked transfer details
      0x0008:          tail chunk specifics
      0x0010:          read/write socket details
      0x0020:          close socket specifics (graceful / UNgraceful close)
      0x0100:          tasks / threads
      0x0200:          URL and other HTTP encoding / decoding
      0x0400:          file system / web server ops (PUT, DELETE, ...)
      0x0800:          web server request handling
      0x1000:          civetweb config options
  0x00010000..onwards: application specific
*/
unsigned int *mg_trace_level(void);
#define DEBUG_TRACE(l, x)                                   \
do {                                                        \
  if (*mg_trace_level() & l) {                              \
    flockfile(stdout);                                      \
    printf("*** %lu.%p.%s.%d: ",                            \
           (unsigned long) time(NULL), MG_PTHREAD_SELF(),   \
           __func__, __LINE__);                             \
    printf x;                                               \
    putchar('\n');                                          \
    fflush(stdout);                                         \
    funlockfile(stdout);                                    \
  }                                                         \
} while (0)
#else
#define MG_DEBUG_TRACING                                    0
#undef DEBUG_TRACE
#define DEBUG_TRACE(l, x)
#endif // DEBUG

// Darwin prior to 7.0 and Win32 do not have socklen_t
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif // NO_SOCKLEN_T
#define _DARWIN_UNLIMITED_SELECT

/* buffer size that will fit both IPv4 and IPv6 addresses formatted by ntoa() / ntop() */
#define SOCKADDR_NTOA_BUFSIZE           42




typedef void * (WINCDECL *mg_thread_func_t)(void *);

#if defined(_WIN32)

#if !defined(HAVE_PTHREAD)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * POSIX pthread features support:
 */
#undef _POSIX_THREADS

#undef _POSIX_READER_WRITER_LOCKS
#define _POSIX_READER_WRITER_LOCKS  200809L

#undef _POSIX_SPIN_LOCKS
#define _POSIX_SPIN_LOCKS           200809L

#undef _POSIX_BARRIERS

#undef _POSIX_THREAD_SAFE_FUNCTIONS

#undef _POSIX_THREAD_ATTR_STACKSIZE

#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t)INVALID_HANDLE_VALUE)

int pthread_mutex_init(pthread_mutex_t *mutex, void *unused);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_cond_init(pthread_cond_t *cv, const void *unused);
int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cv);
int pthread_cond_broadcast(pthread_cond_t *cv);
int pthread_cond_destroy(pthread_cond_t *cv);
pthread_t pthread_self(void);
int pthread_create(pthread_t * tid, const pthread_attr_t * attr, mg_thread_func_t start, void *arg);
void pthread_exit(void *value_ptr);

#if !defined(USE_SRWLOCK)
#if defined(RTL_SRWLOCK_INIT) && (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
#define USE_SRWLOCK      1
#else
#define USE_SRWLOCK      0
#endif
#endif

typedef struct {
    unsigned rw: 1;
#if USE_SRWLOCK         // Windows 7 / Server 2008 with the correct header files, i.e. this also 'fixes' MingW casualties
    SRWLOCK lock;
#else
    pthread_mutex_t mutex;
#endif
} pthread_rwlock_t;

typedef void pthread_rwlockattr_t;

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);

#if USE_SRWLOCK         // Windows 7 / Server 2008 with the correct header files, i.e. this also 'fixes' MingW casualties
#define PTHREAD_RWLOCK_INITIALIZER          { 0, RTL_SRWLOCK_INIT }
#else
#define PTHREAD_RWLOCK_INITIALIZER          { 0 }
#endif

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

#define PTHREAD_SPINLOCK_INITIALIZER      PTHREAD_MUTEX_INITIALIZER

typedef pthread_mutex_t pthread_spinlock_t;

#define PTHREAD_PROCESS_PRIVATE         0

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
//int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif


#if defined(_WIN32_WCE)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

time_t time(time_t *ptime);
struct tm *localtime(const time_t *ptime, struct tm *ptm);
struct tm *gmtime(const time_t *ptime, struct tm *ptm);
static size_t strftime(char *dst, size_t dst_size, const char *fmt,
                       const struct tm *tm);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

#endif // _WIN32 -- for pthread and time lib support



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *mg_malloca(size_t size);
void mg_freea(void *ptr);

#ifdef __cplusplus
}
#endif // __cplusplus



#undef UNUSED_PARAMETER
#if defined(__GNUC__)
#define UNUSED_PARAMETER(p)     p __attribute__((unused))
#else
#define UNUSED_PARAMETER(p)     p
#endif




// Allow the use of a customizable ASSERT instead of the system-defined assert:
#ifndef MG_ASSERT
#ifdef NDEBUG
#define MG_ASSERT(expr)     ((void)0)
#else
#define MG_SIGNAL_ASSERT mg_signal_assert			// to help us detect that this particular assert implementation is desired
#define MG_ASSERT(expr)												\
	do {															\
		if (!(expr)) {												\
			(void)MG_SIGNAL_ASSERT(#expr, __FILE__, __LINE__);		\
		}															\
	} while (0)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	int mg_signal_assert(const char *expr, const char *filepath, unsigned int lineno);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif
#endif


#endif // CIVETWEB_SYS_PORTING_INCLUDE

/* ---------------------- end of sys-porting common chunk ----------------------------- */

#if 0


/* In case our C library is missing "timegm", provide an implementation */
#if defined(NEED_TIMEGM)
static inline int
is_leap(int y)
{
	return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static inline int
count_leap(int y)
{
	return (y - 1969) / 4 - (y - 1901) / 100 + (y - 1601) / 400;
}

static time_t
timegm(struct tm *tm)
{
	static const unsigned short ydays[] = {
	    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
	int year = tm->tm_year + 1900;
	int mon = tm->tm_mon;
	int mday = tm->tm_mday - 1;
	int hour = tm->tm_hour;
	int min = tm->tm_min;
	int sec = tm->tm_sec;

	if (year < 1970 || mon < 0 || mon > 11 || mday < 0
	    || (mday >= ydays[mon + 1] - ydays[mon]
	                    + (mon == 1 && is_leap(year) ? 1 : 0))
	    || hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 60)
		return -1;

	time_t res = year - 1970;
	res *= 365;
	res += mday;
	res += ydays[mon] + (mon > 1 && is_leap(year) ? 1 : 0);
	res += count_leap(year);

	res *= 24;
	res += hour;
	res *= 60;
	res += min;
	res *= 60;
	res += sec;
	return res;
}
#endif /* NEED_TIMEGM */


/* va_copy should always be a macro, C99 and C++11 - DTL */
#if !defined(va_copy)
#define va_copy(x, y) ((x) = (y))
#endif


#if defined(_WIN32)
/* Create substitutes for POSIX functions in Win32. */

#if defined(GCC_DIAGNOSTIC)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif


static pthread_mutex_t global_log_file_lock;

FUNCTION_MAY_BE_UNUSED
static DWORD
pthread_self(void)
{
	return GetCurrentThreadId();
}


FUNCTION_MAY_BE_UNUSED
static int
pthread_key_create(
    pthread_key_t *key,
    void (*_ignored)(void *) /* destructor not supported for Windows */
)
{
	(void)_ignored;

	if ((key != 0)) {
		*key = TlsAlloc();
		return (*key != TLS_OUT_OF_INDEXES) ? 0 : -1;
	}
	return -2;
}


FUNCTION_MAY_BE_UNUSED
static int
pthread_key_delete(pthread_key_t key)
{
	return TlsFree(key) ? 0 : 1;
}


FUNCTION_MAY_BE_UNUSED
static int
pthread_setspecific(pthread_key_t key, void *value)
{
	return TlsSetValue(key, value) ? 0 : 1;
}


FUNCTION_MAY_BE_UNUSED
static void *
pthread_getspecific(pthread_key_t key)
{
	return TlsGetValue(key);
}

#if defined(GCC_DIAGNOSTIC)
/* Enable unused function warning again */
#pragma GCC diagnostic pop
#endif

static struct pthread_mutex_undefined_struct *pthread_mutex_attr = NULL;
#else
static pthread_mutexattr_t pthread_mutex_attr;
#endif /* _WIN32 */


#if defined(GCC_DIAGNOSTIC)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /* defined(GCC_DIAGNOSTIC) */
#if defined(__clang__)
/* Show no warning in case system functions are not used. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

static pthread_mutex_t global_lock_mutex;


FUNCTION_MAY_BE_UNUSED
static void
mg_global_lock(void)
{
	(void)pthread_mutex_lock(&global_lock_mutex);
}


FUNCTION_MAY_BE_UNUSED
static void
mg_global_unlock(void)
{
	(void)pthread_mutex_unlock(&global_lock_mutex);
}


#if defined(_WIN64)
mg_static_assert(SIZE_MAX == 0xFFFFFFFFFFFFFFFFu, "Mismatch for atomic types");
#elif defined(_WIN32)
mg_static_assert(SIZE_MAX == 0xFFFFFFFFu, "Mismatch for atomic types");
#endif


/* Atomic functions working on ptrdiff_t ("signed size_t").
 * Operations: Increment, Decrement, Add, Maximum.
 * Up to size_t, they do not an atomic "load" operation.
 */
FUNCTION_MAY_BE_UNUSED
static ptrdiff_t
mg_atomic_inc(volatile ptrdiff_t *addr)
{
	ptrdiff_t ret;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	ret = InterlockedIncrement64(addr);
#elif defined(_WIN32) && !defined(NO_ATOMICS)
#ifdef __cplusplus
	/* For C++ the Microsoft Visual Studio compiler can not decide what
	 * overloaded function prototpye in the SDC corresponds to "ptrdiff_t". */
	static_assert(sizeof(ptrdiff_t) == sizeof(LONG), "Size mismatch");
	static_assert(sizeof(ptrdiff_t) == sizeof(int32_t), "Size mismatch");
	ret = InterlockedIncrement((LONG *)addr);
#else
	ret = InterlockedIncrement(addr);
#endif
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	ret = __sync_add_and_fetch(addr, 1);
#else
	mg_global_lock();
	ret = (++(*addr));
	mg_global_unlock();
#endif
	return ret;
}


FUNCTION_MAY_BE_UNUSED
static ptrdiff_t
mg_atomic_dec(volatile ptrdiff_t *addr)
{
	ptrdiff_t ret;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	ret = InterlockedDecrement64(addr);
#elif defined(_WIN32) && !defined(NO_ATOMICS)
#ifdef __cplusplus
	/* see mg_atomic_inc */
	static_assert(sizeof(ptrdiff_t) == sizeof(LONG), "Size mismatch");
	static_assert(sizeof(ptrdiff_t) == sizeof(int32_t), "Size mismatch");
	ret = InterlockedDecrement((LONG *)addr);
#else
	ret = InterlockedDecrement(addr);
#endif
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	ret = __sync_sub_and_fetch(addr, 1);
#else
	mg_global_lock();
	ret = (--(*addr));
	mg_global_unlock();
#endif
	return ret;
}


#if defined(USE_SERVER_STATS) || defined(STOP_FLAG_NEEDS_LOCK)
static ptrdiff_t
mg_atomic_add(volatile ptrdiff_t *addr, ptrdiff_t value)
{
	ptrdiff_t ret;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	ret = InterlockedAdd64(addr, value);
#elif defined(_WIN32) && !defined(NO_ATOMICS)
	ret = InterlockedExchangeAdd(addr, value) + value;
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	ret = __sync_add_and_fetch(addr, value);
#else
	mg_global_lock();
	*addr += value;
	ret = (*addr);
	mg_global_unlock();
#endif
	return ret;
}


FUNCTION_MAY_BE_UNUSED
static ptrdiff_t
mg_atomic_compare_and_swap(volatile ptrdiff_t *addr,
                           ptrdiff_t oldval,
                           ptrdiff_t newval)
{
	ptrdiff_t ret;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	ret = InterlockedCompareExchange64(addr, newval, oldval);
#elif defined(_WIN32) && !defined(NO_ATOMICS)
	ret = InterlockedCompareExchange(addr, newval, oldval);
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	ret = __sync_val_compare_and_swap(addr, oldval, newval);
#else
	mg_global_lock();
	ret = *addr;
	if ((ret != newval) && (ret == oldval)) {
		*addr = newval;
	}
	mg_global_unlock();
#endif
	return ret;
}


static void
mg_atomic_max(volatile ptrdiff_t *addr, ptrdiff_t value)
{
	register ptrdiff_t tmp = *addr;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	while (tmp < value) {
		tmp = InterlockedCompareExchange64(addr, value, tmp);
	}
#elif defined(_WIN32) && !defined(NO_ATOMICS)
	while (tmp < value) {
		tmp = InterlockedCompareExchange(addr, value, tmp);
	}
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	while (tmp < value) {
		tmp = __sync_val_compare_and_swap(addr, tmp, value);
	}
#else
	mg_global_lock();
	if (*addr < value) {
		*addr = value;
	}
	mg_global_unlock();
#endif
}


static int64_t
mg_atomic_add64(volatile int64_t *addr, int64_t value)
{
	int64_t ret;

#if defined(_WIN64) && !defined(NO_ATOMICS)
	ret = InterlockedAdd64(addr, value);
#elif defined(_WIN32) && !defined(NO_ATOMICS)
	ret = InterlockedExchangeAdd64(addr, value) + value;
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
	ret = __sync_add_and_fetch(addr, value);
#else
	mg_global_lock();
	*addr += value;
	ret = (*addr);
	mg_global_unlock();
#endif
	return ret;
}
#endif


#if defined(GCC_DIAGNOSTIC)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic pop
#endif /* defined(GCC_DIAGNOSTIC) */
#if defined(__clang__)
/* Show no warning in case system functions are not used. */
#pragma clang diagnostic pop
#endif


#if defined(USE_SERVER_STATS)

struct mg_memory_stat {
	volatile ptrdiff_t totalMemUsed;
	volatile ptrdiff_t maxMemUsed;
	volatile ptrdiff_t blockCount;
};


static struct mg_memory_stat *get_memory_stat(struct mg_context *ctx);


static void *
mg_malloc_ex(size_t size,
             struct mg_context *ctx,
             const char *file,
             unsigned line)
{
	void *data = malloc(size + 2 * sizeof(uintptr_t));
	void *memory = 0;
	struct mg_memory_stat *mstat = get_memory_stat(ctx);

#if defined(MEMORY_DEBUGGING)
	char mallocStr[256];
#else
	(void)file;
	(void)line;
#endif

	if (data) {
		uintptr_t *tmp = (uintptr_t *)data;
		ptrdiff_t mmem = mg_atomic_add(&mstat->totalMemUsed, (ptrdiff_t)size);
		mg_atomic_max(&mstat->maxMemUsed, mmem);
		mg_atomic_inc(&mstat->blockCount);
		tmp[0] = size;
		tmp[1] = (uintptr_t)mstat;
		memory = (void *)&tmp[2];
	}

#if defined(MEMORY_DEBUGGING)
	sprintf(mallocStr,
	        "MEM: %p %5lu alloc   %7lu %4lu --- %s:%u\n",
	        memory,
	        (unsigned long)size,
	        (unsigned long)mstat->totalMemUsed,
	        (unsigned long)mstat->blockCount,
	        file,
	        line);
	DEBUG_TRACE("%s", mallocStr);
#endif

	return memory;
}


static void *
mg_calloc_ex(size_t count,
             size_t size,
             struct mg_context *ctx,
             const char *file,
             unsigned line)
{
	void *data = mg_malloc_ex(size * count, ctx, file, line);

	if (data) {
		memset(data, 0, size * count);
	}
	return data;
}


static void
mg_free_ex(void *memory, const char *file, unsigned line)
{
#if defined(MEMORY_DEBUGGING)
	char mallocStr[256];
#else
	(void)file;
	(void)line;
#endif

	if (memory) {
		void *data = (void *)(((char *)memory) - 2 * sizeof(uintptr_t));
		uintptr_t size = ((uintptr_t *)data)[0];
		struct mg_memory_stat *mstat =
		    (struct mg_memory_stat *)(((uintptr_t *)data)[1]);
		mg_atomic_add(&mstat->totalMemUsed, -(ptrdiff_t)size);
		mg_atomic_dec(&mstat->blockCount);

#if defined(MEMORY_DEBUGGING)
		sprintf(mallocStr,
		        "MEM: %p %5lu free    %7lu %4lu --- %s:%u\n",
		        memory,
		        (unsigned long)size,
		        (unsigned long)mstat->totalMemUsed,
		        (unsigned long)mstat->blockCount,
		        file,
		        line);
		DEBUG_TRACE("%s", mallocStr);
#endif
		free(data);
	}
}


static void *
mg_realloc_ex(void *memory,
              size_t newsize,
              struct mg_context *ctx,
              const char *file,
              unsigned line)
{
	void *data;
	void *_realloc;
	uintptr_t oldsize;

#if defined(MEMORY_DEBUGGING)
	char mallocStr[256];
#else
	(void)file;
	(void)line;
#endif

	if (newsize) {
		if (memory) {
			/* Reallocate existing block */
			struct mg_memory_stat *mstat;
			data = (void *)(((char *)memory) - 2 * sizeof(uintptr_t));
			oldsize = ((uintptr_t *)data)[0];
			mstat = (struct mg_memory_stat *)((uintptr_t *)data)[1];
			_realloc = realloc(data, newsize + 2 * sizeof(uintptr_t));
			if (_realloc) {
				data = _realloc;
				mg_atomic_add(&mstat->totalMemUsed, -(ptrdiff_t)oldsize);
#if defined(MEMORY_DEBUGGING)
				sprintf(mallocStr,
				        "MEM: %p %5lu r-free  %7lu %4lu --- %s:%u\n",
				        memory,
				        (unsigned long)oldsize,
				        (unsigned long)mstat->totalMemUsed,
				        (unsigned long)mstat->blockCount,
				        file,
				        line);
				DEBUG_TRACE("%s", mallocStr);
#endif
				mg_atomic_add(&mstat->totalMemUsed, (ptrdiff_t)newsize);

#if defined(MEMORY_DEBUGGING)
				sprintf(mallocStr,
				        "MEM: %p %5lu r-alloc %7lu %4lu --- %s:%u\n",
				        memory,
				        (unsigned long)newsize,
				        (unsigned long)mstat->totalMemUsed,
				        (unsigned long)mstat->blockCount,
				        file,
				        line);
				DEBUG_TRACE("%s", mallocStr);
#endif
				*(uintptr_t *)data = newsize;
				data = (void *)(((char *)data) + 2 * sizeof(uintptr_t));
			} else {
#if defined(MEMORY_DEBUGGING)
				DEBUG_TRACE("%s", "MEM: realloc failed\n");
#endif
				return _realloc;
			}
		} else {
			/* Allocate new block */
			data = mg_malloc_ex(newsize, ctx, file, line);
		}
	} else {
		/* Free existing block */
		data = 0;
		mg_free_ex(memory, file, line);
	}

	return data;
}


#define mg_malloc(a) mg_malloc_ex(a, NULL, __FILE__, __LINE__)
#define mg_calloc(a, b) mg_calloc_ex(a, b, NULL, __FILE__, __LINE__)
#define mg_realloc(a, b) mg_realloc_ex(a, b, NULL, __FILE__, __LINE__)
#define mg_free(a) mg_free_ex(a, __FILE__, __LINE__)

#define mg_malloc_ctx(a, c) mg_malloc_ex(a, c, __FILE__, __LINE__)
#define mg_calloc_ctx(a, b, c) mg_calloc_ex(a, b, c, __FILE__, __LINE__)
#define mg_realloc_ctx(a, b, c) mg_realloc_ex(a, b, c, __FILE__, __LINE__)


#else /* USE_SERVER_STATS */


static __inline void *
mg_malloc(size_t a)
{
	return malloc(a);
}

static __inline void *
mg_calloc(size_t a, size_t b)
{
	return calloc(a, b);
}

static __inline void *
mg_realloc(void *a, size_t b)
{
	return realloc(a, b);
}

static __inline void
mg_free(void *a)
{
	free(a);
}

#define mg_malloc_ctx(a, c) mg_malloc(a)
#define mg_calloc_ctx(a, b, c) mg_calloc(a, b)
#define mg_realloc_ctx(a, b, c) mg_realloc(a, b)
#define mg_free_ctx(a, c) mg_free(a)

#endif /* USE_SERVER_STATS */


static void mg_vsnprintf(const struct mg_connection *conn,
                         int *truncated,
                         char *buf,
                         size_t buflen,
                         const char *fmt,
                         va_list ap);

static void mg_snprintf(const struct mg_connection *conn,
                        int *truncated,
                        char *buf,
                        size_t buflen,
                        PRINTF_FORMAT_STRING(const char *fmt),
                        ...) PRINTF_ARGS(5, 6);

/* This following lines are just meant as a reminder to use the mg-functions
 * for memory management */
#if defined(malloc)
#undef malloc
#endif
#if defined(calloc)
#undef calloc
#endif
#if defined(realloc)
#undef realloc
#endif
#if defined(free)
#undef free
#endif
#if defined(snprintf)
#undef snprintf
#endif
#if defined(vsnprintf)
#undef vsnprintf
#endif
#if !defined(NDEBUG)
#define malloc DO_NOT_USE_THIS_FUNCTION__USE_mg_malloc
#define calloc DO_NOT_USE_THIS_FUNCTION__USE_mg_calloc
#define realloc DO_NOT_USE_THIS_FUNCTION__USE_mg_realloc
#define free DO_NOT_USE_THIS_FUNCTION__USE_mg_free
#define snprintf DO_NOT_USE_THIS_FUNCTION__USE_mg_snprintf
#endif
#if defined(_WIN32)
/* vsnprintf must not be used in any system,
 * but this define only works well for Windows. */
#define vsnprintf DO_NOT_USE_THIS_FUNCTION__USE_mg_vsnprintf
#endif


/* mg_init_library counter */
static int mg_init_library_called = 0;

#if !defined(NO_SSL)
#if defined(OPENSSL_API_1_0) || defined(OPENSSL_API_1_1)                       \
    || defined(OPENSSL_API_3_0)
static int mg_openssl_initialized = 0;
#endif
#if !defined(OPENSSL_API_1_0) && !defined(OPENSSL_API_1_1)                     \
    && !defined(OPENSSL_API_3_0) && !defined(USE_MBEDTLS)
#error "Please define OPENSSL_API_#_# or USE_MBEDTLS"
#endif
#if defined(OPENSSL_API_1_0) && defined(OPENSSL_API_1_1)
#error "Multiple OPENSSL_API versions defined"
#endif
#if defined(OPENSSL_API_1_1) && defined(OPENSSL_API_3_0)
#error "Multiple OPENSSL_API versions defined"
#endif
#if defined(OPENSSL_API_1_0) && defined(OPENSSL_API_3_0)
#error "Multiple OPENSSL_API versions defined"
#endif
#if (defined(OPENSSL_API_1_0) || defined(OPENSSL_API_1_1)                      \
     || defined(OPENSSL_API_3_0))                                              \
    && defined(USE_MBEDTLS)
#error "Multiple SSL libraries defined"
#endif
#endif


static pthread_key_t sTlsKey; /* Thread local storage index */
static volatile ptrdiff_t thread_idx_max = 0;

#if defined(MG_LEGACY_INTERFACE)
#define MG_ALLOW_USING_GET_REQUEST_INFO_FOR_RESPONSE
#endif

struct mg_workerTLS {
	int is_master;
	unsigned long thread_idx;
	void *user_ptr;
#if defined(_WIN32)
	HANDLE pthread_cond_helper_mutex;
	struct mg_workerTLS *next_waiting_thread;
#endif
	const char *alpn_proto;
#if defined(MG_ALLOW_USING_GET_REQUEST_INFO_FOR_RESPONSE)
	char txtbuf[4];
#endif
};


#if defined(GCC_DIAGNOSTIC)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /* defined(GCC_DIAGNOSTIC) */
#if defined(__clang__)
/* Show no warning in case system functions are not used. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif


/* Get a unique thread ID as unsigned long, independent from the data type
 * of thread IDs defined by the operating system API.
 * If two calls to mg_current_thread_id  return the same value, they calls
 * are done from the same thread. If they return different values, they are
 * done from different threads. (Provided this function is used in the same
 * process context and threads are not repeatedly created and deleted, but
 * CivetWeb does not do that).
 * This function must match the signature required for SSL id callbacks:
 * CRYPTO_set_id_callback
 */
FUNCTION_MAY_BE_UNUSED
static unsigned long
mg_current_thread_id(void)
{
#if defined(_WIN32)
	return GetCurrentThreadId();
#else

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
	/* For every compiler, either "sizeof(pthread_t) > sizeof(unsigned long)"
	 * or not, so one of the two conditions will be unreachable by construction.
	 * Unfortunately the C standard does not define a way to check this at
	 * compile time, since the #if preprocessor conditions can not use the
	 * sizeof operator as an argument. */
#endif

	if (sizeof(pthread_t) > sizeof(unsigned long)) {
		/* This is the problematic case for CRYPTO_set_id_callback:
		 * The OS pthread_t can not be cast to unsigned long. */
		struct mg_workerTLS *tls =
		    (struct mg_workerTLS *)pthread_getspecific(sTlsKey);
		if (tls == NULL) {
			/* SSL called from an unknown thread: Create some thread index.
			 */
			tls = (struct mg_workerTLS *)mg_malloc(sizeof(struct mg_workerTLS));
			tls->is_master = -2; /* -2 means "3rd party thread" */
			tls->thread_idx = (unsigned)mg_atomic_inc(&thread_idx_max);
			pthread_setspecific(sTlsKey, tls);
		}
		return tls->thread_idx;
	} else {
		/* pthread_t may be any data type, so a simple cast to unsigned long
		 * can rise a warning/error, depending on the platform.
		 * Here memcpy is used as an anything-to-anything cast. */
		unsigned long ret = 0;
		pthread_t t = pthread_self();
		memcpy(&ret, &t, sizeof(pthread_t));
		return ret;
	}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif
}


FUNCTION_MAY_BE_UNUSED
static uint64_t
mg_get_current_time_ns(void)
{
	struct timespec tsnow;
	clock_gettime(CLOCK_REALTIME, &tsnow);
	return (((uint64_t)tsnow.tv_sec) * 1000000000) + (uint64_t)tsnow.tv_nsec;
}


#if defined(GCC_DIAGNOSTIC)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic pop
#endif /* defined(GCC_DIAGNOSTIC) */
#if defined(__clang__)
/* Show no warning in case system functions are not used. */
#pragma clang diagnostic pop
#endif


#if defined(NEED_DEBUG_TRACE_FUNC)
static void
DEBUG_TRACE_FUNC(const char *func, unsigned line, const char *fmt, ...)
{
	 #if defined(__ZEPHYR__)
	(void)func;
	(void)line;
	(void)fmt;
	return;
	#else
	va_list args;
	struct timespec tsnow;

	/* Get some operating system independent thread id */
	unsigned long thread_id = mg_current_thread_id();

	clock_gettime(CLOCK_REALTIME, &tsnow);

	flockfile(DEBUG_TRACE_STREAM);
	fprintf(DEBUG_TRACE_STREAM,
	        "*** %lu.%09lu %lu %s:%u: ",
	        (unsigned long)tsnow.tv_sec,
	        (unsigned long)tsnow.tv_nsec,
	        thread_id,
	        func,
	        line);
	va_start(args, fmt);
	vfprintf(DEBUG_TRACE_STREAM, fmt, args);
	va_end(args);
	putc('\n', DEBUG_TRACE_STREAM);
	fflush(DEBUG_TRACE_STREAM);
	funlockfile(DEBUG_TRACE_STREAM);
	#endif /* __ZEPHYR__ */
}
#endif /* NEED_DEBUG_TRACE_FUNC */


#define MD5_STATIC static
#include "md5.inl"

/* Darwin prior to 7.0 and Win32 do not have socklen_t */
#if defined(NO_SOCKLEN_T)
typedef int socklen_t;
#endif /* NO_SOCKLEN_T */

#define IP_ADDR_STR_LEN (50) /* IPv6 hex string is 46 chars */

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL (0)
#endif


/* SSL: mbedTLS vs. no-ssl vs. OpenSSL */
#if defined(USE_MBEDTLS)
/* mbedTLS */
#include "mod_mbedtls.inl"

#elif defined(NO_SSL)
/* no SSL */
typedef struct SSL SSL; /* dummy for SSL argument to push/pull */
typedef struct SSL_CTX SSL_CTX;

#elif defined(NO_SSL_DL)
/* OpenSSL without dynamic loading */
#include <openssl/bn.h>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/dh.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <openssl/x509.h>

#if defined(WOLFSSL_VERSION)
/* Additional defines for WolfSSL, see
 * https://github.com/civetweb/civetweb/issues/583 */
#include "wolfssl_extras.inl"
#endif

#if defined(OPENSSL_IS_BORINGSSL)
/* From boringssl/src/include/openssl/mem.h:
 *
 * OpenSSL has, historically, had a complex set of malloc debugging options.
 * However, that was written in a time before Valgrind and ASAN. Since we now
 * have those tools, the OpenSSL allocation functions are simply macros around
 * the standard memory functions.
 *
 * #define OPENSSL_free free */
#define free free
// disable for boringssl
#define CONF_modules_unload(a) ((void)0)
#define ENGINE_cleanup() ((void)0)
#endif

/* If OpenSSL headers are included, automatically select the API version */
#if (OPENSSL_VERSION_NUMBER >= 0x30000000L)
#if !defined(OPENSSL_API_3_0)
#define OPENSSL_API_3_0
#endif
#define OPENSSL_REMOVE_THREAD_STATE()
#else
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
#if !defined(OPENSSL_API_1_1)
#define OPENSSL_API_1_1
#endif
#define OPENSSL_REMOVE_THREAD_STATE()
#else
#if !defined(OPENSSL_API_1_0)
#define OPENSSL_API_1_0
#endif
#define OPENSSL_REMOVE_THREAD_STATE() ERR_remove_thread_state(NULL)
#endif
#endif


#else
/* SSL loaded dynamically from DLL / shared object */
/* Add all prototypes here, to be independent from OpenSSL source
 * installation. */
#include "openssl_dl.inl"

#endif /* Various SSL bindings */

#endif // 0

