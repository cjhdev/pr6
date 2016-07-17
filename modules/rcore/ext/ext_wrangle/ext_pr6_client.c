/* Copyright (c) 2013-2016 Cameron Harper
 *
 *
 * */

/* includes ***********************************************************/

#include <ruby.h>
#include <assert.h>
#include <stddef.h>

#include "ext_common.h"

/* static variables ***************************************************/

static VALUE ConstMethodRequest;
static VALUE ConstMethodResponse;
static VALUE ConstSecureRandom;
static VALUE ConstClient;

/* static function prototypes *****************************************/

static VALUE clientInitialize(VALUE self, VALUE confirmed, VALUE breakOnError, VALUE block);
static VALUE clientInput(VALUE self, VALUE msg);
static VALUE clientOutput(VALUE self, VALUE outMax);
static void clientResultCallback(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list);
static VALUE resultToSymbol(enum pr6_result result);
static void initClientState(struct pr6_client *r, VALUE confirmed, VALUE breakOnError, VALUE requests);
static VALUE uuid(VALUE self);
static VALUE responseHandler(VALUE self, VALUE receiver, VALUE handler);
static VALUE request(VALUE self, VALUE methodID);
static VALUE setCounter(VALUE self, VALUE counter);
static VALUE getCounter(VALUE self);


/* functions **********************************************************/

void EXT_PR6_ClientInit(void)
{
    VALUE wrangle = rb_define_module("Wrangle");
    ConstClient = rb_define_class_under(wrangle, "Client", rb_cObject);

    rb_define_method(ConstClient, "initialize", clientInitialize, 5);
    rb_define_method(ConstClient, "input", clientInput, 1);
    rb_define_method(ConstClient, "output", clientOutput, 2);
    rb_define_method(ConstClient, "uuid", uuid, 0);
    rb_define_method(ConstClient, "registerCounter", setCounter, 1);
    rb_define_method(ConstClient, "counter", getCounter, 0);

    rb_define_private_method(ConstClient, "request", request, 1);
    rb_define_private_method(ConstClient, "responseHandler", responseHandler, 2);
    
    rb_require("wrangle/method_response");
    rb_require("wrangle/method_request");
    rb_require("securerandom");
        
    ConstMethodResponse = rb_const_get(wrangle, rb_intern("MethodResponse"));
    ConstMethodRequest = rb_const_get(wrangle, rb_intern("MethodRequest"));    
}

/* static functions ***************************************************/

static VALUE uuid(VALUE self)
{
    return rb_iv_get(self, "@uuid");
}

static VALUE request(VALUE self, VALUE methodID)
{
    if(NUM2UINT(rb_funcall(rb_iv_get(self, "@methods"), rb_intern("size"), 0)) > 0xffff){

        rb_bug("too many methods for this version");
    }

    if(rb_obj_is_kind_of(methodID, ConstMethodRequest) == Qtrue){

        rb_ary_push(rb_iv_get(self, "@methods"), methodID);
    }
    else{
        
        rb_raise(rb_eArgError, "expecting a kind_of MethodRequest");
    }

    return Qtrue;
}

static VALUE responseHandler(VALUE self, VALUE receiver, VALUE handler)
{
    rb_iv_set(self, "@receiver", receiver);
    rb_iv_set(self, "@responseHandler", handler);

    return Qtrue;
}

static VALUE clientInitialize(VALUE self, VALUE confirmed, VALUE breakOnError, VALUE block)
{
    if((confirmed == Qnil) || (confirmed == Qfalse)){

        rb_iv_set(self, "@confirmed", Qfalse);
    }
    else{

        rb_iv_set(self, "@confirmed", Qtrue);
    }

    if((breakOnError == Qnil) || (breakOnError == Qfalse)){

        rb_iv_set(self, "@breakOnError", Qfalse);
    }
    else{

        rb_iv_set(self, "@breakOnError", Qtrue);
    }

    rb_iv_set(self, "@methods", rb_ary_new());

    if(block != Qnil){

        rb_funcall(self, rb_intern("instance_exec"), 1, block);
    }
    else{

        rb_raise(rb_eException, "must pass a block");
    }

    if(NUM2UINT(rb_funcall(rb_iv_get(self, "@methods"), rb_intern("size"), 0)) == 0){

        rb_raise(rb_eException, "must define at least one method");
    }
    
    if(rb_iv_get(self, "@responseHandler") == Qnil){

        rb_raise(rb_eException, "must define a response handler");
    }

    rb_iv_set(self, "@uuid", rb_funcall(ConstSecureRandom, rb_intern("uuid"), 0));

    return self;
}

