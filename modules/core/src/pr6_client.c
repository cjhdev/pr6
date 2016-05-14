/* Copyright (c) 2013-2016 Cameron Harper
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

struct pr6_client *PR6_ClientInit(struct pr6_client *r, const struct pr6_client_init *param, uint16_t objectID, uint8_t methodIndex, const uint8_t *arg, uint16_t argLen)
{
    ASSERT((r != NULL))
    ASSERT((param != NULL))
    ASSERT((param->reqResPool != NULL))
    ASSERT((param->cbResult != NULL))

    struct pr6_client *retval = NULL;

    memset(r, 0, sizeof(struct pr6_client));

    if(param->reqResPoolMax > 0U){

        memset(param->reqResPool, 0, sizeof(*param->reqResPool) * param->reqResPoolMax);

        r->list = param->reqResPool;    
        r->listMax = param->reqResPoolMax;

        r->confirmed = param->confirmed;
        r->breakOnError = param->breakOnError;

        r->state = PR6_CLIENT_STATE_INIT;
        
        r->cbResult = param->cbResult;

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

        DEBUG("param->reqResPoolMax must be at least 1")
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

        DEBUG("cannot add a method invocation to an active client instance")
    }

    return retval;        
}

void PR6_ClientInput(struct pr6_client *r, uint16_t expectedCounter, const uint8_t *in, uint16_t inLen)
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
    
    switch(r->state){
    case PR6_CLIENT_STATE_REQ:

        if(inLen > 0U){

            if(PR6_CastTag(in[pos], &tag) > 0U){

                pos += PR6_SIZE_TAG;

                if((inLen - pos) >= PR6_SIZE_RES_COUNTER){

                    /* expected counter will never be zero, therefore by specifying zero you
                     * are saying you don't care about correlation */
                    if((expectedCounter == 0U) || (getCounter(&in[pos]) == expectedCounter)){

                        /* skip over the counter value since it
                         * is too late to use it to correlate response to
                         * client (request) */
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
                                    r->cbResult(r, r->listSize, r->list);
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

                        DEBUG("counter does not match expectedCounter")
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
        break;

    case PR6_CLIENT_STATE_COMPLETE:
    case PR6_CLIENT_STATE_INIT:
    default:

        DEBUG("ignoring a message while instance in non-receptive state (state = %u)", (uint8_t)r->state)
        break;
    }                
}

uint16_t PR6_ClientOutput(struct pr6_client *r, uint8_t *out, uint16_t outMax)
{
    uint16_t outLen = 0U;
    uint16_t i;
    
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    ASSERT((  (outMax == 0U) || ((outMax > 0U) && (out != NULL)) ))

    switch(r->state){
    case PR6_CLIENT_STATE_INIT:
    case PR6_CLIENT_STATE_REQ:

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

            r->state = PR6_CLIENT_STATE_REQ;

            if(r->confirmed == false){

                for(i=0; i < r->listSize; i++){

                    r->list[i].result = PR6_CLIENT_RESULT_SUCCESS;
                    r->list[i].returnValue = NULL;
                    r->list[i].returnValueLen = 0U;                        
                }

                r->state = PR6_CLIENT_STATE_COMPLETE;
                r->cbResult(r, r->listSize, r->list);
            }
        }                    
        break;

    case PR6_CLIENT_STATE_COMPLETE:    
    default:
        /* no action */
        break;
    }

    return outLen;
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

bool PR6_ClientIsInitialised(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return (r->state == PR6_CLIENT_STATE_INIT) ? true : false;
}

bool PR6_ClientIsSent(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return (r->state == PR6_CLIENT_STATE_REQ) ? true : false;
}

bool PR6_ClientIsComplete(const struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))
    return (r->state == PR6_CLIENT_STATE_COMPLETE) ? true : false;
}

void PR6_ClientTimeout(struct pr6_client *r)
{
    ASSERT(((r != NULL) && (r->magic == CLIENT_STATE_MAGIC)))

    uint16_t i;

    switch(r->state){
    case PR6_CLIENT_STATE_INIT:
    case PR6_CLIENT_STATE_REQ:    
        r->state = PR6_CLIENT_STATE_COMPLETE;
        for(i=0U; i < r->listSize; i++){

            r->list[i].result = PR6_CLIENT_RESULT_TIMEOUT;
        }
        r->cbResult(r, r->listSize, r->list); 
        break;
    case PR6_CLIENT_STATE_COMPLETE:
    default:
        break;
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
