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

#include "pr6_peer.h"
#include "debug_interface.h"

#include <stddef.h>
#include <string.h>

/* defines ************************************************************/

#define PR6_PEER_MAGIC 0xdeadb0efU


/* forward declarations ***********************************************/

struct pr6_gcm;

/* static function prototypes *****************************************/

static uint32_t getGCM(uint8_t *in, uint32_t inLen, struct pr6_gcm *out);
static uint32_t putGCM(const uint8_t *dest, const uint8_t *source, const uint8_t *in, uint16_t inLen, uint32_t counter, const uint8_t *mac128, uint8_t *out, uint32_t outMax);
static void makeIV(const uint8_t *entityID, uint32_t counter, uint8_t *out);
static void putCounter(uint32_t in, uint8_t *out);
static uint32_t getCounter(const uint8_t *in);
static void putSize(uint16_t in, uint8_t *out);
static uint16_t getSize(const uint8_t *in);

/* structures *********************************************************/

/** GCM container */
struct pr6_gcm {

    uint8_t *to;
    uint8_t *from;

    uint32_t counter;   /**< counter field used to derive a GCM IV */

    const uint8_t *aad;     /**< pointer to Additional Authenticated Data field */
    uint32_t aadLen;        /**< byte length of `AAD` */

    uint8_t *payload;       /**< pointer to payload field */
    uint16_t payloadLen;    /**< byte length of `payload` */

    uint8_t *mac128;  /**< pointer to 128 bit Message Authentication Code field */
};

/* static variables ***************************************************/

/* pointers to target specific implementation */
static pr6_sm_decrypt_fn_t decrypt;       
static pr6_sm_encrypt_fn_t encrypt;           
static pr6_sm_input_counter_fn_t inputCounter;
static pr6_sm_output_counter_fn_t outputCounter;

/* use this to tell if user has initialised the target specific pointers */
static uint32_t magic;

/* functions **********************************************************/

void PR6_PeerInit(const struct pr6_peer_init *param)
{
    ASSERT((param != NULL))
    ASSERT((param->decrypt != NULL))
    ASSERT((param->encrypt != NULL))
    ASSERT((param->inputCounter != NULL))
    ASSERT((param->outputCounter != NULL))    

#ifndef NDEBUG
    magic = PR6_PEER_MAGIC;
#endif

    decrypt = param->decrypt;
    encrypt = param->encrypt;
    inputCounter = param->inputCounter;
    outputCounter = param->outputCounter;    
}

uint32_t PR6_PeerGetHeader(const uint8_t *in, uint32_t inLen, struct pr6_peer_header *out)
{
    uint32_t pos = 0U;
    
    if(inLen >= (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID)){

        memcpy(out->to, in, (size_t)PR6_SIZE_ENTITY_ID);
        memcpy(out->from, &in[PR6_SIZE_ENTITY_ID], (size_t)PR6_SIZE_ENTITY_ID);
        pos = (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID);
    }

    return pos;
}

uint16_t PR6_PeerInput(const uint8_t *entityID, uint8_t *in, uint32_t inLen, uint8_t *out, uint16_t outMax, struct pr6_peer_header *header, enum pr6_recipient *recipient, uint32_t *counter)
{
    ASSERT((magic == PR6_PEER_MAGIC))
    ASSERT((entityID != NULL))
    ASSERT((in != NULL))
    ASSERT((out != NULL))
    ASSERT((header != NULL))
    ASSERT((recipient != NULL))
    ASSERT((counter != NULL))
    
    uint16_t retval = 0U;
    uint8_t iv[PR6_SIZE_GCM_IV];
    struct pr6_gcm m;
    enum pr6_tag tag;
    
    uint32_t ret = getGCM(in, inLen, &m);

    if(ret > 0U){

        if(ret == inLen){

            memcpy(header->to, m.to, (size_t)PR6_SIZE_ENTITY_ID);
            memcpy(header->from, m.from, (size_t)PR6_SIZE_ENTITY_ID);

            if(outMax >= m.payloadLen){

                makeIV(m.from, m.counter, iv);
            
                if(decrypt(entityID, m.to, m.from, iv, (uint8_t)sizeof(iv), m.payload, m.payloadLen, m.aad, m.aadLen, m.mac128)){

                    if(inputCounter(entityID, m.to, m.from, m.counter)){                                

                        if(in == out){

                            memmove(out, m.payload, (size_t)m.payloadLen);
                        }
                        else{

                            /* not zero copy */
                            if(m.payload != out){

                                memcpy(out, m.payload, (size_t)m.payloadLen);
                            }
                        }
            
                        if(PR6_CastTag(out[0], &tag) > 0U){

                            switch(tag){
                            case PR6_METHOD_REQ:
                            case PR6_METHOD_NC_REQ:
                            case PR6_METHOD_NC_BOE_REQ:
                            case PR6_METHOD_BOE_REQ:                                    
                                *recipient = PR6_RECIPIENT_SERVER;
                                break;
                            case PR6_METHOD_RES:
                            default:
                                *recipient = PR6_RECIPIENT_CLIENT;
                                break;
                            }

                            *counter = m.counter;
                            retval = m.payloadLen;
                        }
                        else{

                            DEBUG("unknown message tag")
                        }                            
                    }
                    else{

                        INFO("counter rejected")
                    }
                }
                else{

                    INFO("could not decrypt")
                }                                       
            }
            else{

                DEBUG("output buffer too short")
            }          
        }
        else{

            DEBUG("input is padded")
        }
    }
            
    return retval;
}

