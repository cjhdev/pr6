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

void test_PR6_PeerInput(void)
{
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 0x00,
            0x00,
            5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            PR6_METHOD_REQ,
                0x00, 0x00,
                0x00,
                5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    const struct pr6_peer_header expectedHeader = {
        {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff},
    };
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(sizeof(expectedOut), retval);
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, retval);     
    TEST_ASSERT_EQUAL(PR6_RECIPIENT_SERVER, recipient);     
    TEST_ASSERT_EQUAL(testOutputCounterResult, counter);     
    TEST_ASSERT_EQUAL_MEMORY(expectedHeader.to, header.to, PR6_SIZE_ENTITY_ID);     
    TEST_ASSERT_EQUAL_MEMORY(expectedHeader.from, header.from, PR6_SIZE_ENTITY_ID);     
}

void test_PR6_PeerInput_clientRecipient(void)
{
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        PR6_METHOD_RES,
            0x00, 0x01,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            PR6_METHOD_RES,
                0x00,0x01,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(sizeof(expectedOut), retval);
    TEST_ASSERT_EQUAL(PR6_RECIPIENT_CLIENT, recipient);     
    TEST_ASSERT_EQUAL(1, counter);         
}

void test_PR6_PeerInput_messageTooShort(void)
{
    testDecryptResult = false;
    
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            PR6_METHOD_RES,
                0x00,0x01,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(0, retval);;            
}

void test_PR6_PeerInput_decryptFailure(void)
{
    testDecryptResult = false;
    
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        (uint8_t)PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            (uint8_t)PR6_METHOD_RES,
                0x00,0x01,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(0, retval);    
}

void test_PR6_PeerInput_inputCounterFailure(void)
{
    testInputCounterResult = false;
    
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        (uint8_t)PR6_METHOD_RES,
            0x00,0x01,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            (uint8_t)PR6_METHOD_RES,
                0x00,0x01,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(0, retval);    
}

void test_PR6_PeerInput_unknownPayloadTag(void)
{
    testInputCounterResult = false;
    
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        0xff,
            0x00,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            0xff,
            0x01,
                0x00,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(0, retval);    
}

void test_PR6_PeerInput_outputBufferTooShort(void)
{
    testInputCounterResult = false;
    
    const uint8_t entityID[] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
    
    const uint8_t expectedOut[] = {
        PR6_METHOD_RES,
            0x00,
                PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o'
    };
    uint8_t in[] = {
        0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
        0x00,0x00,0x00,0x01,
        0, sizeof(expectedOut),
            0xff,
            0x01,
                0x00,
                    PR6_RESULT_SUCCESS, 5, 'h', 'e', 'l', 'l', 'o',
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint8_t out[sizeof(expectedOut)-1U];
    uint32_t retval;    
    enum pr6_recipient recipient;
    uint32_t counter;
    struct pr6_peer_header header;
    
    retval = PR6_PeerInput(entityID, in, sizeof(in), out, sizeof(out), &header, &recipient, &counter);
    
    TEST_ASSERT_EQUAL(0, retval);    
}
