#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "tag_data.h"
#include "configuration.h"
#include "memory_helper/memory_helper_macros.h"
#include "impinj_config/rs2000_config.h"

static char g_Literals[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void initializeHistoryList(size_t initialSize)
{
    resizeHistoryList(initialSize);
    g_CurrentHistoryCount = 0;
}

void freeHistoryList()
{
    resizeHistoryList(0);
}

bool compareTwoTagsSimilar(char* tag1, char* tag2, uint8_t tag1Length, uint8_t tag2Length)
{
    if (!tag1 || !tag2)
        return false;

    if (tag1Length != tag2Length)
        return false;
    int i;

    for (i = 0; i < tag1Length; ++i)
    {
        if (tag1[i] != tag2[i])
            return false;
    }
    return true;
}

bool doesTagAlreadyExist(TagHistory* tagHistory, TagStruct *tag, uint16_t incrementOnAntenna)
{
    int loopCounter = 0;
    for (loopCounter = 0; loopCounter < tagHistory->itemsInList; ++loopCounter)
    {
        if (compareTwoTagsSimilar(tagHistory->tagList[loopCounter]->epc, tag->epc,
                                  tagHistory->tagList[loopCounter]->tagSize,
                                  tag->tagSize))
        {
            if (incrementOnAntenna > 0)
            {
                ++tagHistory->tagList[loopCounter]->foundCount[incrementOnAntenna - 1];
                //++g_ListOfTags[loopCounter]->totalFoundCount;
            }
            return true;
        }

    }
    return false;
}

void insertTagIntoLatestHistory(TagStruct tag, uint16_t antenna)
{
    if (g_CurrentHistoryCount > 0)
        insertIntoTagList(g_TagHistoryList[g_CurrentHistoryCount - 1], tag, antenna);
}

void insertIntoTagList(TagHistory* tagHistory, TagStruct tag, uint16_t antenna)
{
    if (g_CurrentHistoryCount == 0)
        return;
    if (tagHistory->itemsInList >= tagHistory->listSize)
        resizeTagList(tagHistory, tagHistory->itemsInList * 2);


    if (!doesTagAlreadyExist(tagHistory, &tag, antenna))
    {
        int i;
        for (i = 0; i < MAX_ANTENNA_COUNT; ++i) //have to reset found counts to 0;
        {
            tag.foundCount[i] = 0;
        }
        tag.foundCount[antenna - 1] = 1;
        //tag.recentScanCount[antenna - 1] = 1;
        //tag.totalFoundCount = 1;
        //tag.totalRecentScanCount = 1;
        *tagHistory->tagList[tagHistory->itemsInList++] = tag;
    }
    printTagsInCollection();
}

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

void printTagsInCollection()
{
    TagHistory* previousTagHistory;
    TagHistory* tagHistoryToUse = g_TagHistoryList[g_CurrentHistoryCount - 1];

    uint16_t tagFoundCount = 0;
    uint16_t minFoundCount = 0;
    uint16_t maxFoundCount = 0;
    uint16_t avgFoundCount = 0;
    size_t i;
    for (i = 0; i < tagHistoryToUse->itemsInList; ++i)
    {
        uint16_t in_tagFoundCount = tagHistoryToUse->tagList[i]->foundCount[0] +
                                tagHistoryToUse->tagList[i]->foundCount[1] +
                                tagHistoryToUse->tagList[i]->foundCount[2] +
                                tagHistoryToUse->tagList[i]->foundCount[3];

        if (i == 0)
            minFoundCount = in_tagFoundCount;

        tagFoundCount += in_tagFoundCount;
        if (in_tagFoundCount > maxFoundCount)
            maxFoundCount = in_tagFoundCount;
        else if (in_tagFoundCount < minFoundCount)
            minFoundCount = in_tagFoundCount;
    }
    avgFoundCount = tagFoundCount / tagHistoryToUse->itemsInList;
    /*
    system("tput home");
    printf(WHT "Number of history collections: %d\n", g_CurrentHistoryCount);
    if (g_CurrentHistoryCount > 1)
    {
        previousTagHistory = g_TagHistoryList[g_CurrentHistoryCount - 2];
        printf(WHT "Number tags in last collection: %d\n", previousTagHistory->itemsInList);
    }
    printf(WHT "Number tags in current collection: %d\n", tagHistoryToUse->itemsInList);

    size_t i;
    for (i = 0; i < tagHistoryToUse->itemsInList; ++i)
    {
        uint16_t tagFoundCount = tagHistoryToUse->tagList[i]->foundCount[0] +
                                tagHistoryToUse->tagList[i]->foundCount[1] +
                                tagHistoryToUse->tagList[i]->foundCount[2] +
                                tagHistoryToUse->tagList[i]->foundCount[3];

        printf(WHT "%6d ", tagFoundCount);

        if (tagFoundCount < 20)
                printf(RED "%26s", tagHistoryToUse->tagList[i]->epc);
            else if (tagFoundCount < 40)
                printf(YEL "%26s", tagHistoryToUse->tagList[i]->epc);
            else if (tagFoundCount < 60)
                printf(GRN "%26s", tagHistoryToUse->tagList[i]->epc);
            else if (tagFoundCount < 80)
                printf(CYN "%26s", tagHistoryToUse->tagList[i]->epc);
            else if (tagFoundCount < 100)
                printf(MAG "%26s", tagHistoryToUse->tagList[i]->epc);
            else
                printf(WHT "%26s", tagHistoryToUse->tagList[i]->epc);
        if (i % 2 != 0)
            printf("\n");

        //printf("%d, %d, %s\n", g_ListOfTags[i]->recentScanCount, g_ListOfTags[i]->foundCount, g_ListOfTags[i]->epc);
    }
    */
    controller_log(WHT "\rNumber tags in current collection: %d (%d)(%d/%d/%d)",
            tagHistoryToUse->itemsInList, tagFoundCount, minFoundCount, avgFoundCount, maxFoundCount);
}

void resizeTagList(TagHistory* tagHistory, int toSize)
{
    int counter;
    if (toSize < 0)
        toSize = 0;
    if (toSize < tagHistory->listSize)
    {
        for (counter = tagHistory->listSize; counter < toSize; ++counter)
        {
            SAFE_FREE(tagHistory->tagList[counter]);
        }

        if (toSize == 0)
        {
            SAFE_FREE(tagHistory->tagList);
            tagHistory->tagList = NULL;
        }
        else
            tagHistory->tagList = realloc(tagHistory->tagList, sizeof(TagStruct*) * toSize);
    }
    else if (toSize > tagHistory->listSize)
    {
        if (tagHistory->listSize == 0)
            tagHistory->tagList = malloc(sizeof(TagStruct*) * toSize);
        else
            tagHistory->tagList = realloc(tagHistory->tagList, sizeof(TagStruct*) * toSize);

        for (counter = tagHistory->listSize; counter < toSize; ++counter)
        {
            tagHistory->tagList[counter] = malloc(sizeof(TagStruct));
        }
    }
    tagHistory->listSize = toSize;
}

void resizeHistoryList(int toSize)
{
    int counter;
    if (toSize < 0)
        toSize = 0;
    if (toSize < g_MaxHistoryCount)
    {
        for (counter = g_MaxHistoryCount; counter < toSize; ++counter)
        {
            resizeTagList(g_TagHistoryList[counter], 0);
            SAFE_FREE(g_TagHistoryList[counter]);
        }

        if (toSize == 0)
        {
            SAFE_FREE(g_TagHistoryList);
            g_TagHistoryList = NULL;
        }
        else
            g_TagHistoryList = realloc(g_TagHistoryList, sizeof(TagHistory*) * toSize);
    }
    else if (toSize > g_MaxHistoryCount)
    {
        if (g_MaxHistoryCount == 0)
            g_TagHistoryList = malloc(sizeof(TagHistory*) * toSize);
        else
            g_TagHistoryList = realloc(g_TagHistoryList, sizeof(TagHistory*) * toSize);

        for (counter = g_MaxHistoryCount; counter < toSize; ++counter)
        {
            g_TagHistoryList[counter] = malloc(sizeof(TagHistory));
            g_TagHistoryList[counter]->scanDateTime = 0;
            g_TagHistoryList[counter]->longitude = 0.0;
            g_TagHistoryList[counter]->latitude = 0.0;
            g_TagHistoryList[counter]->listSize = 0;
            g_TagHistoryList[counter]->itemsInList = 0;
            g_TagHistoryList[counter]->itemChangeFromLastHistory = false;
            resizeTagList(g_TagHistoryList[counter], DEFAULT_TAG_LIST_SIZE);
        }
    }
    g_MaxHistoryCount = toSize;
}

void sendToServer()
{
    double latitude = 0;
    double longitude = 0;
	getCurrentLatitudeAndLongitude(&latitude, &longitude);

    char *json;
    char *time;
    getUTC(&time);

    char *deviceIdentifier;
    getMAC(&deviceIdentifier);

    json = assignTagsToLocationsJson(deviceIdentifier, latitude, longitude, time, *g_TagHistoryList[g_CurrentHistoryCount - 1]);
    printf("%s\n", json);

    char *result;

    curlPostJson(g_ControllerConfiguration.server_url,
                g_ControllerConfiguration.server_user,
                g_ControllerConfiguration.server_password, json, &result);
   
    printf("%s\n", result);

    free(json);
    free(result);
    sleep(5);
}

void updateTimeStampAndGPSOnRecentTags(double* latitude, double* longitude)
{
    time_t now;
    getCurrentTime(&now);

    g_TagHistoryList[g_CurrentHistoryCount - 1]->latitude = *latitude;
    g_TagHistoryList[g_CurrentHistoryCount - 1]->longitude = *latitude;
    g_TagHistoryList[g_CurrentHistoryCount - 1]->scanDateTime = now;

    if (latitude)
        g_LastLatitude = *latitude;
    if (longitude)
        g_LastLongitude = *longitude;

}

//This is hyper temporary
static void shiftHistoryLeft()
{
    int shiftSize = g_CurrentHistoryCount - g_ControllerConfiguration.history_storage_max_count;
    if (shiftSize < 1)
        shiftSize = 1;

    int i;
    for (i = 0; i < shiftSize; ++i)
    {
        resizeTagList(g_TagHistoryList[i], 0);
        SAFE_FREE(g_TagHistoryList[i]);
    }

    for (i = shiftSize; i < g_CurrentHistoryCount; ++i)
    {
        g_TagHistoryList[i - shiftSize] = g_TagHistoryList[i];
    }

    if (g_CurrentHistoryCount > 0 && g_ControllerConfiguration.history_storage_max_count > 0)
    {
        for (i = g_ControllerConfiguration.history_storage_max_count - 1; i < g_CurrentHistoryCount; ++i)
        {
            g_TagHistoryList[i] = malloc(sizeof(TagHistory));
            g_TagHistoryList[i]->scanDateTime = 0;
            g_TagHistoryList[i]->longitude = 0.0;
            g_TagHistoryList[i]->latitude = 0.0;
            g_TagHistoryList[i]->listSize = 0;
            g_TagHistoryList[i]->itemsInList = 0;
            g_TagHistoryList[i]->itemChangeFromLastHistory = false;
            resizeTagList(g_TagHistoryList[i], DEFAULT_TAG_LIST_SIZE);
        }
        g_CurrentHistoryCount = g_ControllerConfiguration.history_storage_max_count;
    }
}

void performComparisonFromPreviousSession()
{

}

void beginNewHistorySession()
{
    if (g_ControllerConfiguration.history_storage_max_count > 0 &&
        g_CurrentHistoryCount >= g_ControllerConfiguration.history_storage_max_count)
    {
        shiftHistoryLeft();
        printf("History Limit reached [%d/%d]\n", g_CurrentHistoryCount, g_ControllerConfiguration.history_storage_max_count);
    }
    else if (g_CurrentHistoryCount > 0  && g_TagHistoryList[g_CurrentHistoryCount - 1]->itemsInList == 0)
    {
        printf("No Tags recorded in last session. Reusing [%d/%d]\n", g_CurrentHistoryCount, g_ControllerConfiguration.history_storage_max_count);
    }
    else
    {
        if (g_CurrentHistoryCount >= g_MaxHistoryCount)
            resizeHistoryList(g_CurrentHistoryCount + 10);
        printf("Current History Count [%d/%d]\n", ++g_CurrentHistoryCount, g_ControllerConfiguration.history_storage_max_count);
    }
}

