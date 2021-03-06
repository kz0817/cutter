/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008-2010  Kouhei Sutou <kou@clear-code.com>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gcutter.h>
#include <cutter/cut-sequence-matcher.h>

void test_to_indexes_for_string_sequence(void);
void test_to_indexes_for_char_sequence(void);
void test_to_indexes_complex_for_char_sequence(void);
void test_get_longest_match_for_string_sequence(void);
void test_get_longest_match_for_char_sequence(void);
void test_get_longest_match_with_junk_filter_for_string_sequence(void);
void test_get_longest_match_with_junk_filter_for_char_sequence(void);
void test_get_matches_for_string_sequence(void);
void test_get_matches_for_char_sequence(void);
void test_get_matches_complex_for_char_sequence(void);
void test_get_blocks_for_string_sequence(void);
void test_get_blocks_for_char_sequence(void);
void test_get_blocks_complex_for_char_sequence(void);
void test_get_operations_for_string_sequence(void);
void test_get_operations_for_char_sequence(void);
void test_get_grouped_operations(void);
void test_get_ratio_for_string_sequence(void);
void test_get_ratio_for_char_sequence(void);
void test_set_context_size(void);

static CutSequenceMatcher *matcher;
static GList *expected_indexes;
static CutSequenceMatchInfo *actual_info;
static GList *expected_matches;
static GList *expected_operations;
static GList *expected_grouped_operations;
static CutJunkFilterFunc junk_filter_func;
static gpointer junk_filter_func_user_data;

static void
free_matches (GList *matches)
{
    g_list_foreach(matches, (GFunc)cut_sequence_match_info_free, NULL);
    g_list_free(matches);
}

static void
free_operations (GList *operations)
{
    g_list_foreach(operations, (GFunc)cut_sequence_match_operation_free, NULL);
    g_list_free(operations);
}

static void
free_grouped_operations (GList *grouped_operations)
{
    g_list_foreach(grouped_operations, (GFunc)free_operations, NULL);
    g_list_free(grouped_operations);
}

void
cut_setup (void)
{
    matcher = NULL;
    expected_indexes = NULL;
    actual_info = NULL;
    expected_matches = NULL;
    expected_operations = NULL;
    expected_grouped_operations = NULL;
    junk_filter_func = NULL;
    junk_filter_func_user_data = NULL;
}

void
cut_teardown (void)
{
    if (matcher)
        g_object_unref(matcher);

    g_list_free(expected_indexes);

    cut_sequence_match_info_free(actual_info);

    free_matches(expected_matches);

    free_operations(expected_operations);

    free_grouped_operations(expected_grouped_operations);
}

static gboolean
space_string_is_junk (gpointer data, gpointer user_data)
{
    return g_str_equal(data, " ");
}

static gboolean
space_char_is_junk (gpointer data, gpointer user_data)
{
    return GPOINTER_TO_INT(data) == ' ';
}

static CutSequenceMatcher *
string_matcher_new (gchar **from, gchar **to)
{
    if (junk_filter_func)
        return cut_sequence_matcher_string_new_full(from, to,
                                                    junk_filter_func,
                                                    junk_filter_func_user_data);
    else
        return cut_sequence_matcher_string_new(from, to);
}

static CutSequenceMatcher *
char_matcher_new (const gchar *from, const gchar *to)
{
    if (junk_filter_func)
        return cut_sequence_matcher_char_new_full(from, to,
                                                  junk_filter_func,
                                                  junk_filter_func_user_data);
    else
        return cut_sequence_matcher_char_new(from, to);
}

static const gchar *
inspect_string_matcher (gchar **from, gchar **to)
{
    return cut_take_printf("[%s, %s]",
                           cut_inspect_string_array(from),
                           cut_inspect_string_array(to));
}

