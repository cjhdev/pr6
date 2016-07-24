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

#include "unity.h"
#include "pr6_server.h"

#include <string.h>

static struct pr6_server r;
static uint16_t objectID;
static uint8_t methodIndex;
static uint8_t role;
static uint32_t adapterCalled = 0U;
static bool yield;

static enum pr6_adapter_result serverObjectInterface(void *ctxt, const struct pr6_server *r, struct pr6_server_adapter *arg, enum pr6_result *result)
{
    adapterCalled++;

    enum pr6_adapter_result retval = PR6_ADAPTER_SUCCESS;

    if(arg->objectID == objectID){

        if(arg->methodIndex == methodIndex){

            if((arg->roleSize > 0U) && (*arg->role == role)){

                if(arg->outMax >= arg->inLen){

                    if(yield){  

                        retval = PR6_ADAPTER_YIELD;
                    }
                    else{

                        memcpy(arg->out, arg->in, arg->inLen);
                        arg->outLen = arg->inLen;
                        *result = PR6_RESULT_SUCCESS;
                    }
                }
                else{

                    retval = PR6_ADAPTER_BUFFER;
                }
            }
            else{

                *result = PR6_RESULT_ACCESS_DENIED;
            }
        }
        else{

            *result = PR6_RESULT_METHOD_UNDEFINED;
        }
    }
    else{

        *result = PR6_RESULT_OBJECT_UNDEFINED;
    }

    return retval;
}

void setUp(void)
{
    adapterCalled = 0U;
    yield = false;
}

void tearDown(void)
{
}

void test_PR6_ServerInput(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o',
                PR6_RESULT_SUCCESS,
                0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o'
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(3, adapterCalled);
    TEST_ASSERT_EQUAL(sizeof(expOut), outLen);
    TEST_ASSERT_EQUAL_MEMORY(expOut, out, outLen);
}

void test_PR6_ServerInput_yield(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o',
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    yield = true;
    TEST_ASSERT_EQUAL(true, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(1, adapterCalled);

    yield = false;
    TEST_ASSERT_EQUAL(false, PR6_ServerResume(NULL, &r, out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(2, adapterCalled);
    TEST_ASSERT_EQUAL(sizeof(expOut), outLen);
    TEST_ASSERT_EQUAL_MEMORY(expOut, out, outLen);
}

void test_PR6_ServerInput_NonSuccess(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x01,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_OBJECT_UNDEFINED,
                PR6_RESULT_SUCCESS,
                0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o'
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(3, adapterCalled);
    TEST_ASSERT_EQUAL(sizeof(expOut), outLen);
    TEST_ASSERT_EQUAL_MEMORY(expOut, out, outLen);
}

void test_PR6_ServerInput_NonSuccessBOE(void)
{
        const uint8_t in[] = {
        PR6_METHOD_BOE_REQ,
            0x00,0x01,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_OBJECT_UNDEFINED,
                PR6_RESULT_PERMANENT,
                PR6_RESULT_PERMANENT,
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(1, adapterCalled);
    TEST_ASSERT_EQUAL(sizeof(expOut), outLen);
    TEST_ASSERT_EQUAL_MEMORY(expOut, out, outLen);
}

void test_PR6_ServerInput_nonSuccessNCBOE(void)
{
    const uint8_t in[] = {
        PR6_METHOD_NC_BOE_REQ,
            0x00,0x01,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_OBJECT_UNDEFINED,
                PR6_RESULT_PERMANENT,
                PR6_RESULT_PERMANENT,
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(1, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}

void test_PR6_ServerInput_NC(void)
{
    const uint8_t in[] = {
        PR6_METHOD_NC_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o',
                PR6_RESULT_SUCCESS,
                0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o'
    };
    uint8_t out[sizeof(expOut)];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(3, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}

void test_PR6_ServerInput_adapterBufferException(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t successOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o',
                PR6_RESULT_SUCCESS,
                0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o'
    };
    const uint8_t expOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS,
                0x05, 'h', 'e', 'l', 'l','o',
                PR6_RESULT_SUCCESS,
                0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
    };
    uint8_t out[sizeof(successOut)-1];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(3, adapterCalled);
    TEST_ASSERT_EQUAL(sizeof(expOut), outLen);
    TEST_ASSERT_EQUAL_MEMORY(expOut, out, outLen);
}

void test_PR6_ServerInput_outMaxBelowMinimum(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o',
            0x00,0x00,
            0x00,
            0x0a, 'h', 'e', 'l', 'l','o', 'w', 'o', 'r', 'l','d',
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l','o'
    };
    uint8_t out[1];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(0, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}

void test_PR6_ServerInput_boundaryMethodID(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
    };
    uint8_t out[10];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(0, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}

void test_PR6_ServerInput_boundaryArgumentSizeEncoding(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00
    };
    uint8_t out[10];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(0, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}
void test_PR6_ServerInput_boundaryArgument(void)
{
    const uint8_t in[] = {
        PR6_METHOD_REQ,
            0x00,0x00,
            0x00,
            0x05, 'h', 'e', 'l', 'l'
    };
    uint8_t out[10];
    uint16_t outLen;
    uint8_t role[] = {0};
    uint32_t counter = 1;

    TEST_ASSERT_EQUAL(false, PR6_ServerInput(NULL, &r, serverObjectInterface, role, sizeof(role), counter, in, sizeof(in), out, &outLen, sizeof(out)));
    TEST_ASSERT_EQUAL(0, adapterCalled);
    TEST_ASSERT_EQUAL(0, outLen);
}
