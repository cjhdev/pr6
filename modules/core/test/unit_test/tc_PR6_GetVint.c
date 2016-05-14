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

