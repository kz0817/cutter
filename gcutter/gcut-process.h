/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008-2011  Kouhei Sutou <kou@clear-code.com>
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

#ifndef __GCUT_PROCESS_H__
#define __GCUT_PROCESS_H__

#include <glib-object.h>
#include <cutter/cut-features.h>
#include <gcutter/gcut-event-loop.h>

G_BEGIN_DECLS

/**
 * SECTION: gcut-process
 * @title: External command
 * @short_description: Convenience API for using external
 * command.
 *
 * #GCutProcess encapsulates external command execution,
 * communication and termination. #GCutProcess reports an error
 * as #GError. It can be asserted easily by
 * gcut_assert_error().
 *
 * External command is specified to constructor like
 * gcut_process_new(), gcut_process_new_strings() and so
 * on. External command isn't run at the
 * time. gcut_process_run() runs specified external command.
 *
 * Standard/Error outputs of external command are passed by
 * #GCutProcess::output-received/#GCutProcess::error-received signals
 * or #GIOChannel returned by
 * gcut_process_get_output()/gcut_process_get_error().
 * gcut_process_write() writes a chunk to standard input of
 * external command.
 *
 * To wait external command finished, gcut_process_wait() can be
 * used. It accepts timeout to avoid infinite waiting.
 *
 * e.g.:
 * |[
 * static GString *output_string;
 * static GCutProcess *process;
 *
 * void
 * cut_setup (void)
 * {
 *     output_string = g_string_new(NULL);
 *     process = NULL;
 * }
 *
 * void
 * cut_teardown (void)
 * {
 *     if (output_string)
 *         g_string_free(output_string, TRUE);
 *     if (process)
 *         g_object_unref(process);
 * }
 *
 * static void
 * cb_output_received (GCutProcess *process, const gchar *chunk, gsize size,
 *                     gpointer user_data)
 * {
 *     g_string_append_len(output_string, chunk, size);
 * }
 *
 * void
 * test_echo (void)
 * {
 *     GError *error = NULL;
 *
 *     process = gcut_process_new("echo", "XXX", NULL);
 *     g_signal_connect(process, "receive-output",
 *                      G_CALLBACK(cb_output_received), NULL);
 *
 *     gcut_process_run(process, &error);
 *     gcut_assert_error(error);
 *
 *     gcut_process_wait(process, 1000, &error);
 *     gcut_assert_error(error);
 *     cut_assert_equal_string("XXX\n", output_string->str);
 * }
 * ]|
 *
 * Since: 1.1.5
 */

#define GCUT_TYPE_PROCESS            (gcut_process_get_type ())
#define GCUT_PROCESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCUT_TYPE_PROCESS, GCutProcess))
#define GCUT_PROCESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GCUT_TYPE_PROCESS, GCutProcessClass))
#define GCUT_IS_PROCESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCUT_TYPE_PROCESS))
#define GCUT_IS_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCUT_TYPE_PROCESS))
#define GCUT_PROCESS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GCUT_TYPE_PROCESS, GCutProcessClass))

#define GCUT_PROCESS_ERROR           (gcut_process_error_quark())

typedef struct _GCutProcess      GCutProcess;
typedef struct _GCutProcessClass GCutProcessClass;

struct _GCutProcess
{
    GObject object;
};

struct _GCutProcessClass
{
    GObjectClass parent_class;

    void (*output_received) (GCutProcess *process,
                             const gchar *chunk,
                             gsize        size);
    void (*error_received)  (GCutProcess *process,
                             const gchar *chunk,
                             gsize        size);
    void (*reaped)          (GCutProcess *process,
                             gint         status);
    void (*error)           (GCutProcess *process,
                             GError      *error);
};

