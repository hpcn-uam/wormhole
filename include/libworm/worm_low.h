#ifndef __WORM_LOW_H__
#define __WORM_LOW_H__

/**
 *
 * The functions located here are for public usage, BUT TAKE CARE, as all of the functions are low-level and pour-tested.
 *
 **/

#include <libworm/messages.h>
#include <libworm/worm.h>
#include <libworm/worm_private.h>

#ifdef __cplusplus
extern "C" {
#endif

/** WH_disconnectWorm
 * Disconnect from a worm_id and flushes its contents (or it should...)
 * @Param flow: if Set to 1, recv routes are disconnected
 *              if Set to 2, send routes are disconnected
 *              if Set to 3, both routes are disconnected
 */
void WH_disconnectWorm(uint16_t id, int flow);

/*
 =========================
*/
#ifdef __cplusplus
}
#endif
#endif