#define inspect_char_matcher(from, to)          \
    cut_take_printf("[%s, %s]", #from, #to)

static gboolean
equal_match (const CutSequenceMatchInfo *info1,
             const CutSequenceMatchInfo *info2)
{
    return (info1->from_index == info2->from_index &&
            info1->to_index == info2->to_index &&
            info1->size == info2->size);
}

static gboolean
equal_match_with_data (gconstpointer info1, gconstpointer info2)
{
    return equal_match(info1, info2);
}

static gboolean
equal_matches (const GList *matches1, const GList *matches2)
{
    return gcut_list_equal(matches1, matches2, equal_match_with_data);
}

static gchar *
_inspect_match (CutSequenceMatchInfo *info)
{
    return g_strdup_printf("<from: %d, to: %d, size: %d>",
                           info->from_index,
                           info->to_index,
                           info->size);
}

static const gchar *
inspect_match (CutSequenceMatchInfo *info)
{
    return cut_take_string(_inspect_match(info));
}

static void
inspect_match_func (GString *string, gconstpointer data, gpointer user_data)
{
    g_string_append(string, _inspect_match((CutSequenceMatchInfo *)data));
}

static const gchar *
inspect_matches (const GList *matches)
{
    return cut_take_string(gcut_list_inspect(matches, inspect_match_func, NULL));
}

#define cut_assert_to_index(expected, matcher, to_content) do           \
{                                                                       \
    const GList *indexes;                                               \
    indexes = cut_sequence_matcher_get_to_index((matcher),              \
                                                (to_content));          \
    gcut_assert_equal_list_int(expected, indexes);                      \
} while (0)

void
test_to_indexes_for_string_sequence (void)
{
    gchar *from[] = {"", NULL};
    gchar *to[] = {"abc def", "abc", "abc def", NULL};

    matcher = string_matcher_new(from, to);

    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(0));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(2));
    cut_assert_to_index(expected_indexes, matcher, "abc def");
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(1));
    cut_assert_to_index(expected_indexes, matcher, "abc");
}

void
test_to_indexes_for_char_sequence (void)
{
    matcher = char_matcher_new("", "abcad");

    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(0));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(3));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('a'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(1));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('b'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(2));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('c'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(4));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('d'));
}

void
test_to_indexes_complex_for_char_sequence (void)
{
    junk_filter_func = space_char_is_junk;
    matcher = char_matcher_new("",
                               "1 tests, 0 assertions, 0 failures, 1 pendings");

    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(0));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(35));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('1'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(2));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(5));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(16));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('t'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(3));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(14));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(31));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(38));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('e'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(4));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(6));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(12));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(13));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(20));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(32));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(44));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('s'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(7));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(21));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(33));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER(','));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(9));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(23));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('0'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(11));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(26));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('a'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(15));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(30));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('r'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(17));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(27));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(41));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('i'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(18));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('o'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(19));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(39));
    expected_indexes = g_list_append(expected_indexes, GINT_TO_POINTER(42));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('n'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(25));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('f'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(28));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('l'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(29));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('u'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(37));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('p'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(40));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('d'));
    g_list_free(expected_indexes);

    expected_indexes = g_list_append(NULL, GINT_TO_POINTER(43));
    cut_assert_to_index(expected_indexes, matcher, GINT_TO_POINTER('g'));
}

#define cut_assert_longest_match(expected_from_index,                   \
                                 expected_to_index,                     \
                                 expected_size,                         \
                                 sequence_matcher,                      \
                                 sequence_matcher_inspect,              \
                                 from_begin, from_end,                  \
                                 to_begin, to_end) do                   \
{                                                                       \
    CutSequenceMatchInfo info;                                          \
                                                                        \
    info.from_index = (expected_from_index);                            \
    info.to_index = (expected_to_index);                                \
    info.size = (expected_size);                                        \
    matcher = (sequence_matcher);                                       \
    actual_info = cut_sequence_matcher_get_longest_match(matcher,       \
                                                         from_begin,    \
                                                         from_end,      \
                                                         to_begin,      \
                                                         to_end);       \
    if (equal_match(&info, actual_info)) {                              \
        cut_test_pass();                                                \
    } else {                                                            \
        const gchar *message;                                           \
                                                                        \
        message =                                                       \
            cut_take_printf("cut_sequence_matcher_get_longest_match"    \
                            "(%s, %s, %s, %s, %s)\n"                    \
                            " ==\n"                                     \
                            "<from: %s, end: %s, size: %s>\n"           \
                            "expected: %s\n"                            \
                            "  actual: %s",                             \
                            sequence_matcher_inspect,                   \
                            #from_begin,                                \
                            #from_end,                                  \
                            #to_begin,                                  \
                            #to_end,                                    \
                            #expected_from_index,                       \
                            #expected_to_index,                         \
                            #expected_size,                             \
                            inspect_match(&info),                       \
                            inspect_match(actual_info));                \
        cut_test_fail(message);                                         \
    }                                                                   \
                                                                        \
    g_object_unref(matcher);                                            \
    matcher = NULL;                                                     \
    cut_sequence_match_info_free(actual_info);                          \
    actual_info = NULL;                                                 \
} while (0)

