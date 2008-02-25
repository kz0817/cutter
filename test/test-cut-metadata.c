#include "cutter.h"
#include "cut-test.h"
#include "cut-test-result.h"
#include "cut-loader.h"
#include "cut-utils.h"
#include "cut-runner.h"
#include "cuttest-utils.h"
#include "cuttest-assertions.h"

void test_set_metadata (void);
void test_get_bug_id (void);
void test_get_metadata (void);

static CutTest *test;
static CutLoader *test_loader;
static CutTestCase *test_case;

static void
fail_test (void)
{
    cut_assert_equal_string("abc", "xyz");
}

void
setup (void)
{
    gchar *test_path;

    test_path = g_build_filename(cuttest_get_base_dir(),
                                 "metadata_test_dir",
                                 ".libs",
                                 "dummy_metadata_test.so",
                                 NULL);
    test_loader = cut_loader_new(test_path);
    g_free(test_path);

    test_case = NULL;
    test = NULL;
}

void
teardown (void)
{
    if (test)
        g_object_unref(test);
    if (test_loader)
        g_object_unref(test_loader);
    if (test_case)
        g_object_unref(test_case);
}

void
test_set_metadata (void)
{
    test = cut_test_new("fail-test", NULL, fail_test);
    cut_assert(test);
    cut_assert_null(cut_test_get_metadata(test, "category"));
    cut_test_set_metadata(test, "category", "assertion");
    cut_assert_equal_string("assertion",
                            cut_test_get_metadata(test, "category"));
    cut_test_set_metadata(test, "category", "test");
    cut_assert_equal_string("test", cut_test_get_metadata(test, "category"));
}

void
test_get_bug_id (void)
{
    CutTestContainer *container;
    GList *tests;
    const gchar *filter[] = {"/test_get_bug_id/", NULL};

    test_case = cut_loader_load_test_case(test_loader);
    cut_assert(test_case);

    container = CUT_TEST_CONTAINER(test_case);
    tests = (GList *)cut_test_container_filter_children(container, filter);
    cut_assert(tests);

    cut_assert(1, g_list_length(tests));
    cut_assert_equal_string("1234567890", cut_test_get_metadata(CUT_TEST(tests->data), "bug"));
}

void
test_get_metadata (void)
{
    CutTestContainer *container;
    GList *tests;
    const gchar *filter[] = {"/test_metadata/", NULL};

    test_case = cut_loader_load_test_case(test_loader);
    cut_assert(test_case);

    container = CUT_TEST_CONTAINER(test_case);
    tests = (GList *)cut_test_container_filter_children(container, filter);
    cut_assert(tests);

    cut_assert(1, g_list_length(tests));
    cut_assert_equal_string("1234", cut_test_get_metadata(CUT_TEST(tests->data), "bug"));
    cut_assert_equal_string("5678", cut_test_get_metadata(CUT_TEST(tests->data), "priority"));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
