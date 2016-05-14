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
