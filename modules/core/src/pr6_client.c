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
 
/* includes ***********************************************************/

#include "pr6_client.h"
#include "debug_interface.h"

#include <string.h>

/* macros *************************************************************/

#define CLIENT_STATE_MAGIC 0xdeadbeefU

/* static function prototypes *****************************************/

static uint8_t putObjectID(uint16_t in, uint8_t *out);
static uint8_t castResult(uint8_t in, enum pr6_client_result *out);
static uint16_t getCounter(const uint8_t *in);
static uint16_t getObjectID(const uint8_t *in);

/* structure **********************************************************/

/** Complete Response
 *
 * @see `@PR6.RES-Complete`
 *
 * */
struct pr6_res_complete {

    uint16_t resultListSize;    /**< number of results in resultList */
    const uint8_t *resultList;  /**< pointer to resultList buffer */
    uint32_t resultListLen;     /**< byte length of `resultList` */
};

/* functions **********************************************************/

struct pr6_client *PR6_ClientInit(struct pr6_client *r, struct pr6_client_req_res *pool, uint16_t poolMax, bool confirmed, bool breakOnError, pr6_client_result_fn_t result, uint16_t objectID, uint8_t methodIndex, const uint8_t *arg, uint16_t argLen)
{
    ASSERT((r != NULL))
    ASSERT((pool != NULL))
    ASSERT((result != NULL))    

    struct pr6_client *retval = NULL;

    memset(r, 0, sizeof(struct pr6_client));

    if(poolMax > 0U){

        memset(pool, 0, poolMax * sizeof(struct pr6_client_req_res));

        r->state = PR6_CLIENT_STATE_INIT;

        r->list = pool;    
        r->listMax = poolMax;

        r->confirmed = confirmed;
        r->breakOnError = breakOnError;

        r->cbResult = result;

#ifndef NDEBUG
        r->magic = CLIENT_STATE_MAGIC;
#endif

        r->list[r->listSize].objectID = objectID;
        r->list[r->listSize].methodIndex = methodIndex;
        r->list[r->listSize].arg = arg;
        r->list[r->listSize].argLen = argLen;
        r->listSize++;

        r->reqLen = PR6_SIZE_TAG + PR6_SIZE_METHOD_ID + PR6_SizeofVint(argLen) + argLen;
        
        retval = r;

    }
    else{

        DEBUG("poolMax must be at least 1")
    }
    

    return retval;
}

struct pr6_client *PR6_ClientInit_FromOutput(struct pr6_client *r, struct pr6_client_req_res *pool, uint16_t poolMax, pr6_client_result_fn_t result, const uint8_t *in, uint16_t inLen, uint16_t *counter)
{
    ASSERT((r != NULL))
    ASSERT((in != NULL))    
    ASSERT((pool != NULL))
    ASSERT((result != NULL))

    enum loopControl { LOOP, BREAK_LOOP } loopState = BREAK_LOOP;
    struct pr6_client *retval = NULL;
    uint16_t pos = 0U;
    enum pr6_tag tag;
    uint16_t ret;

    memset(r, 0, sizeof(struct pr6_client));