#define cut_assert_longest_match_string(expected_begin,                 \
                                        expected_end,                   \
                                        expected_size,                  \
                                        from, to,                       \
                                        from_begin, from_end,           \
                                        to_begin, to_end) do            \
{                                                                       \
    gchar **_from, **_to;                                               \
    CutSequenceMatcher *matcher;                                        \
                                                                        \
    _from = (from);                                                     \
    _to = (to);                                                         \
    matcher = string_matcher_new(_from, _to);                           \
    cut_assert_longest_match(expected_begin, expected_end,              \
                             expected_size,                             \
                             matcher,                                   \
                             inspect_string_matcher(_from, _to),        \
                             from_begin, from_end, to_begin, to_end);   \
} while (0)

void
test_get_longest_match_for_string_sequence (void)
{
    gchar *bcd[] = {"b", "c", "d", NULL};
    gchar *abcdxyz[] = {"a", "b", "c", "d", "x", "y", "z", NULL};
    gchar *ab[] = {"a", "b", NULL};
    gchar *c[] = {"c", NULL};
    gchar *qabxcd[] = {"q", "a", "b", "x", "c", "d", NULL};
    gchar *abycdf[] = {"a", "b", "y", "c", "d", "f", NULL};

    cut_assert_longest_match_string(0, 1, 3, bcd, abcdxyz, 0, 3, 0, 8);
    cut_assert_longest_match_string(1, 2, 2, bcd, abcdxyz, 1, 3, 0, 7);
    cut_assert_longest_match_string(0, 0, 0, ab, c, 0, 2, 0, 1);
    cut_assert_longest_match_string(1, 0, 2, qabxcd, abycdf, 0, 6, 0, 6);
    cut_assert_longest_match_string(4, 3, 2, qabxcd, abycdf, 3, 6, 2, 6);
}

#define cut_assert_longest_match_char(expected_begin,                   \
                                      expected_end,                     \
                                      expected_size,                    \
                                      from, to,                         \
                                      from_begin, from_end,             \
                                      to_begin, to_end)                 \
    cut_assert_longest_match(expected_begin, expected_end,              \
                             expected_size,                             \
                             char_matcher_new(from, to),                \
                             inspect_char_matcher(from, to),            \
                             from_begin,from_end, to_begin, to_end)

void
test_get_longest_match_for_char_sequence (void)
{
    cut_assert_longest_match_char(0, 1, 3, "bcd", "abcdxyz", 0, 3, 0, 8);
    cut_assert_longest_match_char(1, 2, 2, "bcd", "abcdxyz", 1, 3, 0, 7);
    cut_assert_longest_match_char(0, 0, 0, "ab", "c", 0, 2, 0, 1);
    cut_assert_longest_match_char(1, 0, 2, "qabxcd", "abycdf", 0, 6, 0, 6);
    cut_assert_longest_match_char(4, 3, 2, "qabxcd", "abycdf", 3, 6, 2, 6);

    cut_assert_longest_match_char(0, 0, 1, "efg", "eg", 0, 3, 0, 2);
    cut_assert_longest_match_char(2, 1, 1, "efg", "eg", 1, 3, 1, 2);
}

void
test_get_longest_match_with_junk_filter_for_string_sequence (void)
{
    gchar *_abcd[] = {" ", "a", "b", "c", "d", NULL};
    gchar *abcd_abcd[] = {"a", "b", "c", "d", " ", "a", "b", "c", "d", NULL};

    cut_assert_longest_match_string(0, 4, 5, _abcd, abcd_abcd, 0, 5, 0, 9);
    junk_filter_func = space_string_is_junk;
    cut_assert_longest_match_string(1, 0, 4, _abcd, abcd_abcd, 0, 5, 0, 9);
}

void
test_get_longest_match_with_junk_filter_for_char_sequence (void)
{
    cut_assert_longest_match_char(0, 4, 5, " abcd", "abcd abcd", 0, 5, 0, 9);
    junk_filter_func = space_char_is_junk;
    cut_assert_longest_match_char(1, 0, 4, " abcd", "abcd abcd", 0, 5, 0, 9);
}

static GList *
append_match_info (GList *list, gint begin, gint end, gint size)
{
    return g_list_append(list, cut_sequence_match_info_new(begin, end, size));
}

#define cut_assert_equal_matches(expected, actual, ...)                 \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            cut_assert_equal_matches_helper((expected), (actual),       \
                                            #expected, #actual),        \
            __VA_ARGS__),                                               \
        cut_assert_equal_matches(expected, actual, __VA_ARGS__))

