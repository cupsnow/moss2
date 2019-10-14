#include <moss/moss.h>

static unsigned long alt_memcpy(moss_memcpy_t func, void *tgt, const void *src,
		size_t sz) {
	if (func) return func(tgt, src, sz);
	memcpy(tgt, src, sz);
	return 0;
}

static int moss_rb_cmp(moss_rb_entry_t *a, moss_rb_entry_t *b)
{
	if (a->cmp) return (a->cmp)(a, b);
	return (long)a - (long)b;
}
RB_GENERATE(moss_rb_tree_rec, moss_rb_entry_rec, entry, moss_rb_cmp);

size_t moss_stripl(const void **buf, size_t sz, const char *ext) {
	while (sz > 0) {
		if ((*(char**)buf)[0] == '\0' || (ext && strchr(ext, (*(char**)buf)[0]))) {
			*(char**)buf += 1;
			sz--;
			continue;
		}
		break;
	}
	return sz;
}

int moss_buf_write(moss_buf_t *buf, const void *data, size_t sz) {
	int sz_max = MOSS_MIN(sz, (buf->cap - buf->lmt));
	if (!data || sz_max <= 0) return 0;
	char *data_pos = (char*)buf->data + buf->pos + buf->lmt;
	if (data_pos >= (char*)buf->data + buf->cap) data_pos -= buf->cap;
	int data_sz = MOSS_MIN(sz_max, (char*)buf->data + buf->cap - data_pos);
	alt_memcpy(buf->memcpy, data_pos, data, data_sz);
	if (sz_max > data_sz) {
		alt_memcpy(buf->memcpy, buf->data, (char*)data + data_sz,
				sz_max - data_sz);
	}
	buf->lmt += sz_max;
	return sz - sz_max;
}

int moss_buf_read(moss_buf_t *buf, void *data, size_t sz) {
	size_t _sz = sz = MOSS_MIN(sz, buf->lmt);
	while (sz > 0) {
		char *data_pos = (char*)buf->data + buf->pos;
		int data_sz = buf->cap - buf->pos;
		if (data_sz > sz) data_sz = sz;
		alt_memcpy(buf->memcpy, data, data_pos, data_sz);
		if ((buf->pos += data_sz) >= buf->cap)
			buf->pos -= buf->cap;
		buf->lmt -= data_sz;
		sz -= data_sz;
		data = (char*)data + data_sz;
	}
	return _sz;
}

int moss_buf_expand(moss_buf_t *buf, size_t cap, int retain) {
	void *data;

	if (cap <= 0 || buf->cap >= cap) return 0;
	cap = (cap + 0x3fff) & ~0x3fff;
	if (!(data = malloc(cap))) {
		return -1;
	}
	if (buf->data) {
		if (retain && buf->lmt > 0) {
			moss_buf_read(buf, data, buf->lmt);
			buf->pos = 0;
		}
		free(buf->data);
	}
	buf->data = data;
	buf->cap = cap;
	return 0;
}

int moss_buf_vprintf(moss_buf_t *buf, const char *fmt, va_list va) {
	int r;
	char ch, *ch_pos = NULL;

	if (!fmt) return 0;
	if (buf->pos + buf->lmt < buf->cap) {
		ch = *(ch_pos = (char*)buf->data + buf->pos + buf->lmt);
	}
	r = vsnprintf((char*)buf->data + buf->pos + buf->lmt,
			buf->cap - buf->pos - buf->lmt, fmt, va);
	if (buf->pos + buf->lmt + r >= buf->cap) {
		if (ch_pos) *ch_pos = ch;
		return -1;
	}
	buf->lmt += r;
	return 0;
}

int moss_buf_printf(moss_buf_t *buf, const char *fmt, ...) {
	int r;
	va_list va;

	if (!fmt) return 0;
	va_start(va, fmt);
	r = moss_buf_vprintf(buf, fmt, va);
	va_end(va);
	return r;
}

