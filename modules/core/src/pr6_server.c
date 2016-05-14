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

#include "pr6_server.h"
#include "debug_interface.h"

#include <string.h>

/* macros *************************************************************/

#define SERVER_STATE_MAGIC 0xdeadbeafU

/* static prototypes **************************************************/

static uint16_t getObjectID(const uint8_t *in);
static void putCounter(uint16_t in, uint8_t *out);

/* functions **********************************************************/

bool PR6_ServerInput(struct pr6_server *r, pr6_server_object_interface_fn_t objectInterface, const uint8_t *role, uint8_t roleSize, uint16_t counter, const uint8_t *in, uint16_t inLen, uint8_t *out, uint16_t *outLen, uint16_t outMax)
{
    bool retval = false;
    uint16_t ret;
    enum pr6_tag tag;
    uint16_t pos = 0U;
    uint16_t size;
    enum loopControl { LOOP, BREAK_LOOP } loopState = LOOP;

    ASSERT(( r != NULL ))
    ASSERT(( (inLen == 0U) || (in != NULL) ))
    ASSERT(( (outMax == 0U) || (out != NULL) ))
    ASSERT(( (roleSize == 0U) || (role != NULL) ))

    *outLen = 0U;

    if(inLen > 0U){

#ifndef NDEBUG
        r->magic = SERVER_STATE_MAGIC;
#endif
        r->objectInterface = objectInterface;
        r->state = PR6_SERVER_STATE_IDLE;
        r->confirmed = true;
        r->role = role;
        r->roleSize = roleSize;

        if(PR6_CastTag(in[pos], &tag) > 0U){

            pos += PR6_SIZE_TAG;

            switch(tag){
            case PR6_METHOD_REQ:
            case PR6_METHOD_BOE_REQ:
            case PR6_METHOD_NC_REQ:
            case PR6_METHOD_NC_BOE_REQ:

                r->methodList = &in[pos];
                r->methodListLen = pos;
                r->methodListPos = 0U;
                
                while(pos < inLen){

                    if((inLen - pos) >= PR6_SIZE_METHOD_ID){

                        pos += PR6_SIZE_METHOD_ID;
                        ret = PR6_GetVint(&in[pos], inLen - pos, &size);

                        if(ret > 0U){

                            pos += ret;

                            if((inLen - pos) >= size){

                                pos += size;
                            }
                            else{

                                DEBUG("input too short for argument")
                                loopState = BREAK_LOOP;
                            }                                
                        }
                        else{

                            DEBUG("invalid argument length encoding")
                            loopState = BREAK_LOOP;
                        }
                    }
                    else{
                                                
                        DEBUG("input too short for method-id")
                        loopState = BREAK_LOOP;
                    }

                    if(loopState == BREAK_LOOP){

                        break;
                    }
                }

                if(loopState == LOOP){

                    if(pos > PR6_SIZE_TAG){

                        r->methodListLen = pos - r->methodListLen;

                        /* confirmed? */
                        switch(tag){
                        case PR6_METHOD_REQ:
                        case PR6_METHOD_BOE_REQ:

                            r->confirmed = true;
                            break;
                            
                        default:

                            r->confirmed = false;
                            break;
                        }

                        /* breakOnError? */
                        switch(tag){
                        case PR6_METHOD_NC_BOE_REQ:
                        case PR6_METHOD_BOE_REQ:

                            r->breakOnError = true;
                            break;

                        default:

                            r->breakOnError = false;
                            break;
                        }

                        r->counter = counter;

                        r->state = PR6_SERVER_STATE_BEGIN;
                        retval = PR6_ServerResume(r, out, outLen, outMax);    
                    }
                    else{

                        DEBUG("input too short for methodList")
                    }
                }
                
                break;

            case PR6_METHOD_RES:
            default:

                DEBUG("server does not handle this tag")
                break;
            }
        }
        else{

            DEBUG("unknown tag")
        }
    }
    else{

        DEBUG("empty message")    
    }

    return retval;
}