    if(poolMax > 0U){

        memset(pool, 0, poolMax * sizeof(struct pr6_client_req_res));

        r->list = pool;    
        r->listMax = poolMax;        
        r->cbResult = result;

#ifndef NDEBUG
        r->magic = CLIENT_STATE_MAGIC;
#endif
        r->reqLen = inLen;

        if(counter == NULL){
            r->state = PR6_CLIENT_STATE_PENDING;
        }
        else{
            r->state = PR6_CLIENT_STATE_SENT;
        }
        
        if(inLen > 0){

            if(PR6_CastTag(in[pos], &tag) > 0U){

                switch(tag){
                case PR6_METHOD_REQ:
                case PR6_METHOD_BOE_REQ:
                case PR6_METHOD_NC_BOE_REQ:
                case PR6_METHOD_NC_REQ:

                    switch(tag){
                    case PR6_METHOD_NC_BOE_REQ:
                    case PR6_METHOD_NC_REQ:
                        r->confirmed = false;
                        break;
                    case PR6_METHOD_RES:
                    case PR6_METHOD_REQ:
                    case PR6_METHOD_BOE_REQ:
                    default:
                        r->confirmed = true;
                    }
                    
                    switch(tag){                    
                    case PR6_METHOD_BOE_REQ:
                    case PR6_METHOD_NC_BOE_REQ:                    
                        r->breakOnError = true;
                        break;
                    case PR6_METHOD_REQ:
                    case PR6_METHOD_RES:
                    case PR6_METHOD_NC_REQ:
                    default:
                        r->breakOnError = false;
                    }

                    pos += PR6_SIZE_TAG;

                    while(pos < inLen){

                        loopState = BREAK_LOOP;

                        if(r->listMax > r->listSize){

                            if((inLen - pos) >= PR6_SIZE_METHOD_ID){
                                
                                r->list[r->listSize].objectID = getObjectID(&in[pos]);
                                pos += PR6_SIZE_OBJECT_ID;
                                r->list[r->listSize].methodIndex = in[pos];
                                pos += PR6_SIZE_METHOD_INDEX;
                                ret = PR6_GetVint(&in[pos], inLen - pos, &r->list[r->listSize].argLen);

                                if(ret > 0U){

                                    pos += ret;
                                    r->list[r->listSize].arg = &in[pos];

                                    if((inLen - pos) >= r->list[r->listSize].argLen){
                                    
                                        pos += r->list[r->listSize].argLen;
                                        r->listSize++;
                                        loopState = LOOP;                                        
                                    }
                                    else{

                                        DEBUG("input too short for argument")
                                    }
                                }
                                else{

                                    DEBUG("input too short for argument length determinant")
                                }                                                
                            }
                            else{

                                DEBUG("input too short for next method-id")
                            }
                        }
                        else{

                            DEBUG("input exceeds poolMax")
                        }

                        if(loopState == BREAK_LOOP){

                            break;
                        }
                    }

                    if(loopState == LOOP){

                        retval = r;                        
                    }
                    break;

                case PR6_METHOD_RES:
                default:
                    DEBUG("unexpected message")
                    break;
                }
            }
            else{

                DEBUG("cannot debug tag")
            }
        }
        else{

            DEBUG("zero length input")
        }
    }

    return retval;
}
                
struct pr6_client *PR6_ClientInit_AddMethod(struct pr6_client *r, uint16_t objectID, uint8_t methodIndex, const uint8_t *arg, uint16_t argLen)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))

    struct pr6_client *retval = NULL;

    if(r->state == PR6_CLIENT_STATE_INIT){

        if(r->listSize < r->listMax){

            r->list[r->listSize].objectID = objectID;
            r->list[r->listSize].methodIndex = methodIndex;
            r->list[r->listSize].arg = arg;
            r->list[r->listSize].argLen = argLen;
            r->listSize++;
            r->reqLen += PR6_SIZE_METHOD_ID + PR6_SizeofVint(argLen) + argLen;

            retval = r;            
        }
        else{

            DEBUG("cannot add method invocation because pool is exhausted")
        }
    }
    else{

        DEBUG("can only add methods when client state is PR6_CLIENT_STATE_INIT")
    }
    
    return retval;        
}

