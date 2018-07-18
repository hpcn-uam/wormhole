
#include <libworm/worm_private.h>

extern Worm2EinsConn WH_einsConn;

/** WH_printmsg
 * Sends a message to Zeus and print it.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printmsg(const char *restrict msg)
{
	if (msg)
		return WH_printnmsg(msg, strlen(msg));
	else
		return WH_printnmsg(NULL, 0);
}

/** WH_printnmsg
 * Sends a message to Zeus and print it.
 * The msg can be NULL if length is 0
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printnmsg(const char *restrict msg, const uint32_t length)
{
	enum ctrlMsgType type = PRINTMSG;

	if (tcp_message_ssend(WH_einsConn.socket, &type, sizeof(type))) {
		return 1;
	}

	if (tcp_message_ssend(WH_einsConn.socket, &length, sizeof(length))) {
		return 1;
	}

	if (length) {
		if (tcp_message_ssend(WH_einsConn.socket, msg, length)) {
			return 1;
		}
	}

	return 0;
}

/** WH_printf
 * Sends a message to Zeus and print it.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_printf(const char *restrict format, ...)
{
	va_list args;
	va_list args_cp;

	va_start(args, format);
	va_copy(args_cp, args);
	size_t needed = vsnprintf(NULL, 0, format, args);
	char *out     = malloc(++needed);

	if (!out) {
		va_end(args);
		va_end(args_cp);
		return -1;
	}

	vsnprintf(out, needed, format, args_cp);
	uint8_t ret = WH_printnmsg(out, needed);
	free(out);

	va_end(args);
	va_end(args_cp);
	return ret;
}

/** WH_perror
 * Sends a message to Zeus and print it with the errno value and string.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_perror(const char *restrict format, ...)
{
	va_list args;
	va_list args_cp;
	int errno_prv = errno;

	va_start(args, format);
	va_copy(args_cp, args);
	size_t needed = vsnprintf(NULL, 0, format, args);
	char *msg     = malloc(++needed);

	if (!msg) {
		va_end(args);
		va_end(args_cp);
		return 1;
	}

	vsnprintf(msg, needed, format, args_cp);

	int ret = WH_printf("%s: %s [Errno=%d]", msg, strerror(errno_prv), errno_prv);
	free(msg);

	va_end(args);
	va_end(args_cp);
	return ret;
}

/** WH_sprintf
 * Returns a malloced-string using classic printf format. Must be freeded
 * @return a malloced-string.
 */
char *WH_sprintf(const char *restrict format, ...)
{
	va_list args;
	va_list args_cp;

	va_start(args, format);
	va_copy(args_cp, args);
	size_t needed = vsnprintf(NULL, 0, format, args);
	char *out     = malloc(++needed);

	if (!out) {
		va_end(args);
		va_end(args_cp);
		return NULL;
	}

	vsnprintf(out, needed, format, args_cp);
	va_end(args);
	va_end(args_cp);
	return out;
}

/** WH_abort
 * Sends a message to Zeus, print it and halt all worms.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abort(const char *restrict msg)
{
	if (msg)
		return WH_abortn(msg, strlen(msg));
	else
		return WH_abortn(NULL, 0);
}

/** WH_abortn
 * Sends a message to Zeus, print it and halt all worms.
 * The msg can be NULL if length is 0
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abortn(const char *restrict msg, const uint32_t length)
{
	enum ctrlMsgType type = ABORT;

	fprintf(stderr, "Aborting worm");
	if (length) {
		fprintf(stderr, " cause: %s\n", msg);
	}

	if (tcp_message_ssend(WH_einsConn.socket, &type, sizeof(type))) {
		return 1;
	}

	if (tcp_message_ssend(WH_einsConn.socket, &length, sizeof(length))) {
		return 1;
	}

	if (length) {
		if (tcp_message_ssend(WH_einsConn.socket, msg, length)) {
			return 1;
		}
	}

	fprintf(stderr, "\n");
	fflush(stderr);

	abort();
	return 1;
}

/** WH_abortf
 * Sends a message to Zeus, print it and halt all worms.
 * The msg can be NULL
 * @return 0 if OK, something else if error.
 */
uint8_t WH_abortf(const char *restrict format, ...)
{
	va_list args;
	va_list args_cp;

	va_start(args, format);
	va_copy(args_cp, args);
	size_t needed = vsnprintf(NULL, 0, format, args);
	char *out     = malloc(++needed);

	if (!out) {
		va_end(args);
		va_end(args_cp);
		return -1;
	}

	vsnprintf(out, needed, format, args_cp);
	uint8_t ret = WH_abortn(out, needed);
	free(out);

	va_end(args);
	va_end(args_cp);
	return ret;
}
