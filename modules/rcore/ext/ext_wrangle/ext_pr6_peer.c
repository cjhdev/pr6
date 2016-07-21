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
 * */

/* includes ***********************************************************/

#include <ruby.h>
#include <malloc.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#include "ext_common.h"

/* static variables ***************************************************/

static VALUE SymMAC;
static VALUE SymText;

static VALUE ConstEUI64;
static VALUE ConstAssociationRecord;

/* static function prototypes *****************************************/

static VALUE peerUnpackMessage(VALUE self, VALUE msg);
static VALUE peerPackMessage(VALUE self, VALUE remoteID, VALUE msg);

static bool myEncrypt(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, uint8_t *mac128);
static bool myDecrypt(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, const uint8_t *mac128);
static bool inputCounter(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, uint32_t counter);
static bool outputCounter(const uint8_t *entityID, const uint8_t *remoteID, uint32_t *counter);

/* functions **********************************************************/

void EXT_PR6_PeerInit(void)
{
    rb_require("wrangle/eui64_pair");    
    rb_require("wrangle/association_record");
        
    VALUE wrangle = rb_define_module("Wrangle");
    VALUE peer = rb_define_class_under(wrangle, "Peer", rb_cObject);

    rb_define_private_method(peer, "unpackMessage", peerUnpackMessage, 3);
    rb_define_private_method(peer, "packMessage", peerPackMessage, 2);

    SymText = ID2SYM(rb_intern("text"));
    SymMAC = ID2SYM(rb_intern("mac"));
    
    ConstEUI64 = rb_const_get(wrangle, rb_intern("EUI64"));
    ConstAssociationRecord = rb_const_get(wrangle, rb_intern("AssociationRecord"));
    
    struct pr6_peer_init fnPointers = {
        .encrypt = myEncrypt,
        .decrypt = myDecrypt,
        .inputCounter = inputCounter,
        .outputCounter = outputCounter
    };

    PR6_PeerInit(&fnPointers);
}
    
/* static functions ***************************************************/

static VALUE peerUnpackMessage(VALUE self, VALUE msg)
{
    VALUE retval;
    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_bytes"), 1, rb_funcall(self, rb_intern("entityID"), 0));

    uint32_t outMax = RSTRING_LEN(msg);
    uint8_t *out = ALLOC_N(uint8_t, outMax);
    uint32_t outLen;

    struct pr6_peer_header header;
    enum pr6_recipient recipient;
    uint32_t counter;
    
    outLen = PR6_PeerInput((const uint8_t *)RSTRING_PTR(oEntityID), (uint8_t *)RSTRING_PTR(msg), RSTRING_LEN(msg), out, outMax, &header, &recipient, &counter);

    if(outLen > 0U){

        VALUE oRecipient = (recipient == PR6_RECIPIENT_CLIENT) ? ID2SYM(rb_intern("PR6_RECIPIENT_CLIENT")) : ID2SYM(rb_intern("PR6_RECIPIENT_SERVER"));

        retval = rb_hash_new();

        rb_hash_aset(retval, ID2SYM(rb_intern("data")), rb_str_new((const char *)out, outLen));
        rb_hash_aset(retval, ID2SYM(rb_intern("to")), rb_str_new((const char *)header.to, sizeof(header.to)));
        rb_hash_aset(retval, ID2SYM(rb_intern("from")), rb_str_new((const char *)header.from, sizeof(header.from)));
        rb_hash_aset(retval, ID2SYM(rb_intern("recipient")), oRecipient);
        rb_hash_aset(retval, ID2SYM(rb_intern("counter")), UINT2NUM(counter));
    }
    else{

        retval = Qnil;
    }

    return retval;
}

static VALUE peerPackMessage(VALUE self, VALUE remoteID, VALUE msg)
{
    VALUE retval = Qnil;
    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_bytes"), 1, rb_funcall(self, rb_intern("entityID"), 0));
    VALUE oRemoteID = rb_funcall(ConstEUI64, rb_intern("new_to_bytes"), 1, remoteID);    

    uint32_t outMax = RSTRING_LEN(msg) + PR6_SIZE_GCM_OVERHEAD;
    uint8_t *out = ALLOC_N(uint8_t, outMax);
    uint32_t outLen;
    uint32_t counter;

    outLen = PR6_PeerOutput((const uint8_t *)RSTRING_PTR(oEntityID), (const uint8_t *)RSTRING_PTR(oRemoteID), (const uint8_t *)RSTRING_PTR(msg), RSTRING_LEN(msg), out, outMax, &counter);

    if(outLen > 0U){

        retval = rb_hash_new();
        rb_hash_aset(retval, ID2SYM(rb_intern("data")), rb_str_new((const char *)out, outLen));
        rb_hash_aset(retval, ID2SYM(rb_intern("counter")), UINT2NUM(counter));
    }

    return retval;
}

