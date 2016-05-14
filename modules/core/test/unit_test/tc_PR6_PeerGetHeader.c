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
#include "pr6_peer.h"

#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_PR6_PeerGetHeader(void)
{
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,        
    };
    struct pr6_peer_header out;
    struct pr6_peer_header expectedOut = {
        {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff}
    };
    
    TEST_ASSERT_EQUAL(sizeof(in), PR6_PeerGetHeader(in, sizeof(in), &out));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut.to, out.to, sizeof(out.to));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut.from, out.from, sizeof(out.from));
}

void test_PR6_PeerGetHeader_inputTooShort(void)
{
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
    };
    struct pr6_peer_header out;
        
    TEST_ASSERT_EQUAL(0U, PR6_PeerGetHeader(in, sizeof(in), &out));    
}
