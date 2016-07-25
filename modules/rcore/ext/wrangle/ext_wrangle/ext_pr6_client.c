/* Copyright (c) 2013-2016 Cameron Harper
 *
 *
 * */

/* includes ***********************************************************/

#include <ruby.h>
#include <assert.h>
#include <stddef.h>

#include "pr6_client.h"

/* static variables ***************************************************/

static VALUE ConstMethodRequest;
static VALUE ConstMethodResponse;
static VALUE ConstClient;

/* static function prototypes *****************************************/

static VALUE clientInitialize(int argc, VALUE* argv, VALUE self);
static VALUE clientInput(VALUE self, VALUE msg);
static VALUE clientOutput(int argc, VALUE* argv, VALUE self);
static VALUE clientConfirmed(VALUE self);
static VALUE clientBreakOnError(VALUE self);
static VALUE clientState(VALUE self);
static VALUE clientTimeout(VALUE self);
static VALUE clientCancel(VALUE self);
static VALUE clientOutputConfirm(VALUE self, VALUE counter);

static VALUE clientPeekCounter(VALUE self, VALUE msg);

static void clientResultCallback(void *ctxt, struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list);
static VALUE resultToSymbol(enum pr6_client_result result);

static VALUE alloc(VALUE klass);

/* functions **********************************************************/

void EXT_PR6_ClientInit(void)
{
    VALUE wrangle = rb_define_module("Wrangle");
    ConstClient = rb_define_class_under(wrangle, "Client", rb_cObject);
    
    rb_define_method(ConstClient, "initialize", clientInitialize, -1);
    rb_define_method(ConstClient, "input", clientInput, 1);
    rb_define_method(ConstClient, "output", clientOutput, -1);    
    rb_define_method(ConstClient, "outputConfirm", clientOutputConfirm, 1);    
    rb_define_method(ConstClient, "timeout", clientTimeout, 0);
    rb_define_method(ConstClient, "cancel", clientCancel, 0);
    rb_define_method(ConstClient, "state", clientState, 0);
    rb_define_method(ConstClient, "confirmed?", clientConfirmed, 0);
    rb_define_method(ConstClient, "breakOnError?", clientBreakOnError, 0);
    
    rb_define_module_function(ConstClient, "peekCounter", clientPeekCounter, 1);
    
    rb_require("wrangle/method_response");
    rb_require("wrangle/method_request");
    
    ConstMethodResponse = rb_const_get(wrangle, rb_intern("MethodResponse"));
    ConstMethodRequest = rb_const_get(wrangle, rb_intern("MethodRequest"));

    rb_define_alloc_func(ConstClient, alloc); 
}

/* static functions ***************************************************/

static VALUE clientInitialize(int argc, VALUE* argv, VALUE self)
{
    VALUE requestList = Qnil;
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);
    bool cnf;
    bool boe;
    VALUE responseHandler;
    VALUE opts;
    
    rb_scan_args(argc, argv, "10:&", &requestList, &opts, &responseHandler);

    if(opts == Qnil){
        opts = rb_hash_new();
    }

    if(responseHandler == Qnil){

        rb_raise(rb_eArgError, "must pass &responseHandler");
    }

    VALUE receiver = rb_hash_aref(opts, ID2SYM(rb_intern("receiver")));
    VALUE confirmed = rb_hash_aref(opts, ID2SYM(rb_intern("confirmed")));
    VALUE breakOnError = rb_hash_aref(opts, ID2SYM(rb_intern("breakOnError")));    

    if((confirmed == Qnil) || (confirmed == Qtrue)){
        cnf = true;
    }
    else{
        cnf = false;
    }

    if((breakOnError == Qnil) || (breakOnError == Qfalse)){
        boe = false;
    }
    else{
        boe = true;
    }

    uint16_t i;
    uint16_t poolMax = NUM2UINT(rb_funcall(requestList, rb_intern("size"), 0));
    struct pr6_client_req_res *pool = ALLOC_N(struct pr6_client_req_res, poolMax);

    for(i=0; i < poolMax; i++){

        VALUE req = rb_ary_entry(requestList, i);

        uint16_t objectID = NUM2UINT(rb_funcall(req, rb_intern("objectID"), 0));
        uint8_t methodIndex = NUM2UINT(rb_funcall(req, rb_intern("methodIndex"), 0));
        const uint8_t *arg = (const uint8_t *)RSTRING_PTR(rb_funcall(req, rb_intern("argument"), 0));
        uint32_t argLen = RSTRING_LEN(rb_funcall(req, rb_intern("argument"), 0));

        if(i == 0){

            if(PR6_ClientInit(r, pool, poolMax, cnf, boe, clientResultCallback, objectID, methodIndex, arg, argLen) != r){

                rb_bug("could not add method invocation");
            }
        }
        else{
    
            if(PR6_ClientInit_AddMethod(r, objectID, methodIndex, arg, argLen) != r){

                rb_bug("could not add method invocation");
            }
        }
    }

    rb_iv_set(self, "@requestList", requestList);
    rb_iv_set(self, "@receiver", receiver);
    rb_iv_set(self, "@responseHandler", responseHandler);
    
    return self;
}