int moss_vlogf(moss_buf_t *buf, unsigned flag, const char *tag, long lno,
		const char *fmt, va_list va) {
	char tm_str[32];
	moss_buf_t tm_buf = {.data = tm_str, .cap = sizeof(tm_str) - 1};

	if (moss_logt(&tm_buf, flag, tag, lno) != 0) {
		((char*)tm_buf.data)[tm_buf.lmt = 0] = '\0';
	} else {
		((char*)tm_buf.data)[tm_buf.lmt++] = ' ';
		((char*)tm_buf.data)[tm_buf.lmt++] = '\0';
	}

	if (moss_buf_printf(buf, "%s %s%s #%ld ",
			moss_level_str(flag & moss_log_level_mask, ""),
			tm_str, tag, lno) != 0) {
		return -1;
	}

	if (moss_buf_vprintf(buf, fmt, va) != 0) {
		return -1;
	}
	return 0;
}

int moss_logf(moss_buf_t *buf, unsigned flag, const char *tag, long lno,
		const char *fmt, ...) {
	va_list va;
	int r;

	va_start(va, fmt);
	r = moss_vlogf(buf, flag, tag, lno, fmt, va);
	va_end(va);
	return r;
}

int moss_readline(int (*getc)(void *arg), void *arg, char *_nl)
{
	int c, n;
	char nl;

	for (n = 0, nl = 0; (c = (*getc)(arg)) >= 0; n++) {
		if (c == MOSS_LF) {
			if (_nl) *_nl = nl + 1;
			return n - nl;
		}
		nl = (c == MOSS_CR);
	}
	if (_nl) *_nl = 0;
	return (n <= 0) ? -1 : n;
}

int moss_log_level_max = moss_log_level_info;

#ifdef __GNUC__
void moss_matrix_mul_v4sf(int am, int an, float *a, int bn, float *b,
		float *c) {
	int m, n;

	for (m = 0; m < am; m++) {
		for (n = 0; n < bn; n++) {
			v4sf_t va, vb, vc;

			switch(an) {
			case 4:
				va[3] = a[3]; vb[3] = b[n + bn + bn + bn];
			case 3:
				va[2] = a[2]; vb[2] = b[n + bn + bn];
			case 2:
				va[1] = a[1]; vb[1] = b[n + bn];
			case 1:
			default:
				va[0] = a[0]; vb[0] = b[n];
			}

			vc = va * vb;
			*c = vc[0];

			switch(an) {
			case 4:
				*c += vc[3];
			case 3:
				*c += vc[2];
			case 2:
				*c += vc[1];
			case 1:
			default:
				;
			}
			c++;
		}
		a += an;
	}
}
#endif

void moss_matrix_mul_sw(int am, int an, float *a, int bn, float *b,
		float *c) {
	int m, n;

	for (m = 0; m < am; m++) {
		for (n = 0; n < bn; n++) {
			int z, bm;
			register float cr;

			for (cr = a[0] * b[n], bm = n + bn, z = 1; z < an; bm += bn, z++) {
				cr += a[z] * b[bm];
			}
			*c++ = cr;
		}
		a += an;
	}
}

void moss_matrix_mul(int am, int an, float *a, int bn, float *b,
		float *c) {
#if defined(__GNUC__) && 0
	if (an <= 4) {
		moss_matrix_mul_v4sf(am, an, a, bn, b, c);
		return;
	}
#endif
	moss_matrix_mul_sw(am, an, a, bn, b, c);
}

void moss_int2hexstr(void *_buf, unsigned val, int width, int cap) {
	char *buf = (char*)_buf;
	int nw;

	if (!buf || width < 1) return;
	buf += width;

	nw = MOSS_MIN(width, sizeof(val) * 2);
	width -= nw;
	while (nw-- > 0) {
		int d = val & 0xf;
		*--buf = _moss_int2hexstr(d, cap);
		val >>= 4;
	}
	while (width-- > 0) {
		*--buf = '?';
	}
}

