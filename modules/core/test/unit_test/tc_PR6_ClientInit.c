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

void setUp(void)
{
}

void tearDown(void)
{
}

static void cbResult(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
}

/* initialise a client instance and access read-only attributes */
void test_PR6_ClientInit(void)
{
    struct pr6_client client;    
    struct pr6_client_req_res pool[1U];
    uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = true;
    bool breakOnError = false;

    /* assert that client instance can be initialised */
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello")));

    /* assert that breakOnError setting can be read from instance */
    TEST_ASSERT_EQUAL(false, PR6_ClientIsBreakOnError(&client));

    /* assert that confirmed setting can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsConfirmed(&client));

    /* assert that state can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));        
}

void test_PR6_ClientInit_AddMethod(void)
{
    struct pr6_client client;
    struct pr6_client_req_res pool[3U];
    uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = true;
    bool breakOnError = false;
    
    /* assert that client instance can be initialised */
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello")));
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));

    /* assert that breakOnError setting can be read from instance */
    TEST_ASSERT_EQUAL(false, PR6_ClientIsBreakOnError(&client));

    /* assert that confirmed setting can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsConfirmed(&client));

    /* assert that state can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));        
}

void test_PR6_ClientInit_AddMethod_afterSend(void)
{
    uint8_t out[20U];
    struct pr6_client client;
    struct pr6_client_req_res pool[1U];
    uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = true;
    bool breakOnError = false;
    
    /* assert that client instance can be initialised */
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello")));
    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));        
    
    TEST_ASSERT_TRUE(PR6_ClientOutput(&client, out, sizeof(out)) > 0U);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    TEST_ASSERT_EQUAL_PTR(NULL, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
}

void test_PR6_ClientInit_AddMethod_afterReceive(void)
{
    uint8_t out[20U];    
    struct pr6_client client;
    struct pr6_client_req_res pool[1U];
    uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    bool confirmed = true;
    bool breakOnError = false;
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x00,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd'
    };

    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit(&client, pool, poolMax, confirmed, breakOnError, cbResult, 42, 1, (const uint8_t *)"hello", strlen("hello")));
    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));        
    
    TEST_ASSERT_TRUE(PR6_ClientOutput(&client, out, sizeof(out)) > 0U);

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientInput(&client, 0, input, sizeof(input));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsComplete(&client));

    TEST_ASSERT_EQUAL_PTR(NULL, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
}

void test_PR6_ClientInit_FromMessage(void)
{
    struct pr6_client_req_res pool[4U];
    uint16_t poolMax = sizeof(pool) / sizeof(*pool);
    struct pr6_client client;    
    
    const uint8_t msg[] = {
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

    /* init from message */
    TEST_ASSERT_EQUAL_PTR(&client, PR6_ClientInit_FromMessage(&client, pool, poolMax, cbResult, msg, sizeof(msg)));

    /* assert that breakOnError setting can be read from instance */
    TEST_ASSERT_EQUAL(false, PR6_ClientIsBreakOnError(&client));

    /* assert that confirmed setting can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsConfirmed(&client));

    /* assert that state can be read from instance */
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));        

}

