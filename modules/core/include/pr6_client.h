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
#ifndef PR6_CLIENT_H
#define PR6_CLIENT_H

/**
 * @defgroup pr6_client PR6 Client Interface
 * @ingroup pr05
 *
 * PR6 Client Interface
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif


/* includes ***********************************************************/

#include "pr6_encoder_decoder.h"
#include <stdbool.h>

/* enums **************************************************************/

/** Client instance state */
enum pr6_client_state {

    PR6_CLIENT_STATE_INIT = 0,         /**< Client has been initialised */
    PR6_CLIENT_STATE_REQ,              /**< Client has sent a request */
    PR6_CLIENT_STATE_COMPLETE          /**< Client completed the request */
};

enum pr6_client_result {

    PR6_CLIENT_RESULT_SUCCESS = 0,          /**< successful invocation */
    PR6_CLIENT_RESULT_OBJECT_UNDEFINED,     /**< object does not exist on server */
    PR6_CLIENT_RESULT_METHOD_UNDEFINED,     /**< method does not exist */
    PR6_CLIENT_RESULT_ACCESS_DENIED,        /**< access to method is not allowed */
    PR6_CLIENT_RESULT_ARGUMENT,             /**< argument is invalid for method */
    PR6_CLIENT_RESULT_PERMANENT,            /**< method invocation failed for permanent reason */
    PR6_CLIENT_RESULT_TEMPORARY,            /**< method invocation failed for temporary reason (try again for different result) */
    PR6_CLIENT_RESULT_MISSING,              /**< client expected a result but was not given one */     
    PR6_CLIENT_RESULT_TIMEOUT               /**< client request timed out */
};

/* forward declarations ***********************************************/

struct pr6_client;
struct pr6_client_req_res;

/* typedefs ***********************************************************/

/** Used by client instance to notify user thread that request is complete @see EXAMPLE_ClientResultCallback */
typedef void (*pr6_client_result_fn_t)(struct pr6_client *, uint16_t, const struct pr6_client_req_res *);

/* structs ************************************************************/

/** Data items cached by Client
 *
 * @warning do not access structure members directly
 *
 * */
struct pr6_client {

#ifndef NDEBUG
    uint32_t magic; /**< used to detect uninitialised client instances */
#endif

    enum pr6_client_state state;   /**< stores the state of this client instance */ 

    bool confirmed;     /**< If true Server shall confirm the Request */
    bool breakOnError;  /**< If true Server shall halt invocation of Request on first error */

    struct pr6_client_req_res *list;   /**< list of requests and responses */ 
    uint16_t listSize;                  /**< elements in `list` */
    uint16_t listMax;                   /**< maximum elements allowed in `list` */

    pr6_client_result_fn_t cbResult;         /**< response complete callback */

    uint16_t reqLen;    /**< byte length of request serialised from `list` */
};

/** Binding between Request and Response */
struct pr6_client_req_res {

    uint16_t objectID;      /**< the target object identifier */
    uint8_t methodIndex;    /**< the target method index within scope of the object identifier */

    const uint8_t *arg;     /**< request argument (may be NULL) */
    uint16_t argLen;        /**< byte length of `arg` */

    enum pr6_client_result result;    /**< result of method invocation */

    const uint8_t *returnValue; /**< pointer to return value (may be NULL) */
    uint16_t returnValueLen;    /**< length of returnValue in bytes */
};

/** Initialise a client instance with a Request */
struct pr6_client_init {

    struct pr6_client_req_res *reqResPool;   /**< list element pool */
    uint16_t reqResPoolMax;                   /**< max elements in `reqResPool` */

    bool confirmed;     /**< If true Server shall confirm the Request */
    bool breakOnError;  /**< If true Server shall halt invocation of Request on first error */

    pr6_client_result_fn_t cbResult;       /**< response complete callback */
};

/* functions **********************************************************/

