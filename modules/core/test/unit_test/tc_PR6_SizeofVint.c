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
#include "pr6_encoder_decoder.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_PR6_SizeofVint_one(void)
{
    uint8_t retval = PR6_SizeofVint(0x7f);
    TEST_ASSERT_EQUAL(1U, retval); 
}

void test_PR6_SizeofVint_two(void)
{
    uint8_t retval = PR6_SizeofVint(0x80);
    TEST_ASSERT_EQUAL(2U, retval); 
}

void test_PR6_SizeofVint_three(void)
{
    uint8_t retval = PR6_SizeofVint(0x100);
    TEST_ASSERT_EQUAL(3U, retval); 
}
