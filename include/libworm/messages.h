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