static void
cut_assert_equal_matches_helper(const GList *expected, const GList *actual,
                                const gchar *expression_expected,
                                const gchar *expression_actual)
{
    if (equal_matches(expected, actual)) {
        cut_test_pass();
    } else {
        const gchar *inspected_expected, *inspected_actual;

        inspected_expected = inspect_matches(expected);
        inspected_actual = inspect_matches(actual);
        cut_set_expected(inspected_expected);
        cut_set_actual(inspected_actual);
        cut_test_fail(cut_take_printf("<%s> == <%s>",
                                      expression_expected,
                                      expression_actual));
    }
}

static void
cut_assert_matches(const GList *expected_matches,
                   CutSequenceMatcher *sequence_matcher,
                   const gchar *sequence_matcher_inspect)
{
    matcher = sequence_matcher;
    cut_assert_equal_matches(expected_matches,
                             cut_sequence_matcher_get_matches(matcher),
                             cut_message("cut_sequence_matcher_get_matches(%s)",
                                         sequence_matcher_inspect));
    g_object_unref(matcher);
    matcher = NULL;
}

static void
cut_assert_matches_string(GList *expected_matches, gchar **from, gchar **to)
{
    cut_assert_matches(expected_matches,
                       string_matcher_new(from, to),
                       inspect_string_matcher(from, to));
}

void
test_get_matches_for_string_sequence (void)
{
    gchar *abxcd[] = {"a", "b", "x", "c", "d", NULL};
    gchar *abcd[] = {"a", "b", "c", "d", NULL};
    gchar *qabxcd[] = {"q", "a", "b", "x", "c", "d", NULL};
    gchar *abycdf[] = {"a", "b", "y", "c", "d", "f", NULL};
    gchar *efg[] = {"e", "f", "g", NULL};
    gchar *eg[] = {"e", "g", NULL};

    expected_matches = append_match_info(NULL, 0, 0, 2);
    expected_matches = append_match_info(expected_matches, 3, 2, 2);
    cut_assert_matches_string(expected_matches, abxcd, abcd);
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 1, 0, 2);
    expected_matches = append_match_info(expected_matches, 4, 3, 2);
    cut_assert_matches_string(expected_matches, qabxcd, abycdf);
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 0, 0, 1);
    expected_matches = append_match_info(expected_matches, 2, 1, 1);
    cut_assert_matches_string(expected_matches, efg, eg);
}

#define cut_assert_matches_char(expected_matches, from, to)             \
    cut_assert_matches(expected_matches,                                \
                       char_matcher_new(from, to),                      \
                       inspect_char_matcher(from, to))

void
test_get_matches_for_char_sequence (void)
{
    expected_matches = append_match_info(NULL, 0, 0, 2);
    expected_matches = append_match_info(expected_matches, 3, 2, 2);
    cut_assert_matches_char(expected_matches, "abxcd", "abcd");
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 1, 0, 2);
    expected_matches = append_match_info(expected_matches, 4, 3, 2);
    cut_assert_matches_char(expected_matches, "qabxcd", "abycdf");
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 0, 0, 1);
    expected_matches = append_match_info(expected_matches, 2, 1, 1);
    cut_assert_matches_char(expected_matches, "efg", "eg");
}

void
test_get_matches_complex_for_char_sequence (void)
{
    expected_matches = append_match_info(NULL, 0, 0, 23);
    expected_matches = append_match_info(expected_matches, 24, 24, 11);
    expected_matches = append_match_info(expected_matches, 36, 36, 9);
    cut_assert_matches_char(expected_matches,
                            "1 tests, 0 assertions, 1 failures, 0 pendings",
                            "1 tests, 0 assertions, 0 failures, 1 pendings");
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 0, 0, 1);
    expected_matches = append_match_info(expected_matches, 1, 1, 8);
    expected_matches = append_match_info(expected_matches, 9, 9, 1);
    expected_matches = append_match_info(expected_matches, 10, 10, 13);
    expected_matches = append_match_info(expected_matches, 24, 24, 11);
    expected_matches = append_match_info(expected_matches, 36, 36, 9);
    junk_filter_func = space_char_is_junk;
    cut_assert_matches_char(expected_matches,
                            "1 tests, 0 assertions, 1 failures, 0 pendings",
                            "1 tests, 0 assertions, 0 failures, 1 pendings");
}

#define cut_assert_blocks(expected_matches,                             \
                          sequence_matcher,                             \
                          sequence_matcher_inspect) do                  \
{                                                                       \
    matcher = (sequence_matcher);                                       \
    cut_assert_equal_matches(                                           \
        expected_matches,                                               \
        cut_sequence_matcher_get_blocks(matcher),                       \
        cut_message("cut_sequence_matcher_get_blocks(%s)",              \
                    sequence_matcher_inspect));                         \
    g_object_unref(matcher);                                            \
    matcher = NULL;                                                     \
} while (0)

