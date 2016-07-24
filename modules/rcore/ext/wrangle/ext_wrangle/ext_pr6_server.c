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
#include <assert.h>
#include <stddef.h>

#include "ext_common.h"
#include "pr6_encoder_decoder.h"

/* internal function prototypes ***************************************/

static VALUE serverInput(VALUE self, VALUE counter, VALUE input);
static enum pr6_result symbolToResult(VALUE symbol);
static enum pr6_adapter_result symbolToAdapterResult(VALUE symbol);
static enum pr6_adapter_result serverObjectInterface(void *ctxt, const struct pr6_server *r, struct pr6_server_adapter *arg, enum pr6_result *result);
static uint8_t castAdapterResult(uint8_t in, enum pr6_adapter_result *out);
static VALUE serverInitialise(VALUE self, VALUE association, VALUE objects);
static VALUE association(VALUE self);

/* local variables ****************************************************/


/* public functions ***************************************************/

void EXT_PR6_ServerInit(void)
{
    VALUE wrangle = rb_define_module("Wrangle");
    VALUE pr6Server = rb_define_class_under(wrangle, "Server", rb_cObject);
    
    rb_define_method(pr6Server, "initialize", serverInitialise, 2);
    rb_define_method(pr6Server, "input", serverInput, 2);    
    rb_define_method(pr6Server, "association", association, 0);    
}

/* internal functions *************************************************/

static VALUE serverInitialise(VALUE self, VALUE association, VALUE objects)
{
    rb_iv_set(self, "@association", association);
    rb_iv_set(self, "@objects", objects);

    return self;
}

static VALUE serverInput(VALUE self, VALUE counter, VALUE input)
{
    struct pr6_server r;
    
    const uint8_t *in = (const uint8_t *)RSTRING_PTR(input);
    uint32_t inLen = (uint32_t)RSTRING_LEN(input);
    VALUE outMax = rb_funcall(rb_iv_get(self, "@association"), rb_intern("remoteMax"), 0);
    uint16_t _outMax;

    if((NUM2UINT(outMax) == 0) || (NUM2UINT(outMax) > 0xffff)){

        _outMax = 0xffff;
    }
    else{

        _outMax = NUM2UINT(outMax);
    }
    
    uint8_t *out = ALLOC_N(uint8_t, _outMax);
    uint16_t outLen;

    PR6_ServerInput(&self, &r, serverObjectInterface, NULL, 0U, (uint16_t)NUM2UINT(counter), in, inLen, out, &outLen, _outMax);

    return rb_str_new((const char *)out, outLen);
}

static enum pr6_result symbolToResult(VALUE symbol)
{
    VALUE table = rb_const_get(rb_define_module("Wrangle"), rb_intern("PR6_RESULT"));
    uint16_t i;

    static const enum pr6_client_result results[] = {
        PR6_RESULT_SUCCESS,
        PR6_RESULT_OBJECT_UNDEFINED,
        PR6_RESULT_METHOD_UNDEFINED,
        PR6_RESULT_ACCESS_DENIED,
        PR6_RESULT_ARGUMENT,
        PR6_RESULT_PERMANENT,
        PR6_RESULT_TEMPORARY
    };

    for(i=0; i < 255; i++){
    
        VALUE result = rb_ary_entry(table, i);

        if(result == Qnil){

            rb_bug("PR6_RESULT is out of alignment with enum pr6_result\n");
        }
        else if(result == symbol){

            break;
        }
    }

    assert(i < (sizeof(results)/sizeof(*results)));

    return results[i];
}

static enum pr6_adapter_result symbolToAdapterResult(VALUE symbol)
{
    enum pr6_adapter_result retval;
    VALUE table = rb_const_get(rb_define_module("Wrangle"), rb_intern("PR6_ADAPTER_RESULT"));
    uint8_t i = 0;

    for(i=0; i < 255; i++){
    
        VALUE result = rb_ary_entry(table, i);

        if(result == Qnil){

            rb_bug("PR6_ADAPTER_RESULT is out of alignment with enum pr6_adapter_result\n");
        }
        else if(result == symbol){

            break;
        }

        i++;
    }
    
    if(castAdapterResult(i, &retval) == 0U){

        rb_bug("impossible");
    }

    return retval;
}

static enum pr6_adapter_result serverObjectInterface(void *ctxt, const struct pr6_server *r, struct pr6_server_adapter *arg, enum pr6_result *result)
{
    assert(r != NULL);
    enum pr6_adapter_result retval = PR6_ADAPTER_SUCCESS;    
    VALUE self = *(VALUE *)ctxt;

    VALUE object = rb_hash_aref(rb_iv_get(self, "@objects"), UINT2NUM(arg->objectID));

    if(object != Qnil){

        VALUE callResult = rb_funcall(object, rb_intern("callMethod"), 3, self, UINT2NUM(arg->methodIndex), rb_str_new((const char *)arg->in, arg->inLen));

        retval = symbolToAdapterResult(rb_hash_aref(callResult, ID2SYM(rb_intern("adapterResult"))));

        if(retval == PR6_ADAPTER_SUCCESS){

            *result = symbolToResult(rb_hash_aref(callResult, ID2SYM(rb_intern("result"))));

            if(*result == PR6_RESULT_SUCCESS){

                if(arg->outMax >= RSTRING_LEN(rb_hash_aref(callResult, ID2SYM(rb_intern("returnValue"))))){

                    arg->outLen = RSTRING_LEN(rb_hash_aref(callResult, ID2SYM(rb_intern("returnValue"))));
                    memcpy(arg->out, RSTRING_PTR(rb_hash_aref(callResult, ID2SYM(rb_intern("returnValue")))), arg->outLen);
                }
                else{

                    retval = PR6_ADAPTER_BUFFER;
                }                
            }
        }
    }
    else{

        *result = PR6_RESULT_OBJECT_UNDEFINED;
    }
    
    return retval;
}

static uint8_t castAdapterResult(uint8_t in, enum pr6_adapter_result *out)
{
    uint8_t retval = 0U;

    static const enum pr6_adapter_result results[] = {
        PR6_ADAPTER_SUCCESS,
        PR6_ADAPTER_BUFFER,
        PR6_ADAPTER_YIELD
    };

    if(in < (sizeof(results) / sizeof(*results))){

        *out = results[in];
        retval = 1U;
    }    

    return retval;
}

static VALUE association(VALUE self)
{
    return rb_iv_get(self, "@association");
}
