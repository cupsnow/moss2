/** @author joelai */

#include <moss/unitest.h>

moss_unitest_flag_t moss_unitest_runner(moss_unitest_case_t *suite_runner) {
	moss_unitest_t *suite = MOSS_CONTAINER_OF(suite_runner, moss_unitest_t, runner);
	moss_tailq_entry_t *qent;

	moss_debug("Start test suite[%s]%s", suite->runner.name, moss_newline);

	// The most use-case for setup on suite
	if ((suite_runner->flag_result == moss_unitest_flag_result_pass) &&
			suite->setup &&
			((suite->setup)(suite) != moss_unitest_flag_result_pass)) {
		suite_runner->flag_result = moss_unitest_flag_result_failed_suite;
		if (!suite_runner->cause) suite_runner->cause = "SETUP";
		moss_debug("Setup failed test suite[%s]%s", suite_runner->name, moss_newline);
	}

	TAILQ_FOREACH(qent, &suite->cases, entry) {
		moss_unitest_case_t *case_runner = MOSS_CONTAINER_OF(qent,
				moss_unitest_case_t, qent);

		// Populate prerequisite to contained suites and cases.
		if ((suite_runner->flag_result == moss_unitest_flag_result_failed_suite) ||
				(suite_runner->flag_result == moss_unitest_flag_result_prerequisite)) {
			case_runner->flag_result = moss_unitest_flag_result_prerequisite;
			case_runner->cause = "PREREQUISITE";
			moss_debug("%s for test %s[%s]%s",
					MOSS_UNITEST_RESULT_STR(case_runner->flag_result, "UNKNOWN result"),
					MOSS_UNITEST_CLASS_STR(case_runner->flag_class, "UNKNOWN class"),
					case_runner->name, moss_newline);

			if (case_runner->flag_class == moss_unitest_flag_class_suite) {
				(case_runner->run)(case_runner);
			}
			continue;
		}

		// Test suite failure do not break containing suite.
		if (case_runner->flag_class == moss_unitest_flag_class_suite) {
			case_runner->flag_result = (case_runner->run)(case_runner);
			if (case_runner->flag_result != moss_unitest_flag_result_pass) {
				if (!case_runner->cause) case_runner->cause = "RUN";
				if (suite_runner->flag_result == moss_unitest_flag_result_pass) {
					suite_runner->flag_result = moss_unitest_flag_result_failed;
					suite_runner->cause = case_runner->name;
				}
			}
			continue;
		}

		moss_debug("Start test case[%s]%s", case_runner->name, moss_newline);
		if ((case_runner->flag_result = (case_runner->run)(case_runner)) !=
				moss_unitest_flag_result_pass) {
			if (!case_runner->cause) case_runner->cause = "RUN";
			moss_debug("%s for test case[%s]%s"
					"  Cause: %s%s",
					MOSS_UNITEST_RESULT_STR(case_runner->flag_result, "UNKNOWN result"),
					case_runner->name, moss_newline,
					case_runner->cause, moss_newline);
			if (suite_runner->flag_result < case_runner->flag_result) {
				suite_runner->flag_result = case_runner->flag_result;
				suite_runner->cause = case_runner->name;
				moss_debug("%s for test suite[%s]%s"
						"  Cause: %s%s",
						MOSS_UNITEST_RESULT_STR(suite_runner->flag_result, "UNKNOWN result"),
						suite_runner->name, moss_newline,
						suite_runner->cause, moss_newline);
			}
		}
		moss_debug("Stopped test case[%s]%s", case_runner->name, moss_newline);
	}

	if (suite->shutdown) (suite->shutdown)(suite);

	moss_debug("Stopped test suite[%s]%s", suite_runner->name, moss_newline);

	return suite_runner->flag_result;
}

int moss_unitest_report(moss_unitest_t *suite, moss_unitest_report_t *report_runner) {
#define report_log(_lvl, _args...) if (report_runner->log) { \
		(*report_runner->log)((unsigned)(_lvl), __func__, __LINE__, _args); \
}
#define report_log_d(_args...) report_log(moss_log_level_debug, _args)
#define report_log_e(_args...) report_log(moss_log_level_error, _args)

	moss_tailq_entry_t *qent;
	int r = 0, pass = 0, failed = 0, total = 0, failed_prereq = 0;

	TAILQ_FOREACH(qent, &suite->cases, entry) {
		moss_unitest_case_t *case_runner = MOSS_CONTAINER_OF(qent,
				moss_unitest_case_t, qent);

		if (report_runner && report_runner->runner &&
				((r = (*report_runner->runner)(case_runner, report_runner)) != 0)) {
			report_log_e("Report runner break%s", moss_newline);
			break;
		}

		if (case_runner->flag_class == moss_unitest_flag_class_suite) {
			if ((r = moss_unitest_report(MOSS_CONTAINER_OF(case_runner,
					moss_unitest_t, runner), report_runner)) != 0) {
				report_log_e("Report suite break%s", moss_newline);
				break;
			}
			continue;
		}
		total++;
		switch(case_runner->flag_result) {
		case moss_unitest_flag_result_pass:
			pass++;
			break;
		case moss_unitest_flag_result_failed:
			failed++;
			break;
		case moss_unitest_flag_result_failed_suite:
			failed++;
			break;
		case moss_unitest_flag_result_prerequisite:
			failed++;
			failed_prereq++;
			break;
		default:
			failed++;
			break;
		}
		if (case_runner->flag_result == moss_unitest_flag_result_failed ||
				case_runner->flag_result == moss_unitest_flag_result_failed_suite) {
			report_log_d("Report result %s, test case[%s], #%d in suite[%s]%s"
					"  Cause: %s%s",
					MOSS_UNITEST_RESULT_STR(case_runner->flag_result, "UNKNOWN"),
					case_runner->name, total, suite->runner.name, moss_newline,
					(case_runner->cause ? case_runner->cause : "UNKNOWN"), moss_newline);
		} else {
			report_log_d("Report result %s, test case[%s], #%d in suite[%s]%s",
					MOSS_UNITEST_RESULT_STR(case_runner->flag_result, "UNKNOWN"),
					case_runner->name, total, suite->runner.name, moss_newline);
		}
	}

	report_log_d("%s result %s, test suite[%s]%s"
			"  Summary test cases PASS: %d, FAILED: %d(PREREQUISITE: %d), TOTAL: %d%s",
			(r != 0 ? "Report(incomplete)" : "Report"),
			MOSS_UNITEST_RESULT_STR(suite->runner.flag_result, "UNKNOWN"),
			suite->runner.name, moss_newline,
			pass, failed, failed_prereq, total, moss_newline);

	if (report_runner) {
		report_runner->total += total;
		report_runner->pass += pass;
		report_runner->failed += failed;
		report_runner->failed_prereq += failed_prereq;
	}
	return r;
}
