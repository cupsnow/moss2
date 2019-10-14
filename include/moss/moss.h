/**
 * @author joelai
 */

#ifndef _H_MOSS
#define _H_MOSS

/** @defgroup MOSS
 * Information for moss project.
 *
 * MOSS project provide runtime utility for multiple platform.  Target platform
 * is linux, mkr4000, mmw.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include <errno.h>
#include <sys/tree.h>
#include <sys/queue.h>

/** @defgroup MOSS_MISC Miscellaneous (misc)
 * Miscellaneous API.
 *
 * @ingroup MOSS
 *
 * Misc module provide miscellaneous function.  More detail grouped in subpage.
 */

/** @addtogroup MOSS_MISC
 * @{
 */

/** Minimal. */
#define MOSS_MIN(_a, _b) ((_a) <= (_b) ? (_a) : (_b))

/** Maximal. */
#define MOSS_MAX(_a, _b) ((_a) >= (_b) ? (_a) : (_b))

#define MOSS_LMT(_v, _l, _r) ((_v) <= (_l) ? (_l) : (_v) >= (_r) ? (_r) : (_v))

/** Stringify. */
#define MOSS_STRINGIFY(_s) # _s

/** Stringify expansion. */
#define MOSS_STRINGIFY2(_s) MOSS_STRINGIFY(_s)

/** Get parent from structure member.
 *
 * Example:
 * @code{.c}
 *
 * @endcode
 */
#define MOSS_CONTAINER_OF(_obj, _type, _member) \
	((_type *)((_obj) ? ((char*)(_obj) - offsetof(_type, _member)) : NULL))

/** Count array item. */
#define MOSS_ARRAYSIZE(_a) (sizeof(_a) / sizeof((_a)[0]))

/** Assert in build time. */
#define MOSS_ASSERT_BUILD(_cond) ((void)sizeof(char[1 - 2 * !(_cond)]))

/** Define bit mask against enumeration.
 *
 * Define group of bit masked flag in the enumeration.  The group of flag
 * occupied \<_group\>_mask_offset (count from bit0), and take \<_group\>_mask_bits.
 * \<_group\>_mask used to filter the occupied value.
 *
 * Example:
 * @code{.c}
 * typedef enum flag_enum {
 *   MOSS_FLAG_MASK(flag_class, 0, 2),
 *   MOSS_FLAG(flag_class, _case, 0),
 *   MOSS_FLAG(flag_class, _suite, 1),
 *   MOSS_FLAG_MASK(flag_result, 2, 2),
 *   MOSS_FLAG(flag_result, _pass, 0),
 *   MOSS_FLAG(flag_result, _failed_memory, 1),
 *   MOSS_FLAG(flag_result, _failed_io, 2),
 * } flag_t;
 *
 * flag_t flags = flag_class_suite | flag_result_failed_io; // 0x9
 * flag_t flags_class = flags & flag_class_mask; // 0x1
 * flag_t flags_result = flags & flag_result_mask; // 0x8
 * @endcode
 */
#define MOSS_FLAG_MASK(_group, _offset, _bits) \
		_group ## _mask_offset = _offset, \
		_group ## _mask_bits = _bits, \
		_group ## _mask = (((1 << (_bits)) - 1) << (_offset))

/** Define bit masked value against enumeration.
 *
 * Reference to MOSS_FLAG_MASK()
 */
#define MOSS_FLAG(_group, _name, _val) \
	_group ## _name = ((_val) << _group ## _mask_offset)

/** Carry return(cr) character(\\r, 0xd). */
#define MOSS_CR '\r'

/** Line feed(lf) character(\\n, 0xa). */
#define MOSS_LF '\n'