void PR6_ClientInput(void *ctxt, struct pr6_client *r, const uint8_t *in, uint16_t inLen)
{
    uint16_t i;
    uint16_t ret;
    uint16_t pos = 0U;
    enum pr6_client_result result;
    uint16_t size;
    enum pr6_tag tag;
    enum loopControl { LOOP, BREAK_LOOP } loopState = BREAK_LOOP;
    struct pr6_res_complete m;
    
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    ASSERT((  (inLen == 0U) || ((inLen > 0U) && (in != NULL)) ))

    if(r->state == PR6_CLIENT_STATE_SENT){
    
        if(inLen > 0U){

            if(PR6_CastTag(in[pos], &tag) > 0U){

                pos += PR6_SIZE_TAG;

                if((inLen - pos) >= PR6_SIZE_RES_COUNTER){

                    if(getCounter(&in[pos]) == r->counter){

                        pos += PR6_SIZE_RES_COUNTER;
                        
                        switch(tag){
                        case PR6_METHOD_RES:

                            m.resultListSize = 0U;
                            m.resultList = &in[pos];
                            m.resultListLen = 0U;

                            while(pos < inLen){

                                loopState = BREAK_LOOP;
                                
                                if((inLen - pos) >= PR6_SIZE_RESULT){

                                    if(castResult(in[pos], &result) > 0U){

                                        m.resultListLen++;
                                        m.resultListSize++;
                                        pos += PR6_SIZE_RESULT;

                                        if(result == PR6_CLIENT_RESULT_SUCCESS){

                                            ret = PR6_GetVint(&in[pos], inLen - pos, &size);

                                            if(ret > 0U){                                        

                                                pos += ret;
                                                m.resultListLen += ret;

                                                if((inLen - pos) >= size){

                                                    pos += size;
                                                    m.resultListLen += size;
                                                    loopState = LOOP;
                                                }
                                                else{

                                                    DEBUG("input too short for argument")
                                                }
                                            }
                                            else{

                                                DEBUG("invalid argument length encoding")
                                            }
                                        }
                                        else{

                                            loopState = LOOP;
                                        }
                                    }
                                }
                                else{

                                    DEBUG("input too short for result")
                                }
                            
                                if(loopState == BREAK_LOOP){

                                    break;
                                }
                            }

                            if(loopState == LOOP){                        

                                pos = 0U;

                                if(m.resultListSize <= r->listSize){

                                    for(i=0U; i < r->listSize; i++){

                                        if(i < m.resultListSize){
                                        
                                            /* cast result code back to enumeration */
                                            pos += castResult(m.resultList[pos], &r->list[i].result);
                                            
                                            /* successful result means there will be a returnValue */
                                            if(r->list[i].result == PR6_CLIENT_RESULT_SUCCESS){

                                                pos += PR6_GetVint(&m.resultList[pos], m.resultListLen - pos, &r->list[i].returnValueLen);
                                                r->list[i].returnValue = &m.resultList[pos];
                                                pos += r->list[i].returnValueLen;
                                            }
                                        }
                                        else{

                                            r->list[i].result = PR6_CLIENT_RESULT_MISSING;
                                        }
                                    }

                                    r->state = PR6_CLIENT_STATE_COMPLETE;
                                    r->cbResult(ctxt, r, r->listSize, r->list);
                                }
                                else{

                                    DEBUG("method-list size mismatch")
                                }                                
                            }                        
                            
                            break;

                        case PR6_METHOD_REQ:
                        case PR6_METHOD_BOE_REQ:
                        case PR6_METHOD_NC_REQ:
                        case PR6_METHOD_NC_BOE_REQ:
                        default:
                            DEBUG("unexpected tag")
                            break;
                        }
                    }
                    else{

                        DEBUG("counter does not match r->counter")
                    }
                }
                else{

                    DEBUG("input too short for counter")
                }
            }
        }
        else{

            DEBUG("ignoring an empty message")
        }
    }
}

uint16_t PR6_ClientOutput(struct pr6_client *r, uint8_t *out, uint16_t outMax)
{
    uint16_t outLen = 0U;
    uint16_t i;
    
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    ASSERT((  (outMax == 0U) || ((outMax > 0U) && (out != NULL)) ))

    if(r->state < PR6_CLIENT_STATE_COMPLETE){
    
        if(outMax >= r->reqLen){

            if(r->confirmed == true){

                out[outLen] = (r->breakOnError == true) ? (uint8_t)PR6_METHOD_BOE_REQ : (uint8_t)PR6_METHOD_REQ;
            }
            else{

                out[outLen] = (r->breakOnError == true) ? (uint8_t)PR6_METHOD_NC_BOE_REQ : (uint8_t)PR6_METHOD_NC_REQ;
            }

            outLen += PR6_SIZE_TAG;

            for(i=0; i < r->listSize; i++){

                outLen += putObjectID(r->list[i].objectID, &out[outLen]);

                out[outLen] = r->list[i].methodIndex;
                outLen += PR6_SIZE_METHOD_INDEX;

                outLen += PR6_PutVint(r->list[i].argLen, &out[outLen], outMax - outLen);
                
                memcpy(&out[outLen], r->list[i].arg, (size_t)r->list[i].argLen);
                outLen += r->list[i].argLen;
            }

            ASSERT((outLen == r->reqLen))
            r->state = PR6_CLIENT_STATE_PENDING;
        }                    
    }
    else{

        DEBUG("a complete client cannot send messages")
    }

    return outLen;
}

