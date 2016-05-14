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

void test_PR6_ClientPeekCounter(void)
{
    uint16_t counter;
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
            PR6_RESULT_SUCCESS,
            strlen("world"), 'w', 'o', 'r', 'l', 'd'
    };

    TEST_ASSERT_TRUE(PR6_ClientPeekCounter(input, sizeof(input), &counter) > 0);
    TEST_ASSERT_EQUAL(1, counter);
}

void test_PR6_ClientPeekCounter_boundary(void)
{
    uint16_t counter;
    const uint8_t input[] = {
        PR6_METHOD_RES,
            0x00, 
    };

    TEST_ASSERT_TRUE(PR6_ClientPeekCounter(input, sizeof(input), &counter) == 0);    
}
