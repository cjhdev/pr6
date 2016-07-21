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

#include "ext_common.h"

#include <malloc.h>

/* externs ************************************************************/

void EXT_PR6_ClientInit(void);
void EXT_PR6_ServerInit(void);

void EXT_PR6_PeerInit(void);

/* functions **********************************************************/

/* this is the library entry point used by Ruby */
void Init_ext_wrangle(void)
{
    EXT_PR6_ServerInit();
    EXT_PR6_ClientInit();
    EXT_PR6_PeerInit();
}

VALUE StateWrapperAlloc(VALUE klass)
{
    VALUE obj = Qnil;
    void *ptr = calloc(1, sizeof(struct state_wrapper));

    if(ptr != NULL){

        obj = Data_Wrap_Struct(klass, 0, free, ptr);
        ((struct state_wrapper *)ptr)->self = obj;
    }
    else{

        rb_sys_fail("calloc()");
    }

    return obj;
}

VALUE SelfFromWrapper(const void *ptr)
{
    return ((struct state_wrapper *)((size_t)ptr - offsetof(struct state_wrapper, state)))->self;
}
