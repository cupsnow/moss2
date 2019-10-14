#include <moss/moss.h>

const char *moss_newline = "\n";

static char _log_buf[256];
static moss_buf_t log_buf = {.data = _log_buf, .cap = sizeof(_log_buf)};

extern int moss_log(unsigned lvl, const char *tag, long lno,
		const char *fmt, ...) {
	va_list va;
	int r;

	va_start(va, fmt);
	log_buf.pos = log_buf.lmt = 0;
	r = moss_vlogf(&log_buf, lvl, tag, lno, fmt, va);
	va_end(va);
	printf("%s", (char*)log_buf.data);
	return r;
}

int moss_logt(moss_buf_t *buf, unsigned flag, const char *tag, long lno) {
//	struct timespec _t;
//	struct tm _tm;
//
//	clock_gettime(CLOCK_REALTIME, &_t);
//	localtime_r(&_t.tv_sec, &_tm);
//
//	return moss_buf_printf(buf, "%02d:%02d:%02d:%06d",
//			_tm.tm_hour, _tm.tm_min, _tm.tm_sec, (int)(_t.tv_nsec / 1000));
	return -1;
}

int moss_file_size(const char *path) {
//	struct stat st;
//	int r;
//
//	if (stat(path, &st) != 0) {
//		r = errno;
//		moss_error("Failed get file size: %s(%d)\n", strerror(r), r);
//		return -1;
//	}
//	if (!S_ISREG(st.st_mode)) {
//		moss_error("Not regular file: %s\n", path);
//		return -1;
//	}
//	return st.st_size;
	return -1;
}

unsigned long moss_ts1_get(unsigned long *ts0) {
//	struct timespec ts1;
//
//	if (clock_gettime(CLOCK_MONOTONIC, &ts1) != 0) {
//		int r = errno;
//
//		moss_error("get timestamp: %s(%d)\n", strerror(r), r);
//		return 0;
//	}
//	if (ts0) {
//		if (*ts0 == 0) {
//			*ts0 = ts1.tv_sec * 1e6 + ts1.tv_nsec / 1e3;
//			return 0;
//		}
//		return ts1.tv_sec * 1e6 + ts1.tv_nsec / 1e3 - *ts0;
//	}
//	return ts1.tv_sec * 1e6 + ts1.tv_nsec / 1e3;
	return 0;
}
