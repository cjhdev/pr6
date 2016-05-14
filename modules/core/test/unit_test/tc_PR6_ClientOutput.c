#include "unity.h"
#include "pr6_client.h"
#include <string.h>

int cbResultTouch;

static void cbResult(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    cbResultTouch++;

    TEST_ASSERT_TRUE(listSize > 0);
    TEST_ASSERT_EQUAL(listSize, r->listSize);
    TEST_ASSERT_EQUAL(list, r->list);
}

static struct pr6_client client;

void setUp(void)
{
    cbResultTouch = 0;
    
    static struct pr6_client_req_res reqResPool[4U];
    static struct pr6_client_init in = {

        .reqResPool = reqResPool,
        .reqResPoolMax = sizeof(reqResPool) / sizeof(*reqResPool),

        .confirmed = true,
        .breakOnError = false,

        .cbResult = cbResult,
    };

    PR6_ClientInit(&client, &in, 42, 1, (const uint8_t *)"hello", strlen("hello"));
}

void tearDown(void)
{
}

void test_PR6_ClientOutput(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_PR6_ClientOutput_multiple(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 2, (const uint8_t *)"again", strlen("again")));
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 3, (const uint8_t *)"helloworld", strlen("helloworld")));
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 4, (const uint8_t *)"hello", strlen("hello")));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o',
            0x00, 42,
            2,
            5, 'a','g','a','i','n',            
            0x00, 42,
            3,
            10, 'h','e','l','l','o','w','o','r','l','d',            
            0x00, 42,
            4,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientOutput_outputTooShort(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)-1];

    TEST_ASSERT_EQUAL(0U, PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));    
}

void test_PR6_ClientOutput_retry(void)
{
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
    
    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));
}

void test_PR6_ClientOutput_nonConfirmed(void)
{
    static struct pr6_client_req_res reqResPool[2U];
    static struct pr6_client_init in = {

        .reqResPool = reqResPool,
        .reqResPoolMax = sizeof(reqResPool) / sizeof(*reqResPool),

        .confirmed = false,
        .breakOnError = false,

        .cbResult = cbResult
    };

    TEST_ASSERT_EQUAL(&client, PR6_ClientInit(&client, &in, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
    TEST_ASSERT_EQUAL(&client, PR6_ClientInit_AddMethod(&client, 42, 1, (const uint8_t *)"hello", strlen("hello")));    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));

    uint8_t expectedOut[] = {
        PR6_METHOD_NC_REQ,
            0x00, 42,
            1,
            5, 'h','e','l','l','o',            
            0x00, 42,
            1,
            5, 'h','e','l','l','o'            
    };
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), PR6_ClientOutput(&client, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));

    TEST_ASSERT_EQUAL(1, cbResultTouch);
    
    TEST_ASSERT_EQUAL(true, PR6_ClientIsComplete(&client));
}
