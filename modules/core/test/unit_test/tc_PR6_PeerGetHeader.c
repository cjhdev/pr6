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
