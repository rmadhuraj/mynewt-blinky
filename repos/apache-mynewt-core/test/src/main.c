#include "sysinit/sysinit.h"
#include "testutil/testutil.h"
#include "os/os_time.h"
#include "datetime/datetime.h"
//int datetime_parse(const char *input, struct os_timeval *tv, struct os_timezone *tz);
TEST_CASE_DECL(test_datetime_parse_simple);
TEST_SUITE(test_datetime_suite)
{
    test_datetime_parse_simple();
}

TEST_CASE(test_datetime_parse_simple)
{
    struct os_timezone tz;
    struct os_timeval tv;
    int rc;

    /*** Valid input. */

    /* No timezone; UTC implied. */
    rc = datetime_parse("2017-06-28T22:37:59", &tv, &tz);
    TEST_ASSERT_FATAL(rc == 0);
    TEST_ASSERT(tv.tv_sec == 1498689479);
    TEST_ASSERT(tv.tv_usec == 0);
    TEST_ASSERT(tz.tz_minuteswest == 0);
    TEST_ASSERT(tz.tz_dsttime == 0);

    /* PDT timezone. */
    rc = datetime_parse("2013-12-05T02:43:07-07:00", &tv, &tz);
    TEST_ASSERT_FATAL(rc == 0);
    TEST_ASSERT(tv.tv_sec == 1386236587);
    TEST_ASSERT(tv.tv_usec == 0);
    TEST_ASSERT(tz.tz_minuteswest == 420);
    TEST_ASSERT(tz.tz_dsttime == 0);

    /*** Invalid input. */

    /* Nonsense. */
    rc = datetime_parse("abc", &tv, &tz);
    TEST_ASSERT(rc != 0);

    /* Date-only. */
    rc = datetime_parse("2017-01-02", &tv, &tz);
    TEST_ASSERT(rc != 0);

    /* Zero month. */
    rc = datetime_parse("2017-00-28T22:37:59", &tv, &tz);
    TEST_ASSERT(rc != 0);

    /* 13 month. */
    rc = datetime_parse("2017-13-28T22:37:59", &tv, &tz);
    TEST_ASSERT(rc != 0);
}

#if MYNEWT_VAL(SELFTEST)
int
main(int argc, char **argv)
{
    /* Initialize all packages. */
    sysinit();

    test_datetime_suite();

    /* Indicate whether all test cases passed. */
    return tu_any_failed;
}
#endif