/**
 * Initialise a client instance with a request
 *
 * @param[in] r client instance
 * @param[in] param init parameters
 * @param[in] objectID objectID to reference object
 * @param[in] methodIndex method belonging to object
 * @param[in] arg method invocation argument
 * @param[in] argLen byte length of `arg`
 *
 * @return #pr6_client pointer to client instance
 *
 * @retval !(NULL) client instance initialised
 * @retval NULL client instance could not be initialised
 *
 * */
struct pr6_client *PR6_ClientInit(struct pr6_client *r, const struct pr6_client_init *param, uint16_t objectID, uint8_t methodIndex, const uint8_t *arg, uint16_t argLen);

/**
 * Add another method invocation to an initialised but inactive client instance
 *
 * @param[in] r initialised and inactive client instance
 * @param[in] objectID objectID to reference object
 * @param[in] methodIndex method belonging to object
 * @param[in] arg method invocation argument
 * @param[in] argLen byte length of `arg`
 *
 * @return #pr6_client pointer to client instance
 *
 * @retval !(NULL) method successfully added to client instance
 * @retval NULL method could not be added to client instance
 *
 * */
struct pr6_client *PR6_ClientInit_AddMethod(struct pr6_client *r, uint16_t objectID, uint8_t methodIndex, const uint8_t *arg, uint16_t argLen);

/**
 * Deliver a message to a client instance
 *
 * @note may call `cbResponse()` if complete response has been received
 * @note may call `cbException()` if exception was encountered
 *
 * @param[in] r client instance
 * @param[in] expectedCounter the expected counter value
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * 
 * */ 
void PR6_ClientInput(struct pr6_client *r, uint16_t expectedCounter, const uint8_t *in, uint16_t inLen);

/**
 * Generate an output message
 *
 * @param[in] r client instance
 * @param[out] out output buffer
 * @param[out] outMax maximum byte length of `out`
 *
 * @return bytes successfully written to `out`
 *
 * */
uint16_t PR6_ClientOutput(struct pr6_client *r, uint8_t *out, uint16_t outMax);

/**
 * Return true if Client is in initialised state
 *
 * @param[in] r Client instance
 * @return ClientIsInitialised?
 *
 * */
bool PR6_ClientIsInitialised(const struct pr6_client *r);

/**
 * Return true if Client is in a PR6_CLIENT_STATE_SENT state
 *
 * @param[in] r Client instance
 * @return ClientIsSent?
 *
 * */
bool PR6_ClientIsSent(const struct pr6_client *r);

/**
 * Return true if Client is in a PR6_CLIENT_STATE_COMPLETE state
 *
 * @param[in] r Client instance
 * @return ClientIsComplete?
 *
 * */
bool PR6_ClientIsComplete(const struct pr6_client *r);

/**
 * Return true if client was initialised with a confirmable request
 *
 * @param[in] r client instance
 * @return ClientIsConfirmed?
 * 
 * */
bool PR6_ClientIsConfirmed(const struct pr6_client *r);

/**
 * Return true if client was initialised with a breakOnError request
 *
 * @param[in] r client instance
 * @return ClientIsBreakOnError?
 *
 *
 * */
bool PR6_ClientIsBreakOnError(const struct pr6_client *r);

/**
 * Raise timeout exception
 *
 * @param[in] r client instance
 *
 * */
void PR6_ClientTimeout(struct pr6_client *r);

/**
 * Peek at response to extract the counter value for correlation to a request
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] counter extracted counter value
 *
 * @return number of bytes successfully read from `in`
 *
 * */
uint16_t PR6_ClientPeekCounter(const uint8_t *in, uint16_t inLen, uint16_t *counter);

#ifdef DOXYGEN
/** An <b>example</b> callback function passed to PR6_ClientInit
 *
 * @see pr6_client_result_fn_t
 * @note this only an example for purpose of documentation
 *
 * @param[in] r calling client instance
 * @param[in] listSize number of method invocations in list
 * @param[in] list pointer to method invocation list
 *
 * */
void EXAMPLE_ClientResultCallback(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list);
#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif
