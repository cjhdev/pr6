/* Copyright (c) 2016 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 
 * */
#ifndef PR6_SERVER_H
#define PR6_SERVER_H

/**
 * @defgroup pr6_server PR6 Server Interface
 * @ingroup pr05
 * 
 * PR6 Server Interface
 *
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include "pr6_server_object_interface.h"
#include <stdbool.h>

/* enums **************************************************************/

/** Server instance state */
enum pr6_server_state {

    PR6_SERVER_STATE_IDLE = 0,      /**< no work pending/ready to receive */
    PR6_SERVER_STATE_BEGIN,         /**< begin processing */
    PR6_SERVER_STATE_FIRSTCALL,     /**< call method adapter */
    PR6_SERVER_STATE_CALL,          /**< call method adapter again */
    PR6_SERVER_STATE_YIELD          /**< yield from firstcall/call */
};
    
/* structures *********************************************************/

/** Data items cached by Server
 *
 * @warning structure members are private and should not be accessed directly
 *
 * */
struct pr6_server {

#ifndef NDEBUG
    uint32_t magic; /**< used to detect uninitialised server instances */
#endif

    enum pr6_server_state state;

    bool confirmed;     /**< current METHOD shall be confirmed */
    bool breakOnError;  /**< current METHOD shall break on error */

    const uint8_t *role;
    uint8_t roleSize;
    
    const uint8_t *methodList;  /**< pointer to start of list of methods within req */
    uint16_t methodListLen;     /**< byte length of `methodList` */
    uint16_t methodListPos;     /**< current method position in `methodList` */

    pr6_server_object_interface_fn_t objectInterface;  /**< link to server object interface */

    uint16_t counter;        /**< request counter value */
};

/* functions **********************************************************/

/**
 * Deliver a message to a server instance
 *
 * - Methods called during processing are able to request to yield,
 *   usually on systems that do not support threading. The yield
 *   feature allows the stack to "unwind" and be "resumed" later.
 * 
 * @param[in] r server instance
 * @param[in] objectInterface objects
 * @param[in] role pointer to array of role tokens
 * @param[in] roleSize byte length of `role`
 * @param[in] counter counter from the request message
 * @param[in] in input message buffer
 * @param[in] inLen byte length of `in`
 * @param[in] out pointer to output buffer
 * @param[out] outLen byte length of data in `out`
 * @param[in] outMax maximum byte length of `out` 
 *
 * @return boolean to indicate serverIsYielding?
 *
 * */
bool PR6_ServerInput(void *ctxt, struct pr6_server *r, pr6_server_object_interface_fn_t objectInterface, const uint8_t *role, uint8_t roleSize, uint16_t counter, const uint8_t *in, uint16_t inLen, uint8_t *out, uint16_t *outLen, uint16_t outMax);

/**
 * Resume a server that previously yielded
 *
 * @param[in] r server instance
 * @param[in] out pointer to output buffer
 * @param[out] outLen byte length of data in `out`
 * @param[in] outMax maximum byte length of `out` 
 *
* @return boolean to indicate serverIsYielding?
 * 
 * */
bool PR6_ServerResume(void *ctxt, struct pr6_server *r, uint8_t *out, uint16_t *outLen, uint16_t outMax);

/**
 * Return true if server previously requested to yield
 *
 * @param[in] r server instance
 * 
 * @return bool
 *
 * */
bool PR6_ServerIsYielding(const struct pr6_server *r);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
