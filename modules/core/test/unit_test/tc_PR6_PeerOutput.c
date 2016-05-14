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

static bool testOutputCounterResult;
static bool testInputCounterResult;
static bool testEncryptResult;
static bool testDecryptResult;

static bool testOutputCounter(const uint8_t *entityID, const uint8_t *remoteID, uint32_t *counter)
{
    *counter = 1;
    return testOutputCounterResult;
}

static bool testInputCounter(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, uint32_t counter)
{
    return testInputCounterResult;
}

static bool testEncrypt(const uint8_t *entityID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, uint8_t *mac128)
{
    memset(mac128, 0xff, PR6_SIZE_GCM_MAC128);
    return testEncryptResult;
}

static bool testDecrypt(const uint8_t *entityID, const uint8_t *localID, const uint8_t *remoteID, const uint8_t *iv, uint8_t ivLen, uint8_t *text, uint16_t textLen, const uint8_t *aad, uint32_t aadLen, const uint8_t *mac128)
{    
    return testDecryptResult;
}

void setUp(void)
{
    struct pr6_peer_init param = {
        .decrypt = testDecrypt,
        .encrypt = testEncrypt,
        .inputCounter = testInputCounter,
        .outputCounter = testOutputCounter,
    };

    testOutputCounterResult = true;
    testInputCounterResult = true;
    testEncryptResult = true;
    testDecryptResult = true;

    PR6_PeerInit(&param);   
}

void tearDown(void)
{
}

void test_PR6_PeerOuput(void)
{
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    const uint8_t remoteID[] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    const uint8_t in[] = {
        (uint8_t)PR6_METHOD_REQ,
            0x00, 0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t expectedOut[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0x00,0x00,0x00,0x01,
        0, sizeof(in),
            (uint8_t)PR6_METHOD_REQ,
                0x00,
                    0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;
    uint32_t counter;
    
    retval = PR6_PeerOutput(entityID, remoteID, in, sizeof(in), out, sizeof(out), &counter);
    
    TEST_ASSERT_EQUAL(sizeof(expectedOut), retval);
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, retval);     
    TEST_ASSERT_EQUAL(testOutputCounterResult, counter);     
}

void test_PR6_PeerOuput_inputTooShort(void)
{
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    const uint8_t remoteID[] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    const uint8_t in[] = {
        (uint8_t)PR6_METHOD_REQ,        
    };
    
    uint8_t out[10U];
    uint32_t retval;
    uint32_t counter;
    
    retval = PR6_PeerOutput(entityID, remoteID, in, sizeof(in), out, sizeof(out), &counter);
    
    TEST_ASSERT_EQUAL(0U, retval);    
}

void test_PR6_PeerOuput_outputBufferTooShort(void)
{
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    const uint8_t remoteID[] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
        const uint8_t in[] = {
        (uint8_t)PR6_METHOD_REQ,
        0x00,
            0x00,
                0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t expectedOut[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0x00,0x00,0x00,0x01,
        0, sizeof(in),
            (uint8_t)PR6_METHOD_REQ,
            0x00,
                0x00,
                    0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)-1U];
    uint32_t retval;
    uint32_t counter;
    
    retval = PR6_PeerOutput(entityID, remoteID, in, sizeof(in), out, sizeof(out), &counter);
    
    TEST_ASSERT_EQUAL(0U, retval);    
}

void test_PR6_PeerOuput_outputCounterFailure(void)
{
    testOutputCounterResult = false;

    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    const uint8_t remoteID[] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    const uint8_t in[] = {
        (uint8_t)PR6_METHOD_REQ,
        0x00,
            0x00,
                0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t expectedOut[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0x00,0x00,0x00,0x01,
        0, sizeof(in),
            (uint8_t)PR6_METHOD_REQ,
            0x00,
                0x00,
                    0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;
    uint32_t counter;
    
    retval = PR6_PeerOutput(entityID, remoteID, in, sizeof(in), out, sizeof(out), &counter);
    
    TEST_ASSERT_EQUAL(0U, retval);    
}

void test_PR6_PeerOuput_encryptionFailure(void)
{
    testEncryptResult = false;

    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    const uint8_t remoteID[] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    const uint8_t in[] = {
        (uint8_t)PR6_METHOD_REQ,
        0x00,
            0x00,
                0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t expectedOut[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0x00,0x00,0x00,0x01,
        0, sizeof(in),
            (uint8_t)PR6_METHOD_REQ,
            0x00,
                0x00,
                    0x00, 0x00, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;
    uint32_t counter;
    
    retval = PR6_PeerOutput(entityID, remoteID, in, sizeof(in), out, sizeof(out), &counter);
    
    TEST_ASSERT_EQUAL(0U, retval);    
}
