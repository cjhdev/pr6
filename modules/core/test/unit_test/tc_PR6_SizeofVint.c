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