size_t moss_hd(void *_buf, size_t buf_sz, const uint8_t *data, size_t data_sz,
		const char *sep) {
	int i = 0, sep_len;
	uint8_t *buf = (uint8_t*)_buf;

	// buf enough for [hex*2, \0]
	if (data_sz <= 0 || buf_sz < 3) return 0;

	if (!sep) sep = " ";
	sep_len = strlen(sep);

	moss_int2hexstr(buf, *data, 2, 'a');
	buf += 2;
	buf_sz -= 2;
	data++;

	for (i = 1; i < data_sz; i++, data++, buf += (sep_len + 2)) {
		// buf enough for [sp, hex*2, \0]
		if (buf_sz < (sep_len + 3)) break;
		memcpy(buf, sep, sep_len);
		moss_int2hexstr(buf + sep_len, *data, 2, 'a');
	}
	*buf = '\0';
	return (i - 1) * sep_len + i * 2;
}

size_t moss_hd2(void *_buf, size_t buf_sz, const void *data, size_t data_cnt,
		char width, const char *sep) {
	int i, sep_len;
	uint8_t *buf = (uint8_t*)_buf;
#define uint(_p, _w) ((_w) == 4 ? *(uint32_t*)(_p) : \
		(_w) == 2 ? *(uint16_t*)(_p) : *(uint8_t*)(_p))

	for (i = (1 << 2); i > 0; i>>=1) {
		if (width >= i) {
			width = i;
			break;
		}
	}
	if (width <= 0) width = 1;

	// buf enough for [hex*2 ..., \0]
	if (data_cnt <= 0 || buf_sz < (width * 2 + 1)) return 0;

	if (!sep) sep = " ";
	sep_len = strlen(sep);

	moss_int2hexstr(buf, uint(data, width), width * 2, 'a');
	buf += width * 2;
	buf_sz -= width * 2;
	data = (uint8_t*)data + width;

	for (i = 1; i < data_cnt; i++, data = (uint8_t*)data + width,
			buf += (sep_len + width * 2)) {
		// buf enough for [sp, hex*2 ..., \0]
		if (buf_sz < (sep_len + width * 2 + 1)) break;
		memcpy(buf, sep, sep_len);
		moss_int2hexstr(buf + sep_len, uint(data, width), width * 2, 'a');
	}
	*buf = '\0';
	return (i - 1) * sep_len + i * width * 2;
}

int moss_showhex(void *_buf, const void *_data, size_t sz, unsigned long _addr,
		moss_showhex_sout_t sout, void *arg) {
#if MOSS_SHOWHEX_BUF_MIN < 78
#  error "MOSS_SHOWHEX_BUF_MIN minimal 78"
#endif
/* 0         1         2         3         4         5         6         7
 * 7FFF 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | 0123456789abcdef
 *      ----------------------------------------------- | ----------------
 * DDBE                                           1f 20 |               .
 * DDC0 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 | !"#$%&'()*+,-./0
 */
	char *buf = (char*)_buf, *data = (char*)_data;
	int hpos = 0, r = 0, nlen = strlen(moss_newline);
	unsigned addr;

	addr = (unsigned)(_addr >= 0x10000lu ? _addr / 0x10000lu : 0);
	moss_int2hexstr(buf, addr, 4, 'A');
	buf[4] = ' ';
	snprintf(buf + 5, MOSS_SHOWHEX_BUF_MIN - 5,
			"00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | 0123456789abcdef%s", moss_newline);
	if ((r = sout(arg, buf, 71 + nlen)) < 0) return r;

	snprintf(buf, MOSS_SHOWHEX_BUF_MIN,
			"     ----------------------------------------------- | ----------------%s", moss_newline);
	if ((r = sout(arg, buf, 71 + nlen)) < 0) return r;

	addr = (unsigned)_addr;
	while (sz > 0) {
		int i;

		if (hpos == 0) {
			moss_int2hexstr(buf, addr & (~0xf), 4, 'A');
			for (i = 4; i < 71; i++) buf[i] = ' ';
			buf[53] = '|';
		}
		if ((addr & 0xf) == hpos) {
			moss_int2hexstr(buf + hpos * 3 + 5, *data, 2, 'a');
			buf[hpos + 55] = isprint(*data) ? *data : '.';
			data++;
			addr++;
			sz--;
		}
		if (++hpos > 0xf || sz == 0) {
			hpos = 0;
			memcpy(buf + 71, moss_newline, nlen);
			buf[71 + nlen] = '\0';
			if ((r = sout(arg, buf, 71 + nlen) < 0)) return r;
		}
	}
	return r;
}

