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

static bool testOutputCounter(const uint8_t *entityID, const uint8_t *remoteID, uint32_t *counter)
{
    return false;
}

static bool testInputCounter(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, uint32_t counter)
{
    return false;
}

static bool testEncrypt(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, uint8_t *mac128)
{
    return false;
}

static bool testDecrypt(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, const uint8_t *mac128)
{
    return false;
}

void test_PR6_PeerInit(void)
{
    struct pr6_peer_init param = {
        .decrypt = testDecrypt,
        .encrypt = testEncrypt,
        .inputCounter = testInputCounter,
        .outputCounter = testOutputCounter,
    };

    PR6_PeerInit(&param);    
}