static bool myEncrypt(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, uint8_t *mac128)
{
    bool result = false;

    VALUE oIV = rb_str_new((const char *)iv, ivLen);
    VALUE oText;
    VALUE oAAD = rb_str_new((const char *)aad, aadLen);

    if(text != NULL){

        oText = rb_str_new((const char *)text, textLen);
    }
    else{

        oText = rb_str_new("", 0);
    }

    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)entityID, PR6_SIZE_ENTITY_ID));
    VALUE oRemoteID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)remoteID, PR6_SIZE_ENTITY_ID));

    VALUE retval = rb_funcall(ConstAssociationRecord, rb_intern("encryptGCM"), 5,
        oEntityID,
        oRemoteID,
        oIV,
        oText,
        oAAD
    );

    if(retval != Qnil){

        assert(RSTRING_LEN(rb_hash_aref(retval, SymText)) == textLen);
        assert(RSTRING_LEN(rb_hash_aref(retval, SymMAC)) == PR6_SIZE_GCM_MAC128);

        memcpy(text, RSTRING_PTR(rb_hash_aref(retval, SymText)), textLen);
        memcpy(mac128, RSTRING_PTR(rb_hash_aref(retval, SymMAC)), PR6_SIZE_GCM_MAC128);

        result = true;
    }

    return result;
}

static bool myDecrypt(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, const uint8_t *mac128)
{
    bool result = false;

    VALUE oIV = rb_str_new((const char *)iv, ivLen);
    VALUE oText;
    VALUE oAAD = rb_str_new((const char *)aad, aadLen);
    VALUE oMAC = rb_str_new((const char *)mac128, PR6_SIZE_GCM_MAC128);

    if(text != NULL){

        oText = rb_str_new((const char *)text, textLen);
    }
    else{

        oText = rb_str_new("", 0);
    }

    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)entityID, PR6_SIZE_ENTITY_ID));
    VALUE oLocalID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)localID, PR6_SIZE_ENTITY_ID));
    VALUE oRemoteID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)remoteID, PR6_SIZE_ENTITY_ID));

    VALUE retval = rb_funcall(ConstAssociationRecord, rb_intern("decryptGCM"), 7,
        oEntityID,
        oLocalID,
        oRemoteID,
        oIV,
        oText,
        oAAD,
        oMAC);

    if(retval != Qnil){

        assert(RSTRING_LEN(retval) == textLen);
        
        memcpy(text, RSTRING_PTR(retval), textLen);
        
        result = true;
    }

    return result;
}

static bool inputCounter(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, uint32_t counter)
{
    bool result = false;
    
    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)entityID, PR6_SIZE_ENTITY_ID));
    VALUE oLocalID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)localID, PR6_SIZE_ENTITY_ID));
    VALUE oRemoteID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)remoteID, PR6_SIZE_ENTITY_ID));
    VALUE oCounter = UINT2NUM(counter);
    
    if(rb_funcall(ConstAssociationRecord, rb_intern("inputCounter"), 4, oEntityID, oLocalID, oRemoteID, oCounter) == Qtrue){

        result = true;
    }

    return result;    
}

static bool outputCounter(const uint8_t *entityID, const uint8_t *remoteID, uint32_t *counter)
{
    bool result = false;

    VALUE oEntityID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)entityID, PR6_SIZE_ENTITY_ID));
    VALUE oRemoteID = rb_funcall(ConstEUI64, rb_intern("new_to_s"), 1, rb_str_new((const char *)remoteID, PR6_SIZE_ENTITY_ID));
    
    VALUE retval = rb_funcall(ConstAssociationRecord, rb_intern("outputCounter"), 2, oEntityID, oRemoteID);

    if(retval != Qnil){

        *counter = NUM2UINT(retval);
        result = true;
    }

    return result;
}
