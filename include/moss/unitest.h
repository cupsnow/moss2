/** @author: joelai */

#ifndef _H_MOSS_UNITEST
#define _H_MOSS_UNITEST

#include <moss/moss.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOSS_UNITEST Unit test.
 * @ingroup MOSS
 * @brief Public method to write test case.
 *
 * - Test groups to tree.
 * - Test suite could contain test suites and cases.
 * - The tree runs deep first.
 * - Test case failure may break the containing suite.
 *
 *  @dot "Run test"
 *  digraph G {
 *    subgraph test_suite {
 *      setup;
 *      loop_content[label="loop content"];
 *      shutdown;
 *
 *      setup -> loop_content;
 *      setup -> failed_suite;
 *      loop_content -> shutdown;
 *    };
 *
 *    subgraph test_case {
 *      test_runner;
 *    };
 *
 *    subgraph loop_content {
 *
 *    	runner -> test_suite;
 *    	runner -> test_case;
 *
 *    };
 *
 *  }
 *  @enddot
 *
 *  @{
 */

/** Flags used in API.
 */
typedef enum moss_unitest_flag_enum {
	MOSS_FLAG_MASK(moss_unitest_flag_result, 0, 4),
	MOSS_FLAG(moss_unitest_flag_result, _pass, 0), /**< Test passed. */

	MOSS_FLAG(moss_unitest_flag_result,_failed, 1), /**< Test failure. */

	/** This test failed and populate to containing suite failure. */
	MOSS_FLAG(moss_unitest_flag_result,_failed_suite, 2),

	/** This test failed due to prerequisite, ie. former failed suite. */
	MOSS_FLAG(moss_unitest_flag_result,_prerequisite, 3),

	MOSS_FLAG_MASK(moss_unitest_flag_class, 4, 2),
	MOSS_FLAG(moss_unitest_flag_class, _case, 0),
	MOSS_FLAG(moss_unitest_flag_class, _suite, 1),
} moss_unitest_flag_t;

typedef struct moss_unitest_case_rec {
	const char *name, *cause;
	moss_tailq_entry_t qent;
	moss_unitest_flag_t (*run)(struct moss_unitest_case_rec*);
	moss_unitest_flag_t flag_class, flag_result;
} moss_unitest_case_t;

typedef struct moss_unitest_rec {
	moss_tailq_t cases;
	moss_unitest_case_t runner;
	moss_unitest_flag_t (*setup)(struct moss_unitest_rec*);
	void (*shutdown)(struct moss_unitest_rec*);
} moss_unitest_t;

/** Add to suite. */
#define MOSS_UNITEST_ADD(_baseobj, _obj) \
		TAILQ_INSERT_TAIL(&(_baseobj)->cases, &(_obj)->qent, entry)

/** Initialize test suite data structure. */
#define MOSS_UNITEST_INIT(_obj, _name) do { \
	memset(_obj, 0, sizeof(*(_obj))); \
	TAILQ_INIT(&(_obj)->cases); \
	(_obj)->runner.name = _name; \
	(_obj)->runner.flag_class = moss_unitest_flag_class_suite; \
	(_obj)->runner.run = &moss_unitest_runner; \
} while(0)

#define MOSS_UNITEST_INIT2(_baseobj, _obj, _name) do { \
	MOSS_UNITEST_INIT(_obj, _name); \
	MOSS_UNITEST_ADD(_baseobj, &(_obj)->runner); \
} while(0)

/** Run test suite. */
#define MOSS_UNITEST_RUN(_obj) (_obj)->runner.run(&(_obj)->runner)

/** Initialize test case data structure. */
#define MOSS_UNITEST_CASE_INIT(_obj, _name, _runner) do { \
	memset(_obj, 0, sizeof(*(_obj))); \
	(_obj)->name = _name; \
	(_obj)->flag_class = moss_unitest_flag_class_case; \
	(_obj)->run = _runner; \
} while(0)

#define MOSS_UNITEST_CASE_INIT2(_baseobj, _obj, _name, _case_runner) do { \
	MOSS_UNITEST_CASE_INIT(_obj, _name, _case_runner); \
	MOSS_UNITEST_ADD(_baseobj, _obj); \
} while(0)

#define MOSS_UNITEST_CASE_INIT4(_baseobj, _name, _case_runner) do { \
	static moss_unitest_case_t _obj; \
	MOSS_UNITEST_CASE_INIT(&_obj, _name, _case_runner); \
	MOSS_UNITEST_ADD(_baseobj, &_obj); \
} while(0)

/** Show test suite report. */
#define MOSS_UNITEST_REPORT(_obj, _args...) do { \
	moss_unitest_report_t report_info = {0}; \
	moss_unitest_report(_obj, &report_info); \
	moss_debug("Report result %s, test suite[%s]%s" \
			"  Summary total cases PASS: %d, FAILED: %d(PREREQUISITE: %d), TOTAL: %d%s", \
			MOSS_UNITEST_RESULT_STR((_obj)->runner.flag_result, "UNKNOWN"), \
			(_obj)->runner.name, moss_newline, \
			report_info.pass, report_info.failed, report_info.failed_prereq, \
			report_info.total, moss_newline); \
	_args; \
} while(0)

#define MOSS_UNITEST_RESULT_STR(_val, _unknown) ( \
		(_val) == moss_unitest_flag_result_pass ? "PASS" : \
		(_val) == moss_unitest_flag_result_failed ? "FAILED" : \
		(_val) == moss_unitest_flag_result_failed_suite ? "FAILED-SUITE" : \
		(_val) == moss_unitest_flag_result_prerequisite ? "FAILED-PREREQUISITE" : \
		_unknown)

#define MOSS_UNITEST_CLASS_STR(_val, _unknown) ( \
		(_val) == moss_unitest_flag_class_suite ? "suite" : \
		(_val) == moss_unitest_flag_class_case ? "case" : \
		_unknown)

#define MOSS_UNITEST_ASSERT_THEN(_cond, _runner, _res, _args...) if (!(_cond)) { \
	(_runner)->cause = "#" MOSS_STRINGIFY2(__LINE__) \
			" " MOSS_STRINGIFY2(_cond); \
	(_runner)->flag_result = moss_unitest_flag_result_ ## _res; \
	_args; \
}

#define MOSS_UNITEST_ASSERT_RETURN(_cond, _runner, _res) \
		MOSS_UNITEST_ASSERT_THEN(_cond, _runner, _res, { \
	return (_runner)->flag_result; \
})

/** Start test suite.
 *
 *   Context to execute test case and reentrant for contained test suite.
 *
 *  **Reentrant with prerequisite failure suite:**
 *  ```
 *  - Skip setup
 *  - Populate prerequisite failure in contained test suite and case.
 *  ```
 *
 * @param
 * @return
 */
moss_unitest_flag_t moss_unitest_runner(moss_unitest_case_t*);

typedef struct moss_unitest_report_rec {
	int (*runner)(moss_unitest_case_t*, struct moss_unitest_report_rec*);
	int pass, failed, total, failed_prereq;
	int (*log)(unsigned lvl, const char *tag, long lno,
			const char *fmt, ...) __attribute__((format(printf, 4, 5)));
} moss_unitest_report_t;

int moss_unitest_report(moss_unitest_t*, moss_unitest_report_t*);

/** @} MOSS_UNITEST */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _H_MOSS_UNITEST */