#define cut_assert_blocks_string(expected_matches, from, to) do         \
{                                                                       \
    gchar **_from, **_to;                                               \
    _from = (from);                                                     \
    _to = (to);                                                         \
                                                                        \
    cut_assert_blocks(expected_matches,                                 \
                      string_matcher_new(_from, _to),                   \
                      inspect_string_matcher(_from, _to));              \
} while (0)

void
test_get_blocks_for_string_sequence (void)
{
    gchar *abxcd[] = {"a", "b", "x", "c", "d", NULL};
    gchar *abcd[] = {"a", "b", "c", "d", NULL};
    gchar *qabxcd[] = {"q", "a", "b", "x", "c", "d", NULL};
    gchar *abycdf[] = {"a", "b", "y", "c", "d", "f", NULL};
    gchar *efg[] = {"e", "f", "g", NULL};
    gchar *eg[] = {"e", "g", NULL};

    expected_matches = append_match_info(NULL, 0, 0, 2);
    expected_matches = append_match_info(expected_matches, 3, 2, 2);
    expected_matches = append_match_info(expected_matches, 5, 4, 0);
    cut_assert_blocks_string(expected_matches, abxcd, abcd);
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 1, 0, 2);
    expected_matches = append_match_info(expected_matches, 4, 3, 2);
    expected_matches = append_match_info(expected_matches, 6, 6, 0);
    cut_assert_blocks_string(expected_matches, qabxcd, abycdf);
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 0, 0, 1);
    expected_matches = append_match_info(expected_matches, 2, 1, 1);
    expected_matches = append_match_info(expected_matches, 3, 2, 0);
    cut_assert_blocks_string(expected_matches, efg, eg);
}

#define cut_assert_blocks_char(expected_matches, from, to)              \
    cut_assert_blocks(expected_matches,                                 \
                      char_matcher_new(from, to),                       \
                      inspect_char_matcher(from, to))

void
test_get_blocks_for_char_sequence (void)
{
    expected_matches = append_match_info(NULL, 0, 0, 2);
    expected_matches = append_match_info(expected_matches, 3, 2, 2);
    expected_matches = append_match_info(expected_matches, 5, 4, 0);
    cut_assert_blocks_char(expected_matches, "abxcd", "abcd");
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 1, 0, 2);
    expected_matches = append_match_info(expected_matches, 4, 3, 2);
    expected_matches = append_match_info(expected_matches, 6, 6, 0);
    cut_assert_blocks_char(expected_matches, "qabxcd", "abycdf");
    free_matches(expected_matches);

    expected_matches = append_match_info(NULL, 0, 0, 1);
    expected_matches = append_match_info(expected_matches, 2, 1, 1);
    expected_matches = append_match_info(expected_matches, 3, 2, 0);
    cut_assert_blocks_char(expected_matches, "efg", "eg");
}

void
test_get_blocks_complex_for_char_sequence (void)
{
    expected_matches = append_match_info(NULL, 0, 0, 23);
    expected_matches = append_match_info(expected_matches, 24, 24, 11);
    expected_matches = append_match_info(expected_matches, 36, 36, 9);
    expected_matches = append_match_info(expected_matches, 45, 45, 0);
    cut_assert_blocks_char(expected_matches,
                           "1 tests, 0 assertions, 1 failures, 0 pendings",
                           "1 tests, 0 assertions, 0 failures, 1 pendings");

    junk_filter_func = space_char_is_junk;
    cut_assert_blocks_char(expected_matches,
                           "1 tests, 0 assertions, 1 failures, 0 pendings",
                           "1 tests, 0 assertions, 0 failures, 1 pendings");
}

static gboolean
equal_operation_with_data (gconstpointer data1, gconstpointer data2)
{
    const CutSequenceMatchOperation *operation1, *operation2;

    operation1 = data1;
    operation2 = data2;
    return (operation1->type == operation2->type &&
            operation1->from_begin == operation2->from_begin &&
            operation1->from_end == operation2->from_end &&
            operation1->to_begin == operation2->to_begin &&
            operation1->to_end == operation2->to_end);
}

static gboolean
equal_operations (const GList *operations1, const GList *operations2)
{
    return gcut_list_equal(operations1, operations2,
                           equal_operation_with_data);
}

