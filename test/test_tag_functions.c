#include "unity/unity.h"


void test_compareTwoTagsSimilar_true()
{
    char tag1[12] = "123456789012";
    char tag2[12] = "123456789012";
    TEST_ASSERT_TRUE(compareTwoTagsSimilar(&tag1, &tag2, 12, 12));
}

void test_compareTwoTagsSimilar_SingleTagNull()
{
    char tag1[12] = "123456789012";
    TEST_ASSERT_FALSE(compareTwoTagsSimilar(&tag1, NULL, 12, 12));
}

void test_compareTwoTagsSimilar_BothTagsNull()
{
    TEST_ASSERT_FALSE(compareTwoTagsSimilar(NULL, NULL, 12, 12));
}

void test_compareTwoTagsSimilar_TagsDifferentLength()
{
    char tag1[12] = "123456789012";
    char tag2[13] = "1234567890123";
    TEST_ASSERT_FALSE(compareTwoTagsSimilar(&tag1, &tag2, 12, 13));
}

void test_compareTwoTagsSimilar_TagsSameLengthButDifferent()
{
    char tag1[12] = "123456789012";
    char tag2[12] = "123456789013";
    TEST_ASSERT_FALSE(compareTwoTagsSimilar(&tag1, &tag2, 12, 12));
}

void test_compareTwoTagsSimilar_OutOfBounds()
{
    //Hmmm, is this correct?
    char tag1[12] = "123456789012";
    char tag2[12] = "123456789012";
    TEST_ASSERT_FALSE(compareTwoTagsSimilar(&tag1, &tag2, 16, 16));
}

int test_tag_functions_main()
{
    UnityBegin("test/test_tag_functions.c");
    //need to write tests for if search goes out of bounds
    RUN_TEST(test_compareTwoTagsSimilar_true);
    RUN_TEST(test_compareTwoTagsSimilar_SingleTagNull);
    RUN_TEST(test_compareTwoTagsSimilar_BothTagsNull);
    RUN_TEST(test_compareTwoTagsSimilar_TagsDifferentLength);
    RUN_TEST(test_compareTwoTagsSimilar_TagsSameLengthButDifferent);
    RUN_TEST(test_compareTwoTagsSimilar_OutOfBounds);

    return UnityEnd();
}
