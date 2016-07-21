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
static VALUE ConstClient;

/* static function prototypes *****************************************/

static VALUE clientInitialize(int argc, VALUE* argv, VALUE self);
static VALUE clientInput(VALUE self, VALUE expectedCounter, VALUE msg);
static VALUE clientOutput(int argc, VALUE* argv, VALUE self);
static void clientResultCallback(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list);
static VALUE resultToSymbol(enum pr6_client_result result);
static void initClientState(struct pr6_client *r, VALUE confirmed, VALUE breakOnError, VALUE requests);
static VALUE responseHandler(int argc, VALUE* argv, VALUE self);
static VALUE request(VALUE self, VALUE objectID, VALUE methodIndex, VALUE argument);
static VALUE clientTimeout(VALUE self);

/* functions **********************************************************/

void EXT_PR6_ClientInit(void)
{
    VALUE wrangle = rb_define_module("Wrangle");
    ConstClient = rb_define_class_under(wrangle, "Client", rb_cObject);

    rb_define_method(ConstClient, "initialize", clientInitialize, -1);
    rb_define_method(ConstClient, "input", clientInput, 2);
    rb_define_method(ConstClient, "output", clientOutput, -1);    
    rb_define_method(ConstClient, "timeout", clientTimeout, 0);    

    rb_define_private_method(ConstClient, "request", request, 3);
    rb_define_private_method(ConstClient, "response", responseHandler, -1);
    
    rb_require("wrangle/method_response");
    rb_require("wrangle/method_request");
    
    ConstMethodResponse = rb_const_get(wrangle, rb_intern("MethodResponse"));
    ConstMethodRequest = rb_const_get(wrangle, rb_intern("MethodRequest"));    
    
    rb_define_alloc_func(ConstClient, StateWrapperAlloc);   
}

/* static functions ***************************************************/

static VALUE request(VALUE self, VALUE objectID, VALUE methodIndex, VALUE argument)
{
    if(NUM2UINT(rb_funcall(rb_iv_get(self, "@methods"), rb_intern("size"), 0)) > 0xffff){

        rb_bug("too many methods defined");
    }

    rb_ary_push(rb_iv_get(self, "@methods"), rb_funcall(ConstMethodRequest, rb_intern("new"), 3, objectID, methodIndex, argument));
    
    return Qtrue;
}

static VALUE responseHandler(int argc, VALUE* argv, VALUE self)
{
    VALUE receiver;
    VALUE opts;
    VALUE handler;
    
    rb_scan_args(argc, argv, "01:&", &receiver, &opts, &handler);

    rb_iv_set(self, "@receiver", receiver);
    rb_iv_set(self, "@responseHandler", handler);

    return Qtrue;
}