static const gchar *
operation_type_to_string (CutSequenceMatchOperationType type)
{
    switch (type) {
      case CUT_SEQUENCE_MATCH_OPERATION_EQUAL:
        return "EQUAL";
      case CUT_SEQUENCE_MATCH_OPERATION_INSERT:
        return "INSERT";
      case CUT_SEQUENCE_MATCH_OPERATION_DELETE:
        return "DELETE";
      case CUT_SEQUENCE_MATCH_OPERATION_REPLACE:
        return "REPLACE";
    }

    return "UNKNOWN";
}

static void
inspect_operation_func (GString *string, gconstpointer data, gpointer user_data)
{
    const CutSequenceMatchOperation *operation = data;

    g_string_append_printf(string,
                           "<type: %s, from_begin: %u, from_end: %u, "
                           "to_begin: %u, to_end: %u>",
                           operation_type_to_string(operation->type),
                           operation->from_begin,
                           operation->from_end,
                           operation->to_begin,
                           operation->to_end);
}

static const gchar *
inspect_operations (const GList *operations)
{
    return cut_take_string(gcut_list_inspect(operations,
                                             inspect_operation_func, NULL));
}

#define cut_assert_operations(expected,                                 \
                              sequence_matcher,                         \
                              sequence_matcher_inspect) do              \
{                                                                       \
    const GList *_expected, *_actual;                                   \
                                                                        \
    _expected = (expected);                                             \
    matcher = (sequence_matcher);                                       \
    _actual = cut_sequence_matcher_get_operations(matcher);             \
                                                                        \
    if (equal_operations(_expected, _actual)) {                         \
        cut_test_pass();                                                \
    } else {                                                            \
        const gchar *inspected_expected, *inspected_actual;             \
                                                                        \
        inspected_expected = inspect_operations(_expected);             \
        inspected_actual = inspect_operations(_actual);                 \
        cut_set_message("cut_sequence_matcher_get_operations(%s)",      \
                        sequence_matcher_inspect);                      \
        cut_test_fail(cut_take_printf("expected: <%s>\n"                \
                                      "  actual: <%s>",                 \
                                      inspected_expected,               \
                                      inspected_actual));               \
    }                                                                   \
    g_object_unref(matcher);                                            \
    matcher = NULL;                                                     \
} while (0)

#define cut_assert_operations_string(expected_operations, from, to) do  \
{                                                                       \
    gchar **_from, **_to;                                               \
    _from = (from);                                                     \
    _to = (to);                                                         \
                                                                        \
    cut_assert_operations(expected_operations,                          \
                          string_matcher_new(_from, _to),               \
                          inspect_string_matcher(_from, _to));          \
} while (0)

static GList *
_append_operation (GList *list, CutSequenceMatchOperationType type,
                   gint from_begin, gint from_end, gint to_begin, gint to_end)
{
    return g_list_append(list,
                         cut_sequence_match_operation_new(type,
                                                          from_begin,
                                                          from_end,
                                                          to_begin,
                                                          to_end));
}

#define append_operation(list, type, from_begin, from_end,              \
                         to_begin, to_end)                              \
    _append_operation(list, CUT_SEQUENCE_MATCH_OPERATION_ ## type,      \
                      from_begin, from_end, to_begin, to_end)

void
test_get_operations_for_string_sequence (void)
{
    gchar *empty[] = {NULL};
    gchar *qabxcd[] = {"q", "a", "b", "x", "c", "d", NULL};
    gchar *abycdf[] = {"a", "b", "y", "c", "d", "f", NULL};
    gchar *summary_1010[] = {
        "1", " ", "t", "e", "s", "t", "s", ",", " ",
        "0", " ", "a", "s", "s", "e", "r", "t", "i", "o", "n", "s", ",", " ",
        "1", " ", "f", "a", "i", "l", "u", "r", "e", "s", ",", " ",
        "0", " ", "p", "e", "n", "d", "i", "n", "g", "s",
        NULL};
    gchar *summary_1001[] = {
        "1", " ", "t", "e", "s", "t", "s", ",", " ",
        "0", " ", "a", "s", "s", "e", "r", "t", "i", "o", "n", "s", ",", " ",
        "0", " ", "f", "a", "i", "l", "u", "r", "e", "s", ",", " ",
        "1", " ", "p", "e", "n", "d", "i", "n", "g", "s",
        NULL};

    cut_assert_operations_string(NULL, empty, empty);

    expected_operations = append_operation(NULL, DELETE, 0, 1, 0, 0);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           1, 3, 0, 2);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           3, 4, 2, 3);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           4, 6, 3, 5);
    expected_operations = append_operation(expected_operations, INSERT,
                                           6, 6, 5, 6);
    cut_assert_operations_string(expected_operations, qabxcd, abycdf);
    free_operations(expected_operations);

    expected_operations = append_operation(NULL, EQUAL, 0, 23, 0, 23);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           23, 24, 23, 24);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           24, 35, 24, 35);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           35, 36, 35, 36);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           36, 45, 36, 45);
    cut_assert_operations_string(expected_operations,
                                 summary_1010, summary_1001);

    junk_filter_func = space_string_is_junk;
    cut_assert_operations_string(expected_operations,
                                 summary_1010, summary_1001);
}

