/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008  Kouhei Sutou <kou@cozmixng.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <string.h>

#include "gcut-hash-table.h"
#include "gcut-test-utils.h"

typedef struct _EqualData
{
    GHashTable *target;
    GCompareDataFunc compare_func;
    gpointer user_data;
    gboolean equal;
} EqualData;

static void
equal_hash_table_pair (gpointer key, gpointer value, gpointer user_data)
{
    EqualData *data = user_data;
    gpointer target_key, target_value;

    if (!g_hash_table_lookup_extended((data->target), key,
                                      &target_key, &target_value)) {
        data->equal = FALSE;
        return;
    }

    if (!data->compare_func(value, target_value, data->user_data)) {
        data->equal = FALSE;
        return;
    }
}

gboolean
gcut_hash_table_equal (GHashTable *hash1, GHashTable *hash2,
                       GCompareDataFunc compare_func, gpointer user_data)
{
    EqualData data;

    if (g_hash_table_size(hash1) != g_hash_table_size(hash2))
        return FALSE;

    data.target = hash2;
    data.compare_func = compare_func;
    data.user_data = user_data;
    data.equal = TRUE;
    g_hash_table_foreach(hash1, equal_hash_table_pair, &data);

    return data.equal;
}

typedef struct _InspectData
{
    GString *output;
    GCutInspectFunc key_inspect_func;
    GCutInspectFunc value_inspect_func;
    gpointer user_data;
    guint nth_pair;
    guint total_size;
} InspectData;

static void
inspect_hash_table_pair (gpointer key, gpointer value, gpointer user_data)
{
    InspectData *data = user_data;

    data->nth_pair++;
    data->key_inspect_func(data->output, key, data->user_data);
    g_string_append(data->output, " => ");
    data->value_inspect_func(data->output, value, data->user_data);
    if (data->nth_pair != data->total_size)
        g_string_append(data->output, ", ");
}

gchar *
gcut_hash_table_inspect (GHashTable *hash,
                         GCutInspectFunc key_inspect_func,
                         GCutInspectFunc value_inspect_func,
                         gpointer user_data)
{
    GString *string;
    InspectData data;

    string = g_string_new("{");

    data.output = string;
    data.key_inspect_func = key_inspect_func;
    data.value_inspect_func = value_inspect_func;
    data.user_data = user_data;
    data.nth_pair = 0;
    data.total_size = g_hash_table_size(hash);
    g_hash_table_foreach(hash, inspect_hash_table_pair, &data);

    g_string_append(string, "}");

    return g_string_free(string, FALSE);
}

static gboolean
equal_string (gconstpointer data1, gconstpointer data2, gpointer user_data)
{
    if (data1 == NULL && data2 == NULL)
        return TRUE;

    if (data1 == NULL || data2 == NULL)
        return FALSE;

    return strcmp(data1, data2) == 0;
}

gboolean
gcut_hash_table_string_equal (GHashTable *hash1, GHashTable *hash2)
{
    return gcut_hash_table_equal(hash1, hash2, equal_string, NULL);
}

static void
inspect_string (GString *string, gconstpointer data, gpointer user_data)
{
    const gchar *value = data;

    if (value)
        g_string_append_printf(string, "\"%s\"", value);
    else
        g_string_append(string, "NULL");
}

gchar *
gcut_hash_table_string_string_inspect (GHashTable *hash)
{
    return gcut_hash_table_inspect(hash, inspect_string, inspect_string, NULL);
}

GHashTable *
gcut_hash_table_string_string_new (const gchar *key, ...)
{
    GHashTable *table;
    va_list args;

    table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    va_start(args, key);
    while (key) {
        const gchar *value;

        value = va_arg(args, const gchar *);
        g_hash_table_insert(table, g_strdup(key), g_strdup(value));

        key = va_arg(args, const gchar *);
    }
    va_end(args);

    return table;
}

/*
vi:nowrap:ai:expandtab:sw=4:ts=4
*/
