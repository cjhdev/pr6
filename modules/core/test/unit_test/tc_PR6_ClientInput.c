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
#include "pr6_client.h"
#include <string.h>

int cbResultTouch;

static void cbResult(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    cbResultTouch++;

    TEST_ASSERT_TRUE(listSize > 0);
    TEST_ASSERT_EQUAL(listSize, r->listSize);
    TEST_ASSERT_EQUAL(list, r->list);
}

static struct pr6_client client;    

/* initialise a client instance and have it produce a METHOD-COMPLETE.req */
void setUp(void)
{
    cbResultTouch = 0;

    const uint8_t expOutput[] = {PR6_METHOD_REQ, 0x00, 42, 1, 5, 'h', 'e', 'l', 'l', 'o'};        
    uint8_t out[sizeof(expOutput)];
    
    static struct pr6_client_req_res reqResPool[1U];    
    struct pr6_client_init in = {

        .reqResPool = reqResPool,
        .reqResPoolMax = sizeof(reqResPool) / sizeof(*reqResPool),

        .confirmed = true,
        .breakOnError = false,

        .cbResult = cbResult,
    };

    PR6_ClientInit(&client, &in, 42, 1, (const uint8_t *)"hello", strlen("hello"));
    PR6_ClientOutput(&client, out, sizeof(out));
}

void tearDown(void)
{
}

void test_PR6_ClientInput(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd'
    };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(1U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsComplete(&client));    
}

void test_PR6_ClientInput_listSizeMismatch(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd',
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd'
        };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_expectedCounterMismatch(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd',            
        };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 2, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_padded(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd', 0x00
        };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_boundaryCounter(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES            
    };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_boundaryResult(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
    };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_boundaryReturnValueEncoding(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS
    };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientInput_boundaryReturnValue(void)
{
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world")
    };

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(0U , cbResultTouch);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