static void clientResultCallback(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    uint16_t i;

    VALUE self = SelfFromWrapper(r);
    VALUE resList = rb_ary_new();

    assert(self != Qnil);
    
    for(i=0; i < listSize; i++){

        if(list[i].result == PR6_CLIENT_RESULT_SUCCESS){

            VALUE arg[] = {
                rb_funcall(ConstMethodRequest, rb_intern("new"), 3, UINT2NUM(list[i].objectID), UINT2NUM(list[i].methodIndex), rb_str_new((const char *)list[i].arg, list[i].argLen)),
                resultToSymbol(list[i].result),
                rb_str_new((const char *)list[i].returnValue, list[i].returnValueLen)
            };

            rb_ary_push(resList, rb_class_new_instance(sizeof(arg)/sizeof(VALUE), arg, ConstMethodResponse));
        }
        else{

            VALUE arg[] = {
                rb_funcall(ConstMethodRequest, ID2SYM(rb_intern("new")), 3, UINT2NUM(list[i].objectID), UINT2NUM(list[i].methodIndex), rb_str_new((const char *)list[i].arg, list[i].argLen)),
                resultToSymbol(list[i].result),
                Qnil                
            };
        
            rb_ary_push(resList, rb_class_new_instance(sizeof(arg)/sizeof(VALUE), arg, ConstMethodResponse));
        }
    }

    VALUE receiver = rb_iv_get(self, "@receiver");
    VALUE handler = rb_iv_get(self, "@responseHandler");
    VALUE args[] = {self, resList};

    if(receiver != Qnil){

        rb_funcall_with_block(receiver, rb_intern("instance_exec"), sizeof(args)/sizeof(*args), args, handler);
    }
    else{

        rb_funcall(handler, rb_intern("call"), 2, self, resList);            
    }
}


static VALUE clientInput(VALUE self, VALUE msg)
{
    struct pr6_client *r;

    struct state_wrapper *wrapper;
    Data_Get_Struct(self, struct state_wrapper, wrapper);
    r = &wrapper->state.client;
    assert(wrapper->self == self);

    initClientState(r, rb_iv_get(self, "@confirmed"), rb_iv_get(self, "@breakOnError"), rb_iv_get(self, "@methods"));

    const uint8_t *in = (const uint8_t *)RSTRING_PTR(msg);
    uint32_t inLen = RSTRING_LEN(msg);

    if(rb_iv_get(self, "@counter") == Qnil){

        PR6_ClientInput(r, NUM2UINT(rb_iv_get(self, "@counter")), in, inLen);
    }
    else{

        rb_raise(rb_eException, "no counter has been registered with this client");
    }
        
    return self;
}

static VALUE clientOutput(VALUE self, VALUE outMax)
{
    struct pr6_client *r;
    uint16_t _outMax;
    
    if(NUM2UINT(outMax) > 0xffff){

        _outMax = 0xffff;
    }
    else{

        _outMax = NUM2UINT(outMax);
    }

    uint8_t *out = ALLOC_N(uint8_t, _outMax);
    
    struct state_wrapper *wrapper;
    Data_Get_Struct(self, struct state_wrapper, wrapper);
    r = &wrapper->state.client;
    assert(wrapper->self == self);

    initClientState(r, rb_iv_get(self, "@confirmed"), rb_iv_get(self, "@breakOnError"), rb_iv_get(self, "@methods"));
    
    return rb_str_new((const char *)out, PR6_ClientOutput(r, out, _outMax));
}

static VALUE resultToSymbol(enum pr6_result result)
{
    VALUE sym = rb_ary_entry(rb_const_get(rb_define_module("Wrangle"), rb_intern("PR6_RESULT")), (int)result);

    if(sym == Qnil){

        rb_bug("PR6_RESULT is out of alignment with enum pr6_result");
    }

    return sym;
}

static void initClientState(struct pr6_client *r, VALUE confirmed, VALUE breakOnError, VALUE requests)
{
    uint16_t i;
    uint16_t poolMax = NUM2UINT(rb_funcall(requests, rb_intern("size"), 0));

    bool c = (confirmed == Qtrue) ? true : false;
    bool boe = (breakOnError == Qtrue) ? true : false;
    struct pr6_client_req_res *pool = ALLOC_N(struct pr6_client_req_res, poolMax);
    
    for(i=0; i < poolMax; i++){

        VALUE req = rb_ary_entry(requests, i);

        uint16_t objectID = NUM2UINT(rb_funcall(req, rb_intern("objectID"), 0));
        uint8_t methodIndex = NUM2UINT(rb_funcall(req, rb_intern("methodIndex"), 0));
        const uint8_t *arg = (const uint8_t *)RSTRING_PTR(rb_funcall(req, rb_intern("argument"), 0));
        uint32_t argLen = RSTRING_LEN(rb_funcall(req, rb_intern("argument"), 0));

        if(i == 0){

            if(PR6_ClientInit(r, pool, poolMax, c, boe, clientResultCallback, objectID, methodIndex, arg, argLen) != r){

                rb_bug("could not add method invocation");
            }
        }
        else{
    
            if(PR6_ClientInit_AddMethod(r, objectID, methodIndex, arg, argLen) != r){

                rb_bug("could not add method invocation");
            }
        }
    }
}

static VALUE setCounter(VALUE self, VALUE counter)
{
    VALUE input = rb_funcall(counter, rb_intern("to_i"), 0);
    rb_iv_set(self, "@counter", UINT2NUM(NUM2UINT(input) & 0xffff));
    return self;
}

static VALUE getCounter(VALUE self)
{
    return rb_iv_get(self, "@counter");
}
