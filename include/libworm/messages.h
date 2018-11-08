/* Copyright (c) 2015-2018 Rafael Leira, Paula Roquero, Naudit HPCN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __WORM_MESSAGES_H__
#define __WORM_MESSAGES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libworm/common.h>

/** WH_printmsg
 * Sends a message to Zeus and print it.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printmsg(const char *restrict msg);

/** WH_printnmsg
 * Sends a message to Zeus and print it.
 * The msg can be NULL if length is 0
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printnmsg(const char *restrict msg, const uint32_t length);

/** WH_printf
 * Sends a message to Zeus and print it.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printf(const char *restrict format, ...);

/** WH_perror
 * Sends a message to Zeus and print it with the errno value and string.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_perror(const char *restrict format, ...);

/** WH_abort
 * Sends a message to Zeus, print it and halt all worms.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abort(const char *restrict msg);

/** WH_abortn
 * Sends a message to Zeus, print it and halt all worms.
 * The msg can be NULL if length is 0
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abortn(const char *restrict msg, const uint32_t length);

/** WH_abortf
 * Sends a message to Zeus, print it and halt all worms.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abortf(const char *restrict format, ...);

#ifdef __cplusplus
}
#endif

#endif