#define cut_assert_operations_char(expected_operations, from, to)       \
    cut_assert_operations(expected_operations,                          \
                          char_matcher_new(from, to),                   \
                          inspect_char_matcher(from, to))

void
test_get_operations_for_char_sequence (void)
{
    cut_assert_operations_char(NULL, "", "");

    expected_operations = append_operation(NULL, DELETE, 0, 1, 0, 0);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           1, 3, 0, 2);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           3, 4, 2, 3);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           4, 6, 3, 5);
    expected_operations = append_operation(expected_operations, INSERT,
                                           6, 6, 5, 6);
    cut_assert_operations_char(expected_operations, "qabxcd", "abycdf");
    free_operations(expected_operations);

    expected_operations = append_operation(NULL, EQUAL, 0, 23, 0, 23);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           23, 24, 23, 24);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           24, 35, 24, 35);
    expected_operations = append_operation(expected_operations, REPLACE,
                                           35, 36, 35, 36);
    expected_operations = append_operation(expected_operations, EQUAL,
                                           36, 45, 36, 45);
    cut_assert_operations_char(expected_operations,
                               "1 tests, 0 assertions, 1 failures, 0 pendings",
                               "1 tests, 0 assertions, 0 failures, 1 pendings");

    junk_filter_func = space_char_is_junk;
    cut_assert_operations_char(expected_operations,
                               "1 tests, 0 assertions, 1 failures, 0 pendings",
                               "1 tests, 0 assertions, 0 failures, 1 pendings");
}

static gboolean
equal_grouped_operation_with_data (gconstpointer data1, gconstpointer data2)
{
    const GList *grouped_operation1, *grouped_operation2;

    grouped_operation1 = data1;
    grouped_operation2 = data2;
    return equal_operations(grouped_operation1, grouped_operation2);
}

static gboolean
equal_grouped_operations (const GList *grouped_operations1,
                          const GList *grouped_operations2)
{
    return gcut_list_equal(grouped_operations1, grouped_operations2,
                           equal_grouped_operation_with_data);
}

static void
inspect_grouped_operation_func (GString *string, gconstpointer data,
                                gpointer user_data)
{
    const GList *grouped_operation = data;

    g_string_append(string, inspect_operations(grouped_operation));
}

static const gchar *
inspect_grouped_operations (const GList *grouped_operations)
{
    return cut_take_string(gcut_list_inspect(grouped_operations,
                                             inspect_grouped_operation_func,
                                             NULL));
}

static void
cut_assert_grouped_operations (const GList *expected,
                               CutSequenceMatcher *sequence_matcher,
                               const gchar *sequence_matcher_inspect)
{
    const GList *actual;

    matcher = sequence_matcher;
    actual = cut_sequence_matcher_get_grouped_operations(matcher);

    if (equal_grouped_operations(expected, actual)) {
        cut_test_pass();
    } else {
        cut_set_expected(inspect_grouped_operations(expected));
        cut_set_actual(inspect_grouped_operations(actual));
        cut_fail("cut_sequence_matcher_get_grouped_operations(%s)",
                 sequence_matcher_inspect);
    }
    g_object_unref(matcher);
    matcher = NULL;
}

static void
cut_assert_grouped_operations_string (const GList *expected_grouped_operations,
                                      gchar **from, gchar **to)
{
    cut_assert_grouped_operations(expected_grouped_operations,
                                  string_matcher_new(from, to),
                                  inspect_string_matcher(from, to));
}

