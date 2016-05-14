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

void test_PR6_GetVint_7f(void)
{
    uint8_t retval;
    uint16_t size;
    uint32_t expected = 0x7fU;
    const uint8_t in[] = {0x7fU};

    retval = PR6_GetVint(in, sizeof(in), &size);
    TEST_ASSERT_EQUAL(sizeof(in), retval);
    TEST_ASSERT_EQUAL(expected, size);    
}

void test_PR6_GetVint_7f_boundary(void)
{
    uint8_t retval;
    uint16_t size;
    const uint8_t in[] = {0x7fU};

    retval = PR6_GetVint(in, sizeof(in)-1U, &size);
    TEST_ASSERT_EQUAL(0U, retval); 
}

void test_PR6_GetVint_ff(void)
{
    uint8_t retval;
    uint16_t size;
    uint32_t expected = 0xffU;
    const uint8_t in[] = {0x81U, 0xffU};

    retval = PR6_GetVint(in, sizeof(in), &size);
    TEST_ASSERT_EQUAL(sizeof(in), retval);
    TEST_ASSERT_EQUAL(expected, size);    
}

void test_PR6_GetVint_ff_boundary(void)
{
    uint8_t retval;
    uint16_t size;
    const uint8_t in[] = {0x81U, 0xffU};

    retval = PR6_GetVint(in, sizeof(in)-1U, &size);
    TEST_ASSERT_EQUAL(0U, retval); 
}

void test_PR6_GetVint_ff_zeroMSB(void)
{
    uint8_t retval;
    uint16_t size;
    const uint8_t in[] = {0x81U, 0x7fU};

    retval = PR6_GetVint(in, sizeof(in), &size);
    TEST_ASSERT_EQUAL(0U, retval); 
}

void test_PR6_GetVint_eeff(void)
{
    uint8_t retval;
    uint16_t size;
    uint32_t expected = 0xeeffU;
    const uint8_t in[] = {0x82U, 0xeeU, 0xffU};

    retval = PR6_GetVint(in, sizeof(in), &size);
    TEST_ASSERT_EQUAL(sizeof(in), retval);
    TEST_ASSERT_EQUAL(expected, size);    
}

void test_PR6_GetVint_eeff_boundary(void)
{
    uint8_t retval;
    uint16_t size;
    const uint8_t in[] = {0x82U, 0xeeU, 0xffU};

    retval = PR6_GetVint(in, sizeof(in)-1U, &size);
    TEST_ASSERT_EQUAL(0U, retval); 
}

void test_PR6_GetVint_eeff_zeroMSB(void)
{
    uint8_t retval;
    uint16_t size;
    const uint8_t in[] = {0x82U, 0x00U, 0xffU};

    retval = PR6_GetVint(in, sizeof(in), &size);
    TEST_ASSERT_EQUAL(0U, retval);     
}

