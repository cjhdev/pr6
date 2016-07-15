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

void setUp(void)
{
    cbResultTouch = 0;
    
    static struct pr6_client_req_res pool[4U];
    static uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = true;
    bool breakOnError = false;

    PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello"));
}

void tearDown(void)
{
}

void test_PR6_ClientOutput(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_PR6_ClientOutput_multiple(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 2, (const uint8_t *)"again", strlen("again")));
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 3, (const uint8_t *)"helloworld", strlen("helloworld")));
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 4, (const uint8_t *)"hello", strlen("hello")));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o',
            0x00, 42,
            2,
            5, 'a','g','a','i','n',            
            0x00, 42,
            3,
            10, 'h','e','l','l','o','w','o','r','l','d',            
            0x00, 42,
            4,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientOutput_outputTooShort(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)-1];

    TEST_ASSERT_EQUAL(0U, PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));    
}

void test_PR6_ClientOutput_retry(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
    
    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientOutput_nonConfirmed(void)
{
    static struct pr6_client_req_res pool[4U];
    static uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = false;
    bool breakOnError = false;
    
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_NC_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o',            
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(1, cbResultTouch);
    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsComplete(&client));
}
