#include "unity/unity.h"
#include "../src/tag_data.h"

void setupPreTest()
{
    resizeHistoryList(0);
}

void cleanupPostTest()
{
    resizeHistoryList(0);
}

/*
void addTag3(TagStruct* toAdd, uint16_t* epc, int len, bool little_endian)
{
    toAdd->tagSize = len * 4;

    memset(toAdd->epc, '\0', MAX_EPC_LENGTH);
    int bufferPos = 0;
    int i;
    for (i = 0; i < len; i++)
    {
        if (little_endian)
        {
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 4) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i]) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 12) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 8) & 0xF];
        }
        else
        {
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 12) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 8) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i] >> 4) & 0xF];
            toAdd->epc[bufferPos++] = g_Literals[(epc[i]) & 0xF];
        }
    }
}
*/

void test_addTag3_littleEndian()
{
    TagStruct tag;

    uint16_t epc[] = { //7B00D711CE22
        123,
        4567,
        8910
    };
    addTag3(&tag, &epc, 3, true);
    TEST_ASSERT_EQUAL(12, tag.tagSize);
    TEST_ASSERT_EQUAL_STRING("7B00D711CE22", tag.epc);
}

void test_addTag3_BigEndian()
{
    TagStruct tag;

    uint16_t epc[] = { //007B11D722CE
        123,
        4567,
        8910
    };
    addTag3(&tag, &epc, 3, false);
    TEST_ASSERT_EQUAL(12, tag.tagSize);
    TEST_ASSERT_EQUAL_STRING("007B11D722CE", tag.epc);
}

void test_resizeEPCList0to3()
{
    setupPreTest();
    resizeHistoryList(3);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList);
    TEST_ASSERT_EQUAL(3, g_MaxHistoryCount);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList[0]);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList[1]);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList[2]);
    cleanupPostTest();
}

void test_resizeEPCList3to0()
{
    setupPreTest();
    resizeHistoryList(3);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList);
    TEST_ASSERT_EQUAL(3, g_MaxHistoryCount);
    //actual test
    resizeHistoryList(0);
    TEST_ASSERT_NULL(g_TagHistoryList);
    TEST_ASSERT_EQUAL(0, g_MaxHistoryCount);
    cleanupPostTest();
}

void test_resizeEPCListNegative()
{
    setupPreTest();
    resizeHistoryList(3);
    TEST_ASSERT_NOT_NULL(g_TagHistoryList);
    TEST_ASSERT_EQUAL(3, g_MaxHistoryCount);
    //actual test
    resizeHistoryList(-1);
    TEST_ASSERT_NULL(g_TagHistoryList);
    TEST_ASSERT_EQUAL(0, g_MaxHistoryCount);
    cleanupPostTest();
}


/*
void test_resetRecentScanCount()
{
    setupPreTest();
    resizeHistoryList(2);
    //actual test;
    TagStruct newTag1;
    TagStruct newTag2;
    newTag1.totalRecentScanCount = 5;
    newTag1.recentScanCount[0] = 1;
    newTag1.recentScanCount[1] = 2;
    newTag1.recentScanCount[2] = 1;
    newTag1.recentScanCount[3] = 1;

    newTag2.totalRecentScanCount = 6;
    newTag2.recentScanCount[0] = 2;
    newTag2.recentScanCount[1] = 2;
    newTag2.recentScanCount[2] = 1;
    newTag2.recentScanCount[3] = 1;
    *g_ListOfTags[0] = newTag1;
    *g_ListOfTags[1] = newTag2;
    g_NumberOfTagsInList = 2;

    resetRecentScanCount();
    TEST_ASSERT_EQUAL(0, g_ListOfTags[0]->totalRecentScanCount);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[0]->recentScanCount[0]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[0]->recentScanCount[1]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[0]->recentScanCount[2]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[0]->recentScanCount[3]);

    TEST_ASSERT_EQUAL(0, g_ListOfTags[1]->totalRecentScanCount);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[1]->recentScanCount[0]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[1]->recentScanCount[1]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[1]->recentScanCount[2]);
    TEST_ASSERT_EQUAL(0, g_ListOfTags[1]->recentScanCount[3]);

    cleanupPostTest();
}
*/

int test_tag_data_main()
{
    UnityBegin("test/test_tag_data.c");
    RUN_TEST(test_resizeEPCList0to3);
    RUN_TEST(test_resizeEPCList3to0);
    RUN_TEST(test_resizeEPCListNegative);

    //RUN_TEST(test_resetRecentScanCount);

    RUN_TEST(test_addTag3_littleEndian);
    RUN_TEST(test_addTag3_BigEndian);
    return UnityEnd();
}
