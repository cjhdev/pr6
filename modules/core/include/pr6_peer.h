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
#ifndef PR6_PEER_H
#define PR6_PEER_H

/**
 * @defgroup pr6_peer PR6 Peer
 * @ingroup pr05
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include "pr6_encoder_decoder.h"

#include <stdbool.h>

/* typedefs ***********************************************************/

/** called to get the next output counter for a given association @see EXAMPLE_SM_OutputCounter */
typedef bool (*pr6_sm_output_counter_fn_t)(const uint8_t *, const uint8_t *, uint32_t *);

/** called to input an input counter for a given association @see EXAMPLE_SM_InputCounter */
typedef bool (*pr6_sm_input_counter_fn_t)(const uint8_t *, const uint8_t *, const uint8_t *, uint32_t);

/** called to encrypt and authenticate in-place for a given association @see EXAMPLE_SM_Encrypt */
typedef bool (*pr6_sm_encrypt_fn_t)(const uint8_t *, const uint8_t *, const uint8_t *, uint8_t, uint8_t *, uint16_t, const uint8_t *, uint32_t, uint8_t *);

/** called to decrypt and authenticate in-place for a given association @see EXAMPLE_SM_Decrypt */
typedef bool (*pr6_sm_decrypt_fn_t)(const uint8_t *, const uint8_t *, const uint8_t *, const uint8_t *, uint8_t, uint8_t *, uint16_t, const uint8_t *, uint32_t, const uint8_t *);

/* structures *********************************************************/

/** platform specific functionality */
struct pr6_peer_init {

    pr6_sm_decrypt_fn_t decrypt;                /**< @see EXAMPLE_SM_Decrypt */ 
    pr6_sm_encrypt_fn_t encrypt;                /**< @see EXAMPLE_SM_Encrypt */ 
    pr6_sm_input_counter_fn_t inputCounter;     /**< @see EXAMPLE_SM_InputCounter */ 
    pr6_sm_output_counter_fn_t outputCounter;   /**< @see EXAMPLE_SM_OutputCounter */ 
};

/** peer address header */
struct pr6_peer_header {

    uint8_t to[PR6_SIZE_ENTITY_ID];    /**< destination */
    uint8_t from[PR6_SIZE_ENTITY_ID];  /**< source */
};

/* functions **********************************************************/

/**
 * Receive a message from a remote peer
 *
 * @see `@PR6.GCM`
 * 
 * @param[in] entityID entity identifier of this peer
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 * @param[out] header extracted header
 * @param[out] recipient client/server recipient
 * @param[out] counter extracted counter value
 *
 * @return number of bytes successfully written to `out`
 * 
 * */
uint16_t PR6_PeerInput(const uint8_t *entityID, uint8_t *in, uint32_t inLen, uint8_t *out, uint16_t outMax, struct pr6_peer_header *header, enum pr6_recipient *recipient, uint32_t *counter);

/**
 * Send a Message to a remote peer
 *
 * @see `@PR6.GCM`
 *
 * @param[in] entityID entity identifier of this peer
 * @param[in] remoteID remote identifier
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 * @param[out] counter counter assigned to this message
 *
 * @return number of bytes successfully written to `out`
 * 
 * */
uint32_t PR6_PeerOutput(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *in, uint16_t inLen, uint8_t *out, uint32_t outMax, uint32_t *counter);

/**
 * Initialise platform specific function pointers
 * 
 * @param[in] param platform specific pointers
 *
 * */
void PR6_PeerInit(const struct pr6_peer_init *param);

/**
 * Get header from input
 *
 * @see `@PR6.GCM` 
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out peer header structure
 *
 * @return number of bytes successfully read from `in`
 *
 * */
uint32_t PR6_PeerGetHeader(const uint8_t *in, uint32_t inLen, struct pr6_peer_header *out);

/* only for generated API documentation */
#ifdef DOXYGEN

/**
 * <b>Example</b> of an interface to provision an output counter value
 * 
 * @param[in] entityID Entity Identifier
 * @param[in] remoteID Entity Identifier of remote party
 * @param[out] counter returned counter value
 *
 * @return status
 *
 * @retval true counter value was provided
 * @retval false counter value cannot be provided
 *
 * */
bool EXAMPLE_SM_OutputCounter(const uint8_t *entityID, const uint8_t *remoteID, uint32_t *counter);

/**
 * <b>Example</b> of an interface to test and input an input counter value 
 *
 * @param[in] entityID Entity Identifier
 * @param[in] localID local Entity Identifier
 * @param[in] remoteID remote Entity Identifier
 * @param[in] counter input counter
 *
 * @return status
 *
 * @retval true counter value is acceptable
 * @retval false counter value is not acceptable
 *
 * */
bool EXAMPLE_SM_InputCounter(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, uint32_t counter);

/**
 * <b>Example</b> of an interface to perform in-place encryption on message elements
 *
 * @param[in] entityID Entity Identifier
 * @param[in] remoteID Entity Identifier of remote party
 * @param[in] iv pointer to 96 bit initial value
 * @param[in] ivLen byte length of `iv`
 * @param[in/out] text pointer to clear text to encrypt in place
 * @param[in] textLen byte length of `text`
 * @param[in] aad pointer to additional authenticated data
 * @param[in] aadLen byte length of `aad`
 * @param[out] mac128 pointer to 16 byte buffer to receive the message authentication code
 *
 * @return status
 *
 * @retval true operation was successful
 * @retval false operation was not successful
 *
 * */
bool EXAMPLE_SM_Encrypt(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, uint8_t *mac128);

/**
 * <b>Example</b> of an interface to perform in-place decryption on message elements
 *
 * @param[in] entityID Entity Identifier
 * @param[in] localID local Entity Identifier
 * @param[in] remoteID remote Entity Identifier
 * @param[in] iv pointer to 96 bit initial value
 * @param[in] ivLen byte length of `iv`
 * @param[in/out] text pointer to clear text to encrypt in place
 * @param[in] textLen byte length of `text`
 * @param[in] aad pointer to additional authenticated data
 * @param[in] aadLen byte length of `aad`
 * @param[out] mac128 pointer to 16 byte buffer containing message authentication code to check against
 *
 * @return status
 *
 * @retval true operation was successful
 * @retval false operation was not successful
 *
 * */
bool EXAMPLE_SM_Decrypt(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, const uint8_t *mac128);
#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif

