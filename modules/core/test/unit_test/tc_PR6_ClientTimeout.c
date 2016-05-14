#include "unity.h"
#include "pr6_client.h"
#include <string.h>

int cbResultTouch;
uint16_t expectedListSize;
struct pr6_client_req_res *expectedList;

static void cbResult(struct pr6_client *r, uint16_t listSize, const struct pr6_client_req_res *list)
{
    cbResultTouch++;
    TEST_ASSERT_TRUE(listSize > 0);
    TEST_ASSERT_EQUAL(listSize, r->listSize);
    TEST_ASSERT_EQUAL(list, r->list);

    TEST_ASSERT_EQUAL(expectedListSize, listSize);

    for(uint16_t i=0; i < expectedListSize; i++){

        TEST_ASSERT_EQUAL(expectedList[i].objectID, list[i].objectID);
        TEST_ASSERT_EQUAL(expectedList[i].methodIndex, list[i].methodIndex);
        TEST_ASSERT_EQUAL(expectedList[i].argLen, list[i].argLen);
        
        TEST_ASSERT_EQUAL_MEMORY(expectedList[i].arg, list[i].arg, list[i].argLen);
        //TEST_ASSERT_EQUAL(expectedList[i].result, list[i].result);
        
        TEST_ASSERT_EQUAL(expectedList[i].returnValueLen, list[i].returnValueLen);
        //TEST_ASSERT_EQUAL_MEMORY(expectedList[i].returnValue, list[i].returnValue, list[i].returnValueLen);
    }
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

void test_PR6_ClientTimeout(void)
{
    uint8_t output[20U];

    struct pr6_client_req_res result[] = {
        {42, 1, (const uint8_t *)"hello", strlen("hello"), PR6_CLIENT_RESULT_TIMEOUT, NULL, 0U}
    };
    expectedListSize = sizeof(result) / sizeof(struct pr6_client_req_res);
    expectedList = result;

    TEST_ASSERT_EQUAL(true, PR6_ClientIsInitialised(&client));
    TEST_ASSERT_TRUE(PR6_ClientOutput(&client, output, sizeof(output)) > 0U);
    TEST_ASSERT_EQUAL(true, PR6_ClientIsSent(&client));

    PR6_ClientTimeout(&client);

    TEST_ASSERT_EQUAL(cbResultTouch, 1);
    TEST_ASSERT_EQUAL(true, PR6_ClientIsComplete(&client));
}
