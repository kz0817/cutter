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

#ifndef __CUT_DIFFER_H__
#define __CUT_DIFFER_H__

#include <glib-object.h>
#include <cutter/cut-public.h>
#include <cutter/cut-sequence-matcher.h>
#include <cutter/cut-diff-writer.h>

G_BEGIN_DECLS

#define CUT_TYPE_DIFFER            (cut_differ_get_type ())
#define CUT_DIFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUT_TYPE_DIFFER, CutDiffer))
#define CUT_DIFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUT_TYPE_DIFFER, CutDifferClass))
#define CUT_IS_DIFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUT_TYPE_DIFFER))
#define CUT_IS_DIFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CUT_TYPE_DIFFER))
#define CUT_DIFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CUT_TYPE_DIFFER, CutDifferClass))

typedef struct _CutDiffer         CutDiffer;
typedef struct _CutDifferClass    CutDifferClass;

struct _CutDiffer
{
    GObject object;
};

struct _CutDifferClass
{
    GObjectClass parent_class;

    void    (*diff)              (CutDiffer *differ, CutDiffWriter *writer);
    gdouble (*get_best_ratio)    (CutDiffer *differ);
    gdouble (*get_cut_off_ratio) (CutDiffer *differ);
    guint   (*get_initial_context_size)
                                 (CutDiffer *differ);
};

GType         cut_differ_get_type         (void) G_GNUC_CONST;

void          cut_differ_diff             (CutDiffer   *differ,
                                           CutDiffWriter *writer);
void          cut_differ_write_summary    (CutDiffer   *differ,
                                           CutDiffWriter *writer,
                                           const GList *operations);
gdouble       cut_differ_get_best_ratio   (CutDiffer   *differ);
gdouble       cut_differ_get_cut_off_ratio(CutDiffer   *differ);

gchar       **cut_differ_get_from         (CutDiffer   *differ);
gchar       **cut_differ_get_to           (CutDiffer   *differ);
CutSequenceMatcher *
              cut_differ_get_sequence_matcher
                                          (CutDiffer   *differ);
void          cut_differ_set_context_size (CutDiffer   *differ,
                                           guint        context);
guint         cut_differ_get_context_size (CutDiffer   *differ);

gboolean      cut_differ_need_diff        (CutDiffer   *differ);

gboolean      cut_differ_util_is_space_character
                                          (gpointer     data,
                                           gpointer     user_data);
guint         cut_differ_util_compute_width
                                          (const gchar *string,
                                           guint        begin,
                                           guint        end);
gboolean      cut_differ_util_is_same_content
                                          (const GList  *groups);

G_END_DECLS

#endif /* __CUT_DIFFER_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