/** @} MOSS_MISC */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup MOSS_MISC
* @{
*/

/** String for new line. */
extern const char *moss_newline;

typedef unsigned long (*moss_memcpy_t)(void*, const void*, size_t);

/** Entry to red-black tree. */
typedef struct moss_rb_entry_rec {
	RB_ENTRY(moss_rb_entry_rec) entry;
	int (*cmp)(struct moss_rb_entry_rec *a, struct moss_rb_entry_rec *b);
} moss_rb_entry_t;

/** Head to red-black tree. */
typedef RB_HEAD(moss_rb_tree_rec, moss_rb_entry_rec) moss_rb_tree_t;

RB_PROTOTYPE(moss_rb_tree_rec, moss_rb_entry_rec, entry, );

/** Entry to tail queue.
 *
 * @param entry
 */
typedef struct moss_tailq_entry_rec {
	TAILQ_ENTRY(moss_tailq_entry_rec) entry;
} moss_tailq_entry_t;

/** Head to tail queue. */
typedef TAILQ_HEAD(moss_tailq_rec, moss_tailq_entry_rec) moss_tailq_t;

/** Strip characters from start of string. */
size_t moss_stripl(const void **buf, size_t sz, const char *ext);

/** @} MOSS_MISC */

/** @defgroup MOSS_BUF
 * Viewport for memory.
 *
 * @ingroup MOSS
 */

/** @addtogroup MOSS_BUF
 * @{
 */
/** moss buffer data structure. */
typedef struct moss_buf_rec {
	size_t pos, /**< Current data position of the data memory. */
			cap, /**< Total capable of the data memory. */
			lmt; /**< Valid data range. */
	void *data; /**< Pointer to data memory. */
	moss_memcpy_t memcpy;
} moss_buf_t;

int moss_buf_write(moss_buf_t *buf, const void *data, size_t sz);

/** Read from moss buffer to mempry.
 *
 * @param buf
 * @param data
 * @param sz
 * @return
 */
int moss_buf_read(moss_buf_t *buf, void *data, size_t sz);

/** Expand moss buffer capable.
 *
 * @param buf
 * @param cap
 * @param retain
 * @return 0 when success, others when failure.
 */
int moss_buf_expand(moss_buf_t *buf, size_t cap, int retain);

/** Printf to moss buffer.
 *
 * - Assume valid data occupied buf->lmt bytes from buf->pos.
 * - Printf from buf->pos + buf->lmt and append trailing 0.
 * - After printf successful, add the size of written character(no count
 *   trailing 0) to buf->lmt.
 * - If printf failed, restore the first character after original valid data
 *   (useful to retain the string trailing 0).
 *
 * @param buf
 * @param fmt
 * @param va
 * @return
 */
int moss_buf_vprintf(moss_buf_t *buf, const char *fmt, va_list va)
		__attribute__((format(printf, 2, 0)));

/** Printf to moss buffer.
 *
 * Reference to moss_buf_vprintf()
 *
 * @param buf
 * @param fmt
 * @return
 */
int moss_buf_printf(moss_buf_t *buf, const char *fmt, ...)
		__attribute__((format(printf, 2, 3)));

/** @} MOSS_BUF */

/** @defgroup MOSS_LOG
 * Message catalog.
 *
 * @ingroup MOSS
 *
 * A message log contains
 * - level: Severity of the message.
 * - timestamp: Time when issue the message.
 * - tag: Tagged name for the message (ie. method name).
 * - line number: Location of the source to issue the message (ie. Source code line number).
 * - message: The message.
 */

/** @addtogroup MOSS_LOG
 * @{
 */

/** Enum for log level. */
typedef enum moss_log_level_enum {
	MOSS_FLAG_MASK(moss_log_level, 0, 8), /**< MOSS_FLAG_MASK(moss_log_level, ). */
	MOSS_FLAG(moss_log_level, _error, 8), /**< Error level. */
	MOSS_FLAG(moss_log_level, _info, 9), /**< Information level. */
	MOSS_FLAG(moss_log_level, _debug, 10), /**< Debug level. */
	MOSS_FLAG(moss_log_level, _verbose, 11), /**< Verbose level. */
} moss_log_level_t;

/** Provide timestamp string.
 *
 * @param buf Buffer to save string.
 * @param flag Log level.
 * @param tag Log tag.
 * @param lno Line number.
 * @return Bytes appended to the buffer.
 */
extern int moss_logt(moss_buf_t *buf, unsigned flag, const char *tag, long lno);

/** Format the message log.
 *
 * @param buf
 * @param flag
 * @param tag
 * @param lno
 * @param fmt
 * @param va
 * @return
 */
int moss_vlogf(moss_buf_t *buf, unsigned flag, const char *tag, long lno,
		const char *fmt, va_list va) __attribute__((format(printf, 5, 0)));

/** Format the message log.
 *
 *
 * @param buf
 * @param flag
 * @param tag
 * @param lno
 * @param fmt
 * @return
 */
int moss_logf(moss_buf_t *buf, unsigned flag, const char *tag, long lno,
		const char *fmt, ...) __attribute__((format(printf, 5, 6)));

/** The max severity to output. */
extern int moss_log_level_max;

/** Convert log enum to string. */
#define moss_level_str(_lvl, _na) \
	((_lvl) == moss_log_level_error ? "ERROR" : \
	(_lvl) == moss_log_level_info ? "INFO" : \
	(_lvl) == moss_log_level_debug ? "Debug" : \
	(_lvl) == moss_log_level_verbose ? "verbose" : \
	(_na))

/** Output the message. */
extern int moss_vlog(unsigned lvl, const char *tag, long lno,
		const char *fmt, va_list va) __attribute__((format(printf, 4, 0)));

extern int moss_log(unsigned lvl, const char *tag, long lno,
		const char *fmt, ...) __attribute__((format(printf, 4, 5)));

/** Output the error severity message. */
#define moss_error(...) moss_log(moss_log_level_error, __func__, __LINE__, __VA_ARGS__)

/** Output the debug severity message. */
#define moss_debug(...) moss_log(moss_log_level_debug, __func__, __LINE__, __VA_ARGS__)

/** Output the information severity message. */
#define moss_info(...) moss_log(moss_log_level_info, __func__, __LINE__, __VA_ARGS__)

/** Output the verbose severity message. */
#define moss_verbose(...) moss_log(moss_log_level_verbose, __func__, __LINE__, __VA_ARGS__)

/** @} MOSS_LOG */

/** @addtogroup MOSS_MISC
 * @{
 */

/** Find a line.
 *
 * Accept end of line(EOL) for \<CR\>\<LF\> or \<LF\>.
 *
 * @param getc A function to get a character, return negative to end process.
 * @param arg The argument pass to getc().
 * @param _nl
 *   - 0: Missing EOL.
 *   - 1: EOL is \<LF\>.
 *   - 2: EOL is \<CR\>\<LF\>
 * @return
 */
int moss_readline(int (*getc)(void *arg), void *arg, char *_nl);

/** Get file size. */
extern int moss_file_size(const char *path);

#ifdef __GNUC__
/* 4 float vector type */
typedef float v4sf_t __attribute__((vector_size(sizeof(float) * 4)));

void moss_matrix_mul_v4sf(int am, int an, float *a, int bn, float *b, float *c);
#endif

/** Matrix multiply.
 *
 * The matrix e.g. b[2][3] contains **m** (2 rows) \* **n** (3 columns), memory
 * organized in [b11, b12, b13, b21, b22, b23].
 *
 * Multiplication illustrate.
 *
 *                 b11 *b12  b13
 *                 b21 *b22  b23
 *               +--------------
 *      a11  a12 | c11  c12  c13
 *     *a21 *a22 | c21 *c22  c23  c22 = a21 * b12 + a22 * b22
 *      a31  a32 | c31  c32  c33
 *      a41  a42 | c41  c42  c43
 *
 * @param am The **m** (row) of the matrix **a**
 * @param an The **n** (column) of the matrix **a**
 * @param a The matrix **a**
 * @param bn The **n** (column) of the matrix **b**
 * @param b The matrix **b**
 * @param c The matrix **c** (output)
 */
void moss_matrix_mul_sw(int am, int an, float *a, int bn, float *b, float *c);
void moss_matrix_mul(int am, int an, float *a, int bn, float *b, float *c);

/** Get timestamp in type unsigned long. */
extern unsigned long moss_ts1_get(unsigned long *ts0);

/** @} MOSS_MISC */

/** @defgroup MOSS_HEX Hex dump format.
 * @ingroup MOSS
 * @brief Hex dump format.
 *
 * @{
 */

/** Convert a decimal number to hex ASCII.
 *
 * @param _d A decimal number.
 * @param _a The character above decimal 9.
 */
#define _moss_int2hexstr(_d, _a) ((_d) < 10 ? (_d) + '0' : (_d) - 10 + (_a))

/** Convert integer to hex ASCII.
 *
 * @param _buf
 * @param val
 * @param width
 * @param cap
 */
void moss_int2hexstr(void *_buf, unsigned val, int width, int cap);

/** Hex dump to buffer.
 *
 * @param _buf
 * @param buf_sz
 * @param data
 * @param data_sz
 * @param sep
 * @return
 */
size_t moss_hd(void *_buf, size_t buf_sz, const uint8_t *data, size_t data_sz,
		const char *sep);

/** Hex dump to buffer.
 *
 * Native byte order.
 *
 * @param _buf
 * @param buf_sz
 * @param data
 * @param data_cnt
 * @param width
 * @param sep
 * @return
 */
size_t moss_hd2(void *_buf, size_t buf_sz, const void *data, size_t data_cnt,
		char width, const char *sep);

#define MOSS_SHOWHEX_BUF_MIN 78
typedef long (*moss_showhex_sout_t)(void*, const void*, unsigned long);
/** Hex dump to buffer.
 *
 * Example:
 * @code
 * 	char ln_buf[MOSS_SHOWHEX_BUF_MIN];
 * 	moss_buf_t hex_buf = {.data = _buf, .cap = _buf_sz};
 * 	moss_showhex(ln_buf, _data, _data_sz, _addr,
 * 	    (moss_showhex_sout_t)&moss_showhex_sout, &hex_buf);
 * @endcode
 *
 * @param _buf
 * @param _data
 * @param sz
 * @param _addr
 * @param sout
 * @param arg
 * @return
 */
int moss_showhex(void *_buf, const void *_data, size_t sz, unsigned long _addr,
		moss_showhex_sout_t sout, void *arg);

/** Append text to buffer. */
long moss_showhex_sout(moss_buf_t *buf, const void *msg, unsigned long len);

/** @} MOSS_HEX */

int moss_cli_tok(char *cli, int *tok_argc, char **tok_argv, const char *sep);

/**
 * get mask of the bit[3..4]: MOSS_BITMASK(3, 2) -> 0x18
 */
#define MOSS_BITMASK(_shift, _bits) (((1 << (_bits)) - 1) << (_shift))

/**
 * set 0x2 into bit[3..4] of 0x5a: MOSS_BITWORD(0x5a, 3, 2, 0x2) -> 0x52
 */
#define MOSS_BITWORD(_word, _shift, _bits, _field) \
	(((_word) & ~MOSS_BITMASK(_shift, _bits)) | \
	(((_field) & MOSS_BITMASK(0, _bits)) << (_shift)))

/** Parse cli to \"<addr7> [w [bytes...]] [r [size]]\".
 *
 * "<addr7> [w [bytes...]] [r [size]]"
 *
 * @param argc
 * @param argv
 * @param addr7
 * @param wbuf
 * @param wlen
 * @param rlen
 * @return
 */
int moss_parse_i2c_cli(int32_t argc, const char **argv, unsigned *addr7,
		uint8_t *wbuf, unsigned *wlen, unsigned *rlen);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _H_MOSS */
