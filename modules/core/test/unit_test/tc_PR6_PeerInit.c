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