bool PR6_ServerResume(struct pr6_server *r, uint8_t *out, uint16_t *outLen, uint16_t outMax)
{
    bool yielding = false;
    enum loopControl { LOOP, BREAK_LOOP } loopState = LOOP;            
    ASSERT(( (r != NULL) && (r->magic == SERVER_STATE_MAGIC) ))

    if(outMax >= 2U){
    
        while(loopState == LOOP){
            
            switch(r->state){
            case PR6_SERVER_STATE_IDLE:
                loopState = BREAK_LOOP;
                break;

            case PR6_SERVER_STATE_YIELD:
                r->state = PR6_SERVER_STATE_CALL;
                break;

            case PR6_SERVER_STATE_BEGIN:

                *outLen = 0U;

                if(outMax > (PR6_SIZE_TAG + PR6_SIZE_RES_COUNTER)){
                    
                    out[*outLen] = (uint8_t)PR6_METHOD_RES;
                    *outLen += PR6_SIZE_TAG;
                    putCounter(r->counter, &out[*outLen]);
                    *outLen += PR6_SIZE_RES_COUNTER;

                    r->state = PR6_SERVER_STATE_FIRSTCALL;
                }
                else{

                    r->state = PR6_SERVER_STATE_IDLE;
                }
                break;

            case PR6_SERVER_STATE_FIRSTCALL:
            case PR6_SERVER_STATE_CALL:

                if(r->confirmed == false){

                    *outLen = 0U;                
                }

                if((outMax - *outLen) > (PR6_SIZE_RESULT + 1U)){

                    struct pr6_server_adapter adapter;
                    enum pr6_result methodResult;
                    enum pr6_adapter_result retval;

                    ASSERT((r->methodListLen >= (r->methodListPos + PR6_SIZE_METHOD_ID)))

                    uint16_t outOffset = PR6_SIZE_RESULT + (uint16_t)PR6_SizeofVint(outMax - (*outLen + PR6_SIZE_RESULT));                
                    uint16_t inOffset = PR6_SIZE_METHOD_ID + (uint16_t)PR6_GetVint(&r->methodList[r->methodListPos + PR6_SIZE_METHOD_ID], r->methodListPos + PR6_SIZE_METHOD_ID - r->methodListLen, &adapter.inLen);

                    adapter.objectID = getObjectID(&r->methodList[r->methodListPos]);
                    adapter.methodIndex = r->methodList[r->methodListPos + PR6_SIZE_OBJECT_ID];
                    adapter.in = &r->methodList[r->methodListPos + inOffset];
                    
                    adapter.role = r->role;
                    adapter.roleSize = r->roleSize;
                    
                    adapter.outLen = 0U;
                    adapter.out = &out[(size_t)*outLen + outOffset];
                    adapter.outMax = outMax - (*outLen + outOffset);

                    retval = r->objectInterface(r, &adapter, &methodResult);

                    r->state = PR6_SERVER_STATE_CALL;

                    switch(retval){
                    case PR6_ADAPTER_SUCCESS:

                        r->methodListPos += inOffset;
                        r->methodListPos += adapter.inLen;
                        
                        out[*outLen] = (uint8_t)methodResult;
                        *outLen += PR6_SIZE_RESULT;

                        switch(methodResult){
                        case PR6_RESULT_SUCCESS:

                            *outLen += (uint16_t)PR6_PutVint(adapter.outLen, &out[*outLen], outMax - *outLen);
                            memmove(&out[*outLen], adapter.out, (size_t)adapter.outLen);
                            *outLen += adapter.outLen;
                            break;            
                            
                        case PR6_RESULT_OBJECT_UNDEFINED:
                        case PR6_RESULT_METHOD_UNDEFINED:
                        case PR6_RESULT_ACCESS_DENIED:
                        case PR6_RESULT_ARGUMENT:
                        case PR6_RESULT_PERMANENT:
                        case PR6_RESULT_TEMPORARY:

                            if(r->breakOnError == true){

                                while(r->methodListPos < r->methodListLen){

                                    if(*outLen < outMax){

                                        uint16_t argumentLen;
                                        r->methodListPos += PR6_SIZE_METHOD_ID;
                                        r->methodListPos += PR6_GetVint(&r->methodList[r->methodListPos], r->methodListLen - r->methodListPos, &argumentLen);
                                        r->methodListPos += argumentLen;
                                        out[*outLen] = (uint8_t)PR6_RESULT_PERMANENT;
                                        *outLen += PR6_SIZE_RESULT;                                    
                                    }
                                    else{

                                        if(r->confirmed == false){
                                            *outLen = 0U;
                                        }
                                        r->state = PR6_SERVER_STATE_IDLE;                                        
                                        break;                                    
                                    }
                                }
                            }                        
                            break;

                        default:

                            /*impossible*/
                            break;
                        }

                        if(r->state != PR6_SERVER_STATE_IDLE){
                        
                            if(r->methodListPos < r->methodListLen){

                                r->state = PR6_SERVER_STATE_FIRSTCALL;
                            }
                            else{

                                if(r->confirmed == false){

                                    *outLen = 0U;
                                }
                                r->state = PR6_SERVER_STATE_IDLE;
                            }
                        }

                        break;

                    case PR6_ADAPTER_YIELD:
                        yielding = true;
                        r->state = PR6_SERVER_STATE_YIELD;
                        loopState = BREAK_LOOP;
                        break;

                    
                    case PR6_ADAPTER_BUFFER:
                        if(r->confirmed == false){

                            *outLen = 0U;
                        }
                        r->state = PR6_SERVER_STATE_IDLE;
                        break;
                        
                    default:

                        /* impossible */
                        break;
                    }
                }
                else{

                    r->state = PR6_SERVER_STATE_IDLE;
                }

                break;

            default:

                /* impossible */
                break;
            }
        }
    }
    else{

        *outLen = 0U;
        DEBUG("outMax is less than minimum pdu size")
    }
    
    return yielding;
}

bool PR6_ServerIsYielding(const struct pr6_server *r)
{
    ASSERT(( (r != NULL) && (r->magic == SERVER_STATE_MAGIC) ))

    bool retval;

    if(r->state == PR6_SERVER_STATE_YIELD){

        retval = true;
    }
    else{

        retval = false;
    }

    return retval;
}

/* static functions ***************************************************/

static uint16_t getObjectID(const uint8_t *in)
{
    uint16_t ret = in[0];
    ret <<= 8U;
    ret |= in[1];
    
    return ret;
}

static void putCounter(uint16_t in, uint8_t *out)
{
    out[0] = (uint8_t)(in >> 8U);
    out[1] = (uint8_t)(in);
}
