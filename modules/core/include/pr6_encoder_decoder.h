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
#ifndef PR6_ENCODER_DECODER_H
#define PR6_ENCODER_DECODER_H

/**
 * @defgroup pr6_encoder_decoder PR6 Common Encode/Decode Interface
 * @ingroup pr05
 * 
 * PR6 encoder/decoder functions common to client and server
 *
 *
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>

/* defines ************************************************************/

/** byte size of an alternative determinant */
#define PR6_SIZE_TAG 1U

/** byte size of a `@PR6.Result` */
#define PR6_SIZE_RESULT 1U

/** byte size of a `@PR6.Method-ID.object-id` */
#define PR6_SIZE_OBJECT_ID 2U

/** byte size of a `@PR6.Method-ID.method-index` */
#define PR6_SIZE_METHOD_INDEX 1U

/** byte size of a `@PR6.Method-ID` */
#define PR6_SIZE_METHOD_ID (PR6_SIZE_OBJECT_ID + PR6_SIZE_METHOD_INDEX)

/** byte size of an Entity Identifier */
#define PR6_SIZE_ENTITY_ID 8U

/** byte size of a `@PR6.GCM.counter` */
#define PR6_SIZE_GCM_COUNTER 4U

/** byte size of a `@PR6.METHOD-RES.counter` */
#define PR6_SIZE_RES_COUNTER 2U

/** byte size of a `@PR6.GCM.payload` length determinant field */
#define PR6_SIZE_GCM_PAYLOAD_LEN 2U

/** byte size of a `@PR6.GCM.mac` field */
#define PR6_SIZE_GCM_MAC128 16U

/** byte size of a GCM IV */
#define PR6_SIZE_GCM_IV 12U

/** byte size of all `@PR6.GCM` overhead */
#define PR6_SIZE_GCM_OVERHEAD  (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID + PR6_SIZE_GCM_COUNTER + PR6_SIZE_GCM_PAYLOAD_LEN + PR6_SIZE_GCM_MAC128)

/** byte size of `@PR6.GCM` elements before `@PR6.GCM.ct` */
#define PR6_SIZE_GCM_HEADER (PR6_SIZE_ENTITY_ID + PR6_SIZE_ENTITY_ID + PR6_SIZE_GCM_COUNTER + PR6_SIZE_GCM_PAYLOAD_LEN)

/* enumerations *******************************************************/

/**
 * PR6 Result
 *
 * @see `@PR6.Result`
 *
 * */
enum pr6_result {

    PR6_RESULT_SUCCESS = 0,        /**< successful invocation */
    PR6_RESULT_OBJECT_UNDEFINED,   /**< object does not exist on server */
    PR6_RESULT_METHOD_UNDEFINED,   /**< method does not exist */
    PR6_RESULT_ACCESS_DENIED,      /**< access to method is not allowed */
    PR6_RESULT_ARGUMENT,           /**< argument is invalid for method */
    PR6_RESULT_PERMANENT,          /**< method invocation failed for permanent reason */
    PR6_RESULT_TEMPORARY           /**< method invocation failed for temporary reason (try again for different result) */
};

/** PR6 PDU CHOICE encoding
 *
 * @see `@PR6.METHOD`
 * 
 * */
enum pr6_tag {

    PR6_METHOD_REQ = 0,     /**< tag for `@PR6.METHOD.method-req` */
    PR6_METHOD_NC_REQ,      /**< tag for `@PR6.METHOD.method-nc-req` */
    PR6_METHOD_NC_BOE_REQ,  /**< tag for `@PR6.METHOD.method-nc-boe-req` */
    PR6_METHOD_BOE_REQ,     /**< tag for `@PR6.METHOD.method-boe-req` */    
    PR6_METHOD_RES          /**< tag for `@PR6.METHOD.method-res` */    
};

/** Enumeration to indicate which type of instance (Client/Server) should recieve this type of message */
enum pr6_recipient {

    PR6_RECIPIENT_CLIENT,      /**< message to be recieved by client instance */
    PR6_RECIPIENT_SERVER       /**< message to be recieved by server instance */
};

/* structures *********************************************************/


/* functions **********************************************************/

/**
 * Get a variable length unsigned integer
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded unsigned integer
 *
 * @return number of bytes successfully read from `in`
 * 
 * */
uint8_t PR6_GetVint(const uint8_t *in, uint16_t inLen, uint16_t *out);

/**
 * Put a variable length unsigned integer
 *
 * @param[in] in unsigned integer to encode
 * @param[in] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 *
 * */
uint8_t PR6_PutVint(uint16_t in, uint8_t *out, uint16_t outMax);

/**
 * Get encoded size of variable length unsigned integer
 *
 * @param[in] in unsigned integer to encode
 *
 * @return number of bytes required to encode `in`
 *
 * */
uint8_t PR6_SizeofVint(uint16_t in);

/**
 * Cast a byte to a pr6_tag enumerated type
 *
 * @param[in] in byte
 * @param[out] out enumerated type
 *
 * @return number of bytes successfully cast 
 *
 * */
uint8_t PR6_CastTag(uint8_t in, enum pr6_tag *out);


#ifdef __cplusplus
}
#endif

/** @} */
#endif