static VALUE clientInitialize(int argc, VALUE* argv, VALUE self)
{
    VALUE opts;
    VALUE block;

    rb_scan_args(argc, argv, ":&", &opts, &block);

    if(opts == Qnil){
        opts = rb_hash_new();
    }

    rb_iv_set(self, "@responseHandler", Qnil);

    VALUE confirmed = rb_hash_aref(opts, ID2SYM(rb_intern("confirmed")));
    VALUE breakOnError = rb_hash_aref(opts, ID2SYM(rb_intern("breakOnError")));

    if((confirmed == Qnil) || (confirmed == Qtrue)){

        rb_iv_set(self, "@confirmed", Qtrue);
    }
    else{

        rb_iv_set(self, "@confirmed", Qfalse);
    }

    if((breakOnError == Qnil) || (breakOnError == Qfalse)){

        rb_iv_set(self, "@breakOnError", Qfalse);
    }
    else{

        rb_iv_set(self, "@breakOnError", Qtrue);
    }

    rb_iv_set(self, "@methods", rb_ary_new());

    if(block != Qnil){

        rb_funcall_with_block(self, rb_intern("instance_exec"), 0, NULL, block);
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

    return self;
}

static void clientResultCallback(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    uint16_t i;

    VALUE self = SelfFromWrapper(r);
    VALUE resList = rb_ary_new();
    VALUE methods = rb_iv_get(self, "@methods");

    assert(self != Qnil);
    
    for(i=0; i < listSize; i++){

        if(list[i].result == PR6_CLIENT_RESULT_SUCCESS){

            VALUE arg[] = {
                rb_ary_entry(methods, i),
                resultToSymbol(list[i].result),
                rb_str_new((const char *)list[i].returnValue, list[i].returnValueLen)
            };

            rb_ary_push(resList, rb_class_new_instance(sizeof(arg)/sizeof(VALUE), arg, ConstMethodResponse));
        }
        else{

            VALUE arg[] = {
                rb_ary_entry(methods, i),
                resultToSymbol(list[i].result)
            };
        
            rb_ary_push(resList, rb_class_new_instance(sizeof(arg)/sizeof(VALUE), arg, ConstMethodResponse));
        }
    }

    VALUE receiver = rb_iv_get(self, "@receiver");
    VALUE handler = rb_iv_get(self, "@responseHandler");
    VALUE args[] = {resList};

    
    if(handler != Qnil){
        
        if(receiver != Qnil){

            rb_funcall_with_block(receiver, rb_intern("instance_exec"), sizeof(args)/sizeof(*args), args, handler);
        }
        else{

            rb_funcallv(handler, rb_intern("call"), sizeof(args)/sizeof(*args), args);            
        }
    }
}

static VALUE clientInput(VALUE self, VALUE expectedCounter, VALUE msg)
{
    struct pr6_client *r;

    struct state_wrapper *wrapper;
    Data_Get_Struct(self, struct state_wrapper, wrapper);
    r = &wrapper->state.client;
    assert(wrapper->self == self);

    initClientState(r, rb_iv_get(self, "@confirmed"), rb_iv_get(self, "@breakOnError"), rb_iv_get(self, "@methods"));

    expectedCounter = rb_funcall(expectedCounter, rb_intern("to_i"), 0);

    const uint8_t *in = (const uint8_t *)RSTRING_PTR(msg);
    uint32_t inLen = RSTRING_LEN(msg);

    PR6_ClientInput(r, (uint16_t)NUM2UINT(expectedCounter), in, inLen);
    
    return self;
}

static VALUE clientOutput(int argc, VALUE* argv, VALUE self)
{
    VALUE outMax;
    struct pr6_client *r;
    uint16_t _outMax = 0xffff;

    rb_scan_args(argc, argv, "01", &outMax);

    if(outMax != Qnil){

        outMax = rb_funcall(outMax, rb_intern("to_i"), 0);
        
        if((NUM2UINT(outMax) < 0xffff) && (NUM2UINT(outMax) > 0)){

            _outMax = NUM2UINT(outMax);
        }
    }
    
    uint8_t *out = ALLOC_N(uint8_t, _outMax);

    struct state_wrapper *wrapper;
    Data_Get_Struct(self, struct state_wrapper, wrapper);
    r = &wrapper->state.client;
    assert(wrapper->self == self);

    initClientState(r, rb_iv_get(self, "@confirmed"), rb_iv_get(self, "@breakOnError"), rb_iv_get(self, "@methods"));
    
    return rb_str_new((const char *)out, PR6_ClientOutput(r, out, _outMax));
}

static VALUE resultToSymbol(enum pr6_client_result result)
{
    VALUE sym = rb_ary_entry(rb_const_get(rb_define_module("Wrangle"), rb_intern("PR6_CLIENT_RESULT")), (int)result);

    if(sym == Qnil){

        rb_bug("PR6_RESULT is out of alignment with enum pr6_client_result");
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

static VALUE clientTimeout(VALUE self)
{
    struct pr6_client *r;

    struct state_wrapper *wrapper;
    Data_Get_Struct(self, struct state_wrapper, wrapper);
    r = &wrapper->state.client;
    assert(wrapper->self == self);

    initClientState(r, rb_iv_get(self, "@confirmed"), rb_iv_get(self, "@breakOnError"), rb_iv_get(self, "@methods"));

    PR6_ClientTimeout(r);

    return self;
}