uint32_t PR6_PeerOutput(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *in, uint16_t inLen, uint8_t *out, uint32_t outMax, uint32_t *counter)
{
    ASSERT((magic == PR6_PEER_MAGIC))
    ASSERT((entityID != NULL))
    ASSERT((remoteID != NULL))
    ASSERT((in != NULL))
    ASSERT((out != NULL))
    
    uint32_t retval = 0U;
    uint32_t ret;
    const uint8_t mac[PR6_SIZE_GCM_MAC128];
    uint8_t iv[PR6_SIZE_GCM_IV];
    struct pr6_gcm m;

    if(inLen > 0U){
        
        if(outMax >= (PR6_SIZE_GCM_OVERHEAD + inLen)){

            if(outputCounter(entityID, remoteID, counter)){

                makeIV(entityID, *counter, iv);

                retval = putGCM(remoteID, entityID, in, inLen, *counter, mac, out, outMax);

                if(retval > 0U){

                    ret = getGCM(out, retval, &m);

                    ASSERT((ret == retval))
        
                    if(ret > 0U){
                    
                        if(encrypt(entityID, remoteID, iv, (uint8_t)sizeof(iv), m.payload, m.payloadLen, m.aad, m.aadLen, m.mac128) == false){

                            DEBUG("could not encrypt output")
                            retval = 0U;
                        }
                    }
                }
            }
            else{

                DEBUG("could not produce an output counter value")
            }
        }
        else{

            DEBUG("output buffer too short")
        }
    }
    else{

        DEBUG("will not output a zero length message")
    }
                    
    return retval;
}

/* static functions ***************************************************/

static uint32_t getGCM(uint8_t *in, uint32_t inLen, struct pr6_gcm *out)
{
    ASSERT((in != NULL))
    ASSERT((out != NULL))
    
    uint32_t retval = 0U;
    uint32_t pos = 0U;

    if(inLen > (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID + PR6_SIZE_GCM_COUNTER + PR6_SIZE_GCM_PAYLOAD_LEN + PR6_SIZE_GCM_MAC128)){

        out->to = in;
        pos += PR6_SIZE_ENTITY_ID;

        out->from = &in[pos];
        pos += PR6_SIZE_ENTITY_ID;

        out->aad = &in[pos];

        out->counter = getCounter(&in[pos]);
        pos += PR6_SIZE_GCM_COUNTER;
        
        out->payloadLen = getSize(&in[pos]);
        pos += PR6_SIZE_GCM_PAYLOAD_LEN;

        if((inLen - pos) >= (PR6_SIZE_GCM_MAC128 + (uint32_t)out->payloadLen)){
        
            out->payload = &in[pos];
            out->aadLen = pos;

            pos += out->payloadLen;

            out->mac128 = &in[pos];
            pos += PR6_SIZE_GCM_MAC128;

            retval = pos;
        }
        else{

            DEBUG("input too short for encoded payload size")
        }   
    }
    else{

        DEBUG("input too short")
    }        
    
    return retval;
}

static uint32_t putGCM(const uint8_t *to, const uint8_t *from, const uint8_t *in, uint16_t inLen, uint32_t counter, const uint8_t *mac128, uint8_t *out, uint32_t outMax)
{
    ASSERT((to != NULL))
    ASSERT((from != NULL))
    ASSERT((in != NULL))
    ASSERT((mac128 != NULL))
    ASSERT((out != NULL))

    uint32_t pos = 0U;
    
    if(outMax >= (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID + PR6_SIZE_GCM_COUNTER + PR6_SIZE_GCM_PAYLOAD_LEN + (uint32_t)inLen + PR6_SIZE_GCM_MAC128)){

        memcpy(out, to, (size_t)PR6_SIZE_ENTITY_ID);
        pos += PR6_SIZE_ENTITY_ID;
        memcpy(&out[pos], from, (size_t)PR6_SIZE_ENTITY_ID);
        pos += PR6_SIZE_ENTITY_ID;
        
        putCounter(counter, &out[pos]);
        pos += PR6_SIZE_GCM_COUNTER;
        putSize(inLen, &out[pos]);
        pos += PR6_SIZE_GCM_PAYLOAD_LEN;

        if(in != &out[pos]){

            memcpy(&out[pos], in, (size_t)inLen);
        }
            
        pos += inLen;

        memcpy(&out[pos], mac128, (size_t)PR6_SIZE_GCM_MAC128);
        pos += PR6_SIZE_GCM_MAC128;                
    }

    return pos;
}

static void makeIV(const uint8_t *entityID, uint32_t counter, uint8_t *out)
{
    memcpy(out, entityID, (size_t)PR6_SIZE_ENTITY_ID);
    putCounter(counter, &out[PR6_SIZE_ENTITY_ID]);        
}

static void putCounter(uint32_t in, uint8_t *out)
{
    out[0] = (uint8_t)(in >> 24U);
    out[1] = (uint8_t)(in >> 16U);
    out[2] = (uint8_t)(in >> 8U);
    out[3] = (uint8_t)(in);
}

static uint32_t getCounter(const uint8_t *in)
{
    uint32_t ret = in[0];
    ret <<= 8U;
    ret |= in[1];
    ret <<= 8U;
    ret |= in[2];
    ret <<= 8U;
    ret |= in[3];

    return ret;
}

static void putSize(uint16_t in, uint8_t *out)
{
    out[0] = (uint8_t)(in >> 8U);
    out[1] = (uint8_t)(in);
}

static uint16_t getSize(const uint8_t *in)
{
    uint16_t ret = in[0];
    ret <<= 8U;
    ret |= in[1];

    return ret;
}
