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

#ifndef PR6_SERVER_OBJECT_INTERFACE_H
#define PR6_SERVER_OBJECT_INTERFACE_H

/**
 * @defgroup pr6_server_object_interface PR6 Server Object Interface
 * @ingroup pr05
 * 
 * Interface to PR6 server objects
 *
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include "pr6_encoder_decoder.h"

/* enums **************************************************************/

/** A method adapter shall return the following result */
enum pr6_adapter_result {

    PR6_ADAPTER_SUCCESS = 0,   /**< adapter invocation was successful */
    PR6_ADAPTER_BUFFER,        /**< adapter has filled return buffer */
    PR6_ADAPTER_YIELD          /**< adapter wants to yield to other tasks */
};

/* forward declarations ***********************************************/

struct pr6_server;
struct pr6_server_adapter;

/* typedefs ***********************************************************/

/** Server object interface @see EXAMPLE_ServerObjectInterface */
typedef enum pr6_adapter_result (*pr6_server_object_interface_fn_t)(void *ctxt, const struct pr6_server *, struct pr6_server_adapter *, enum pr6_result *);

/* structs ************************************************************/

/** everything needed to call out to an object */
struct pr6_server_adapter {

    uint16_t objectID;      /**< reference to Object */
    uint8_t methodIndex;    /**< Method within Object */
    const uint8_t *role;    /**< invocation role */
    uint8_t roleSize;       /**< byte length of `role` */
    const uint8_t *in;      /**< invocation argument */
    uint16_t inLen;         /**< byte length of `in` */
    uint8_t *out;           /**< returnValue buffer */
    uint16_t outLen;        /**< byte length of data in `out` */
    uint16_t outMax;        /**< maximum byte length of `out` */
};


/* functions **********************************************************/

#ifdef DOXYGEN
/** An <b>example</b> server object interface
 *
 * @see pr6_server_object_fn_t
 * @note this only an example for purpose of documentation
 *
 * @param[in] r server instance
 * @param[in] arg object information, argument and buffer for returnValue
 * @param[out] out result invocation result
 *
 * @return #pr6_adapter_result result of calling the adapater
 *
 * */
enum pr6_adapter_result EXAMPLE_ServerObjectInterface(void *ctxt, const struct pr6_server *r, struct pr6_server_adapter *arg, enum pr6_result *result);

#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif
