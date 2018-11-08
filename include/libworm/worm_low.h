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
