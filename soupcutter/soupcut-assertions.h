/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2009  Yuto HAYAMIZU <y.hayamizu@gmail.com>
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

#ifndef __SOUPCUT_ASSERTIONS_H__
#define __SOUPCUT_ASSERTIONS_H__

#include <glib.h>

#include <soupcutter/soupcut-assertions-helper.h>

G_BEGIN_DECLS

/**
 * SECTION: soupcut-assertions
 * @title: Assertions for HTTP client and server
 * @short_description: Checks that your HTTP client/server
 * works as you expect with libsoup support.
 *
 */

/**
 * soupcut_message_assert_equal_content_type:
 * @expected: an expected Content-Type.
 * @message: a #SoupMessage.
 * @...: optional message. See cut_message() for details.
 *
 * Passes if @expected == @message->response_headers->content_type.
 *
 * Since: 1.0.8
 */
#define soupcut_message_assert_equal_content_type(expected, message,    \
                                                  ...) do               \
{                                                                       \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            soupcut_message_assert_equal_content_type_helper(           \
                expected, message, #expected, #message),                \
            __VA_ARGS__),                                               \
        soupcut_message_assert_equal_content_type(expected, message));  \
} while (0)

/**
 * soupcut_client_assert_equal_content_type:
 * @expected: an expected Content-Type.
 * @client: a #SoupCutClient.
 * @...: optional message. See cut_message() for details.
 *
 * Passes if @expected equals to Content-Type of the latest
 * message of @client.
 *
 * Since: 1.0.8
 */
#define soupcut_client_assert_equal_content_type(expected, client,      \
                                                 ...) do                \
{                                                                       \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            soupcut_client_assert_equal_content_type_helper(            \
                expected, client, #expected, #client),                  \
            __VA_ARGS__),                                               \
        soupcut_client_assert_equal_content_type(expected, client));    \
} while (0)


/**
 * soupcut_client_assert_response:
 * @client: a #SoupCutClient.
 * @...: optional message. See cut_message() for details.
 *
 * Passes if status code of @client is 2XX.
 *
 * Since: 1.0.8
 */
#define soupcut_client_assert_response(client, ...) do                  \
{                                                                       \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            soupcut_client_assert_response_helper(client, #client),     \
            __VA_ARGS__),                                               \
        soupcut_client_assert_response(client));                        \
} while (0)

/**
 * soupcut_client_assert_equal_body:
 * @expected: an expected response body.
 * @client: a #SoupCutClient.
 * @...: optional message. See cut_message() for details.
 *
 * Passes if @expected == @client->response_body->data.
 *
 * Since: 1.0.8
 */
#define soupcut_client_assert_equal_body(expected, client, ...) do      \
{                                                                       \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            soupcut_client_assert_equal_body_helper(expected,           \
                                                    client,             \
                                                    #expected,          \
                                                    #client),           \
            __VA_ARGS__),                                               \
        soupcut_client_assert_equal_body(expected, client));            \
} while (0)

/**
 * soupcut_client_assert_match_body:
 * @pattern: the regular expression pattern.
 * @client: a #SoupCutClient.
 * @...: optional message. See cut_message() for details.
 *
 * Passes if @pattern matches the response body of
 * the latest message of @client.
 *
 * Since: 1.0.8
 */
#define soupcut_client_assert_match_body(pattern, client, ...) do       \
{                                                                       \
    cut_trace_with_info_expression(                                     \
        cut_test_with_user_message(                                     \
            soupcut_client_assert_match_body_helper(pattern, client,    \
                                                    #pattern, #client), \
            __VA_ARGS__),                                               \
        soupcut_client_assert_match_body(pattern, client));             \
} while (0)


G_END_DECLS

#endif /* __SOUPCUT_ASSERTIONS_H__ */

/*
vi:nowrap:ai:expandtab:sw=4
*/