long moss_showhex_sout(moss_buf_t *buf, const void *msg, unsigned long len) {
	if (buf->pos >= buf->cap) return -1;
	if (len > buf->cap - buf->pos - 1) len = buf->cap - buf->pos - 1;
	if (len > 0) memcpy((char*)buf->data + buf->pos, msg, len);
	((char*)buf->data)[buf->pos += len] = '\0';
	if (buf->pos + 1 >= buf->cap) return -1;
	return 0;
}

int moss_cli_tok(char *cli, int *tok_argc, char **tok_argv, const char *sep) {
	int tok_argv_sz = *tok_argc;
	char *tok_str[1];

	if (!sep) sep = " \r\n\t";
	for (*tok_argc = 0, tok_argv[*tok_argc] = strtok_r(cli, sep, &tok_str[0]);
			tok_argv[*tok_argc] && (*tok_argc < tok_argv_sz);
			(*tok_argc)++, tok_argv[*tok_argc] = strtok_r(NULL, sep, &tok_str[0]));
	return 0;
}

int moss_parse_i2c_cli(int32_t argc, const char **argv, unsigned *addr7,
		uint8_t *wbuf, unsigned *wlen, unsigned *rlen) {
	enum {
		flag_null,
		flag_parse_addr7,
		flag_parse_rw,
		flag_parse_w,
		flag_parse_ww,
		flag_parse_r,
		flag_parse_rr,
		flag_done,
	};
	int parse = flag_null, parsed_argc = 1;
	unsigned wbuf_sz = *wlen;

	*wlen = *rlen = 0;
	while (1) {
		if ((parse == flag_null) || (parse == flag_parse_addr7)) {
			parse = flag_parse_addr7;
			if (argc < (parsed_argc + 1)) {
				moss_error("parse addr7\n");
				return -1;
			}
			*addr7 = (unsigned)strtol(argv[parsed_argc++], NULL, 0);
			parse = flag_parse_rw;
			continue;
		}
		if (parse == flag_parse_rw) {
			if (argc < (parsed_argc + 1)) {
				moss_error("parse rw\n");
				return -1;
			}
			switch(*argv[parsed_argc++]) {
			case 'w':
			case 'W':
				parse = flag_parse_w;
				break;
			case 'r':
			case 'R':
				parse = flag_parse_r;
				break;
			default:
				moss_error("parse r/w\n");
				return -1;
			}
			continue;
		}
		if ((parse == flag_parse_w) || (parse == flag_parse_ww)) {
			parse = flag_parse_ww;
			if (argc < (parsed_argc + 1)) {
				parse = flag_done;
				break;
			}
			if (!isdigit(*argv[parsed_argc])) {
				parse = flag_parse_rw;
				continue;
			}
			if (*wlen >= wbuf_sz) {
				moss_error("data_w insufficient\n");
				return -1;
			}
			wbuf[(*wlen)++] = (unsigned)strtol(argv[parsed_argc++], NULL, 0);
			continue;
		}
		if ((parse == flag_parse_r) || (parse == flag_parse_rr)) {
			parse = flag_parse_rr;
			if (argc < (parsed_argc + 1)) {
				parse = flag_done;
				break;
			}
			if (!isdigit(*argv[parsed_argc])) {
				parse = flag_parse_rw;
				continue;
			}
			*rlen = (unsigned)strtol(argv[parsed_argc++], NULL, 0);
			continue;
		}
		if (parse == flag_done) {
			break;
		}
		moss_error("fsm\n");
		return -1;
	}
	return 0;
}