static VALUE clientInput(VALUE self, VALUE msg)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);
    
    const uint8_t *in = (const uint8_t *)RSTRING_PTR(msg);
    uint16_t inLen = RSTRING_LEN(msg);

    PR6_ClientInput(&self, r, in, inLen);
    
    return self;
}

static VALUE clientOutput(int argc, VALUE* argv, VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);
    VALUE outMax;    
    uint16_t _outMax = 0xffff;
    
    rb_scan_args(argc, argv, "01", &outMax);

    if(outMax != Qnil){

        outMax = rb_funcall(outMax, rb_intern("to_i"), 0);
        
        if((NUM2UINT(outMax) < 0xffff) && (NUM2UINT(outMax) > 0)){

            _outMax = NUM2UINT(outMax);
        }
    }

    uint8_t *out = ALLOC_N(uint8_t, _outMax);

    return rb_str_new((const char *)out, PR6_ClientOutput(r, out, _outMax));
}

static VALUE clientOutputConfirm(VALUE self, VALUE counter)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);

    counter = rb_funcall(counter, rb_intern("to_i"), 0);

    PR6_ClientOutputConfirm(&self, r, NUM2UINT(counter));

    return self;
}

static VALUE clientState(VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);
    enum pr6_client_state state;
    
    VALUE symbols[] = {
        ID2SYM(rb_intern("PR6_CLIENT_STATE_INIT")),
        ID2SYM(rb_intern("PR6_CLIENT_STATE_PENDING")),
        ID2SYM(rb_intern("PR6_CLIENT_STATE_SEND")),
        ID2SYM(rb_intern("PR6_CLIENT_STATE_COMPLETE"))
    };

    state = PR6_ClientState(r);

    assert(((int)state) < (sizeof(symbols)/sizeof(*symbols)));

    return symbols[(int)state];
}

static VALUE clientConfirmed(VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);

    return PR6_ClientIsConfirmed(r) ? Qtrue : Qfalse;
}

static VALUE clientBreakOnError(VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);

    return PR6_ClientIsBreakOnError(r) ? Qtrue : Qfalse;
}

static VALUE clientTimeout(VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);

    PR6_ClientTimeout(&self, r);

    return self;
}

static VALUE clientCancel(VALUE self)
{
    struct pr6_client *r;
    Data_Get_Struct(self, struct pr6_client, r);

    PR6_ClientCancel(&self, r);

    return self;
}

static VALUE resultToSymbol(enum pr6_client_result result)
{
    VALUE symbols[] = {
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_SUCCESS")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_OBJECT_UNDEFINED")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_METHOD_UNDEFINED")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_ACCESS_DENIED")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_ARGUMENT")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_PERMANENT")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_TEMPORARY")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_MISSING")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_TIMEOUT")),
        ID2SYM(rb_intern("PR6_CLIENT_RESULT_CANCELLED"))
    };

    assert(((int)result) < (sizeof(symbols)/sizeof(*symbols)));

    return symbols[(int)result];
}

static VALUE clientPeekCounter(VALUE self, VALUE msg)
{
    VALUE retval = Qnil;
    uint16_t counter;
    msg = rb_funcall(msg, rb_intern("to_s"), 0);
    
    if(PR6_ClientPeekCounter((const uint8_t *)RSTRING_PTR(msg), RSTRING_LEN(msg), &counter) > 0U){
        retval = UINT2NUM(counter);
    }

    return retval;
}

static void clientResultCallback(void *ctxt, struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    uint16_t i;

    VALUE self = *(VALUE *)ctxt;

    VALUE resList = rb_ary_new();
    VALUE methods = rb_iv_get(self, "@requestList");

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

static VALUE alloc(VALUE klass)
{
    VALUE obj = Qnil;
    void *ptr = calloc(1, sizeof(struct pr6_client));

    if(ptr != NULL){

        obj = Data_Wrap_Struct(klass, 0, free, ptr);
    }
    else{

        rb_sys_fail("calloc()");
    }

    return obj;
}