void
test_get_grouped_operations (void)
{
    GList *grouped_operation = NULL;
    gchar *empty[] = {NULL};
    gchar *abc[] = {"a", "b", "c", NULL};
    gchar *alnum[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
        NULL};
    gchar *alnum_garbage[] = {
        "1", "i", "3", "4", "5", "6", "7", "8", "9",
        "a", "b", "cX", "d", "h", "iX", "j", "k",
        NULL};

    grouped_operation = append_operation(NULL, EQUAL, 0, 0, 0, 0);
    expected_grouped_operations = g_list_append(NULL, grouped_operation);
    cut_assert_grouped_operations_string(expected_grouped_operations,
                                         empty, empty);
    free_grouped_operations(expected_grouped_operations);


    grouped_operation = append_operation(NULL, EQUAL, 0, 3, 0, 3);
    expected_grouped_operations = g_list_append(NULL, grouped_operation);
    cut_assert_grouped_operations_string(expected_grouped_operations,
                                         abc, abc);
    free_grouped_operations(expected_grouped_operations);


    grouped_operation = append_operation(NULL, EQUAL, 0, 1, 0, 1);
    grouped_operation = append_operation(grouped_operation, REPLACE, 1, 2, 1, 2);
    grouped_operation = append_operation(grouped_operation, EQUAL, 2, 5, 2, 5);
    expected_grouped_operations = g_list_append(NULL, grouped_operation);

    grouped_operation = append_operation(NULL, EQUAL, 8, 11, 8, 11);
    grouped_operation = append_operation(grouped_operation,
                                         REPLACE, 11, 12, 11, 12);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 12, 13, 12, 13);
    grouped_operation = append_operation(grouped_operation,
                                         DELETE, 13, 16, 13, 13);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 16, 17, 13, 14);
    grouped_operation = append_operation(grouped_operation,
                                         REPLACE, 17, 18, 14, 15);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 18, 20, 15, 17);
    expected_grouped_operations = g_list_append(expected_grouped_operations,
                                                grouped_operation);

    cut_assert_grouped_operations_string(expected_grouped_operations,
                                         alnum, alnum_garbage);
}

static void
cut_assert_ratio(gdouble expected, CutSequenceMatcher *sequence_matcher,
                 const gchar *sequence_matcher_inspect)
{
    matcher = sequence_matcher;
    cut_assert_equal_double(expected,
                            0.01,
                            cut_sequence_matcher_get_ratio(matcher),
                            cut_message("cut_sequence_matcher_get_ratio(%s)",
                                        sequence_matcher_inspect));
    g_object_unref(matcher);
    matcher = NULL;
}

static void
cut_assert_ratio_string (gdouble expected, gchar **from, gchar **to)
{
    cut_assert_ratio(expected,
                     string_matcher_new(from, to),
                     inspect_string_matcher(from, to));
}

void
test_get_ratio_for_string_sequence (void)
{
    gchar *abcd[] = {"a", "b", "c", "d", NULL};
    gchar *bcde[] = {"b", "c", "d", "e", NULL};
    gchar *efg[] = {"e", "f", "g", NULL};
    gchar *eg[] = {"e", "g", NULL};

    cut_assert_ratio_string(0.75, abcd, bcde);
    cut_assert_ratio_string(0.80, efg, eg);
}

#define cut_assert_ratio_char(expected, from, to)                       \
    cut_assert_ratio(expected,                                          \
                     char_matcher_new(from, to),                        \
                     inspect_char_matcher(from, to))

void
test_get_ratio_for_char_sequence (void)
{
    cut_assert_ratio_char(0.75, "abcd", "bcde");
    cut_assert_ratio_char(0.80, "efg", "eg");
}

void
test_set_context_size (void)
{
    GList *grouped_operation = NULL;
    gchar *alnum[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
        NULL};
    gchar *alnum_garbage[] = {
        "1", "i", "3", "4", "5", "6", "7", "8", "9",
        "a", "b", "cX", "d", "h", "iX", "j", "k",
        NULL};

    grouped_operation = append_operation(NULL, EQUAL, 0, 1, 0, 1);
    grouped_operation = append_operation(grouped_operation, REPLACE, 1, 2, 1, 2);
    grouped_operation = append_operation(grouped_operation, EQUAL, 2, 11, 2, 11);
    grouped_operation = append_operation(grouped_operation,
                                         REPLACE, 11, 12, 11, 12);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 12, 13, 12, 13);
    grouped_operation = append_operation(grouped_operation,
                                         DELETE, 13, 16, 13, 13);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 16, 17, 13, 14);
    grouped_operation = append_operation(grouped_operation,
                                         REPLACE, 17, 18, 14, 15);
    grouped_operation = append_operation(grouped_operation,
                                         EQUAL, 18, 20, 15, 17);
    expected_grouped_operations = g_list_append(expected_grouped_operations,
                                                grouped_operation);

    matcher = string_matcher_new(alnum, alnum_garbage);
    cut_sequence_matcher_set_context_size(matcher, 5);
    cut_assert_grouped_operations(expected_grouped_operations,
                                  matcher,
                                  inspect_string_matcher(alnum, alnum_garbage));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