void PR6_ClientOutputConfirm(void *ctxt, struct pr6_client *r, uint16_t counter)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))

    uint16_t i;

    if((r->state == PR6_CLIENT_STATE_PENDING) || (r->state == PR6_CLIENT_STATE_SENT)){

        r->counter = counter;
        r->state = PR6_CLIENT_STATE_SENT;

        if(r->confirmed == false){

            for(i=0; i < r->listSize; i++){

                r->list[i].result = PR6_CLIENT_RESULT_SUCCESS;
                r->list[i].returnValue = NULL;
                r->list[i].returnValueLen = 0U;                        
            }

            r->state = PR6_CLIENT_STATE_COMPLETE;
            r->cbResult(ctxt, r, r->listSize, r->list);
        }
    }
}

enum pr6_client_state PR6_ClientState(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return r->state;
}

bool PR6_ClientIsConfirmed(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return r->confirmed;
}

bool PR6_ClientIsBreakOnError(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return r->breakOnError;
}

void PR6_ClientTimeout(void *ctxt, struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))

    uint16_t i;

    if(r->state > PR6_CLIENT_STATE_INIT){

        for(i=0U; i < r->listSize; i++){

            r->list[i].result = PR6_CLIENT_RESULT_TIMEOUT;
        }
        r->state = PR6_CLIENT_STATE_COMPLETE;
        r->cbResult(ctxt, r, r->listSize, r->list);
    }
}

void PR6_ClientCancel(void *ctxt, struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))

    uint16_t i;

    if(r->state > PR6_CLIENT_STATE_INIT){

        for(i=0U; i < r->listSize; i++){

            r->list[i].result = PR6_CLIENT_RESULT_CANCEL;
        }
        r->state = PR6_CLIENT_STATE_COMPLETE;
        r->cbResult(ctxt, r, r->listSize, r->list);
    }
}

uint16_t PR6_ClientPeekCounter(const uint8_t *in, uint16_t inLen, uint16_t *counter)
{
    uint16_t pos = 0U;
    enum pr6_tag tag;

    if(inLen >= (PR6_SIZE_TAG + PR6_SIZE_RES_COUNTER)){

        if(PR6_CastTag(*in, &tag) > 0U){

            if(tag == PR6_METHOD_RES){
                
                *counter = getCounter(&in[PR6_SIZE_TAG]);
                pos = (PR6_SIZE_TAG + PR6_SIZE_RES_COUNTER);
            }
        }
    }

    return pos;
}

/* static functions ***************************************************/

static uint8_t putObjectID(uint16_t in, uint8_t *out)
{
    out[0U] = (uint8_t)(in >> 8U);
    out[1U] = (uint8_t)(in);
    return 2U;
}

static uint16_t getCounter(const uint8_t *in)
{
    uint16_t ret = in[0];
    ret <<= 8U;
    ret |= in[1];

    return ret;
}

static uint8_t castResult(uint8_t in, enum pr6_client_result *out)
{
    static const enum pr6_client_result results[] = {
        PR6_CLIENT_RESULT_SUCCESS,
        PR6_CLIENT_RESULT_OBJECT_UNDEFINED,
        PR6_CLIENT_RESULT_METHOD_UNDEFINED,
        PR6_CLIENT_RESULT_ACCESS_DENIED,
        PR6_CLIENT_RESULT_ARGUMENT,
        PR6_CLIENT_RESULT_PERMANENT,
        PR6_CLIENT_RESULT_TEMPORARY
    };
    
    uint8_t retval = 0U;

    if(in < (sizeof(results) / sizeof(*results))){

        *out = results[in];
        retval = 1U;
    }
    else{

        DEBUG("invalid result code")
    }

    return retval;
}

static uint16_t getObjectID(const uint8_t *in)
{
    uint16_t ret = in[0];
    ret <<= 8U;
    ret |= in[1];
    
    return ret;
}