/**
 * GCutProcessError:
 * @GCUT_PROCESS_ERROR_COMMAND_LINE: Command line related error.
 * @GCUT_PROCESS_ERROR_IO_ERROR: IO error.
 * @GCUT_PROCESS_ERROR_ALREADY_RUNNING: External command is already running.
 * @GCUT_PROCESS_ERROR_NOT_RUNNING: External command isn't running.
 * @GCUT_PROCESS_ERROR_INVALID_OBJECT: Invalid #GCutProcess object is passed.
 * @GCUT_PROCESS_ERROR_INVALID_SIGNAL: Invalid signal is passed.
 * @GCUT_PROCESS_ERROR_PERMISSION_DENIED: Permission denied.
 * @GCUT_PROCESS_ERROR_TIMEOUT: Timeout.
 *
 * Error codes returned by #GCutProcess related operations.
 *
 * Since: 1.1.5
 */
typedef enum
{
    GCUT_PROCESS_ERROR_COMMAND_LINE,
    GCUT_PROCESS_ERROR_IO_ERROR,
    GCUT_PROCESS_ERROR_ALREADY_RUNNING,
    GCUT_PROCESS_ERROR_NOT_RUNNING,
    GCUT_PROCESS_ERROR_INVALID_OBJECT,
    GCUT_PROCESS_ERROR_INVALID_SIGNAL,
    GCUT_PROCESS_ERROR_PERMISSION_DENIED,
    GCUT_PROCESS_ERROR_TIMEOUT
} GCutProcessError;

GQuark       gcut_process_error_quark (void);

GType        gcut_process_get_type    (void) G_GNUC_CONST;

/**
 * gcut_process_new:
 * @command: the external command name to be ran
 * @...: the arguments for @command
 *
 * Creates a new #GCutProcess object that runs @command.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess *gcut_process_new        (const gchar  *command,
                                      ...) G_GNUC_NULL_TERMINATED;

/**
 * gcut_process_new_command_line:
 * @command_line: a command line
 *
 * Creates a new #GCutProcess object that runs @command_line.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess      *gcut_process_new_command_line (const gchar *command_line);

/**
 * gcut_process_new_va_list:
 * @command: the external command name to be ran
 * @args: arguments for @command
 *
 * Creates a new #GCutProcess object that runs @command.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess      *gcut_process_new_va_list   (const gchar  *command,
                                              va_list       args);

/**
 * gcut_process_new_argv:
 * @argc: the number of elements of @argv
 * @argv: the external command name to be ran and arguments
 * of it.
 *
 * Creates a new #GCutProcess object that runs @command.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess      *gcut_process_new_argv  (gint          argc,
                                          gchar       **argv);

/**
 * gcut_process_new_strings:
 * @command: the external command name to be ran and
 * arguments of it. %NULL-terminated.
 *
 * Creates a new #GCutProcess object that runs @command.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess      *gcut_process_new_strings   (const gchar **command);

/**
 * gcut_process_new_array:
 * @command: the external command name to be ran and
 * arguments of it. The #GArray should be zero-terminated.
 *
 * Creates a new #GCutProcess object that runs @command.
 *
 * Returns: a new #GCutProcess.
 *
 * Since: 1.1.5
 */
GCutProcess      *gcut_process_new_array     (GArray       *command);

/**
 * gcut_process_set_flags:
 * @process: a #GCutProcess
 * @flags: the flags to be passed to g_spawn_async_with_pipes().
 *
 * Sets @flags for spawning.
 *
 * Since: 1.1.5
 */
void          gcut_process_set_flags (GCutProcess  *process,
                                      GSpawnFlags   flags);

/**
 * gcut_process_get_flags:
 * @process: a #GCutProcess
 *
 * Gets @flags for spawning.
 *
 * Returns: the flags for spawning.
 *
 * Since: 1.1.5
 */
GSpawnFlags   gcut_process_get_flags (GCutProcess      *process);

/**
 * gcut_process_set_env:
 * @process: a #GCutProcess
 * @name: the first environment name.
 * @...: the value of @name, followed by name and value
 * pairs. %NULL-terminated.
 *
 * Sets environment variable for external command.
 *
 * Since: 1.1.5
 */
void          gcut_process_set_env   (GCutProcess  *process,
                                      const gchar  *name,
                                      ...) G_GNUC_NULL_TERMINATED;

/**
 * gcut_process_get_env:
 * @process: a #GCutProcess
 *
 * Gets environment variable for external command.
 *
 * Returns: a newly-allocated %NULL-terminated environment
 * variables. ("NAME1=VALUE1", "NAME2=VALUE2",
 * ..., %NULL) It should be freed by g_strfreev() when no longer
 * needed.
 *
 * Since: 1.1.5
 */
gchar       **gcut_process_get_env  (GCutProcess  *process);

/**
 * gcut_process_run:
 * @process: a #GCutProcess
 * @error: return location for an error, or %NULL
 *
 * Runs a new external process.
 *
 * Returns: %TRUE on success, otherwise %FALSE
 *
 * Since: 1.1.5
 */
gboolean     gcut_process_run (GCutProcess *process,
                               GError     **error);

/**
 * gcut_process_get_pid:
 * @process: a #GCutProcess
 *
 * Gets the process ID of running external process. If
 * external process isn't running, 0 is returned.
 *
 * Returns: the process ID of running external process if
 * external process is running, otherwise 0.
 *
 * Since: 1.1.5
 */
GPid         gcut_process_get_pid (GCutProcess *process);

/**
 * gcut_process_wait:
 * @process: a #GCutProcess
 * @timeout: the timeout period in milliseconds
 * @error: return location for an error, or %NULL
 *
 * Waits running external process is finished while @timeout
 * milliseconds. If external process isn't finished while
 * @timeout milliseconds, %GCUT_PROCESS_ERROR_TIMEOUT error is
 * set and -1 is returned. If external process isn't
 * running, %GCUT_PROCESS_ERROR_NOT_RUNNING error is set and -1
 * is returned.
 *
 * Returns: an exit status of external process on success,
 * otherwise -1.
 *
 * Since: 1.1.5
 */
gint         gcut_process_wait    (GCutProcess *process,
                                   guint        timeout,
                                   GError     **error);
/**
 * gcut_process_kill:
 * @process: a #GCutProcess
 * @signal_number: the signal number to be sent to external process
 * @error: return location for an error, or %NULL
 *
 * Sends @signal_number signal to external process.
 *
 * Returns: %TRUE on success, otherwise %FALSE
 *
 * Since: 1.1.5
 */
gboolean     gcut_process_kill    (GCutProcess *process,
                                   gint         signal_number,
                                   GError     **error);

/**
 * gcut_process_write:
 * @process: a #GCutProcess
 * @chunk: the data to be wrote
 * @size: the size of @chunk
 * @error: return location for an error, or %NULL
 *
 * Writes @chunk to external process's standard input.
 *
 * Returns: %TRUE on success, otherwise %FALSE
 *
 * Since: 1.1.5
 */
gboolean      gcut_process_write          (GCutProcess  *process,
                                           const gchar  *chunk,
                                           gsize         size,
                                           GError      **error);

/**
 * gcut_process_flush:
 * @process: a #GCutProcess
 * @error: return location for an error, or %NULL
 *
 * Flush buffered external process's standard input.
 *
 * Returns: the status of the operation: One of
 *          %G_IO_STATUS_NORMAL, %G_IO_STATUS_AGAIN, or
 *          %G_IO_STATUS_ERROR.
 *
 * Since: 1.1.5
 */
GIOStatus     gcut_process_flush          (GCutProcess  *process,
                                           GError      **error);

/**
 * gcut_process_get_output_string:
 * @process: a #GCutProcess
 *
 * Returns: a #GString that has all result of standard
 * output of external process.
 *
 * Since: 1.1.5
 */
GString     *gcut_process_get_output_string
                                  (GCutProcess *process);

/**
 * gcut_process_get_error_string:
 * @process: a #GCutProcess
 *
 * Returns: a #GString that has all result of standard
 * error of external process.
 *
 * Since: 1.1.5
 */
GString     *gcut_process_get_error_string
                                  (GCutProcess *process);

/**
 * gcut_process_get_input_channel:
 * @process: a #GCutProcess
 *
 * Gets a #GIOChannel connected with standard input of
 * external process.
 *
 * Returns: a #GIOChannel if external process is running,
 * otherwise %NULL.
 *
 * Since: 1.1.5
 */
GIOChannel *gcut_process_get_input_channel (GCutProcess  *process);

/**
 * gcut_process_get_output_channel:
 * @process: a #GCutProcess
 *
 * Gets a #GIOChannel connected with standard output of
 * external process.
 *
 * Returns: a #GIOChannel if external process is running,
 * otherwise %NULL.
 *
 * Since: 1.1.5
 */
GIOChannel *gcut_process_get_output_channel (GCutProcess *process);

/**
 * gcut_process_get_error_channel:
 * @process: a #GCutProcess
 *
 * Gets a #GIOChannel connected with standard error output
 * of external process.
 *
 * Returns: a #GIOChannel if external process is running,
 * otherwise %NULL.
 *
 * Since: 1.1.5
 */
GIOChannel *gcut_process_get_error_channel (GCutProcess *process);

#ifdef CUT_SUPPORT_GIO
#include <gio/gio.h>

/**
 * gcut_process_get_output_stream:
 * @process: a #GCutProcess
 *
 * Gets a #GInputStream connected with standard output of
 * external process.
 *
 * Returns: a #GInputStream if external process is running,
 * otherwise %NULL.
 *
 * Since: 1.1.5
 */
GInputStream *gcut_process_get_output_stream (GCutProcess *process);

/**
 * gcut_process_get_error_stream:
 * @process: a #GCutProcess
 *
 * Gets a #GInputStream connected with standard error output
 * of external process.
 *
 * Returns: a #GInputStream if external process is running,
 * otherwise %NULL.
 *
 * Since: 1.1.5
 */
GInputStream   *gcut_process_get_error_stream (GCutProcess *process);

#endif /* CUT_SUPPORT_GIO */

/**
 * gcut_process_get_forced_termination_wait_time:
 * @process: a #GCutProcess
 *
 * Gets a wait time in milliseconds for forced termination
 * on dispose.
 *
 * Returns: a timeout value for waiting forced terminated
 * external command on dispose.
 *
 * Since: 1.1.5
 */
guint         gcut_process_get_forced_termination_wait_time
                                     (GCutProcess      *process);

/**
 * gcut_process_set_forced_termination_wait_time:
 * @process: a #GCutProcess
 * @timeout: the timeout value in milliseconds
 *
 * Sets a wait time in milliseconds for forced termination
 * on dispose. If @timeout is 0, it doesn't wait
 * termination of external process. The default value is 10.
 *
 * Since: 1.1.5
 */
void          gcut_process_set_forced_termination_wait_time
                                     (GCutProcess      *process,
                                      guint         timeout);

/**
 * gcut_process_get_event_loop:
 * @process: a #GCutProcess
 *
 * Gets a event loop using by the @process.
 *
 * Returns: a #GCutEventLoop.
 *
 * Since: 1.1.6
 */
GCutEventLoop *gcut_process_get_event_loop
                                     (GCutProcess      *process);

/**
 * gcut_process_set_event_loop:
 * @process: a #GCutProcess
 * @loop: the event loop or %NULL
 *
 * Sets a event loop for the @process. If @loop is %NULL,
 * the default GLib event loop will be used.
 *
 * Since: 1.1.6
 */
void          gcut_process_set_event_loop
                                     (GCutProcess      *process,
                                      GCutEventLoop    *loop);

G_END_DECLS

#endif /* __GCUT_PROCESS_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
