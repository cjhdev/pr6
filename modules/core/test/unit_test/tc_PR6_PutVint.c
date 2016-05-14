#include "unity.h"
#include "pr6_encoder_decoder.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_PR6_PutVint_7f(void)
{
    uint8_t retval;
    uint32_t in = 0x7fU;
    uint8_t expected[] = {0x7fU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out));
    TEST_ASSERT_EQUAL(sizeof(expected), retval);
    TEST_ASSERT_EQUAL_MEMORY(expected, out, sizeof(expected));
}

void test_PR6_PutVint_7f_boundary(void)
{
    uint8_t retval;
    uint32_t in = 0x7fU;
    uint8_t expected[] = {0x7fU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out)-1);
    TEST_ASSERT_EQUAL(0U, retval);  
}

void test_PR6_PutVint_ff(void)
{
    uint8_t retval;
    uint32_t in = 0xffU;
    uint8_t expected[] = {0x81U, 0xffU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out));
    TEST_ASSERT_EQUAL(sizeof(expected), retval);
    TEST_ASSERT_EQUAL_MEMORY(expected, out, sizeof(expected));
}

void test_PR6_PutVint_ff_boundary(void)
{
    uint8_t retval;
    uint32_t in = 0xffU;
    uint8_t expected[] = {0x81U, 0x7fU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out)-1);
    TEST_ASSERT_EQUAL(0U, retval);  
}

void test_PR6_PutVint_eeff(void)
{
    uint8_t retval;
    uint32_t in = 0xeeffU;
    uint8_t expected[] = {0x82U, 0xeeU, 0xffU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out));
    TEST_ASSERT_EQUAL(sizeof(expected), retval);
    TEST_ASSERT_EQUAL_MEMORY(expected, out, sizeof(expected));
}

void test_PR6_PutVint_eeff_boundary(void)
{
    uint8_t retval;
    uint32_t in = 0xeeffU;
    uint8_t expected[] = {0x82U, 0xeeU, 0x7fU};
    uint8_t out[sizeof(expected)];
    
    retval = PR6_PutVint(in, out, sizeof(out)-1);
    TEST_ASSERT_EQUAL(0U, retval);  
}

