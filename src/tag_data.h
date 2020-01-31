#ifndef TAG_DATA_H
#define TAG_DATA_H

#include <stdbool.h>
#include "tagstruct.h"


#define RED   "\x1B[31m" // <10
#define YEL   "\x1B[33m" // 11-20
#define GRN   "\x1B[32m" // 21-30
#define BLU   "\x1B[34m" // 31-40
#define MAG   "\x1B[35m" // 41-50
#define CYN   "\x1B[36m" // 51-60
#define WHT   "\x1B[37m" // >60
#define RESET "\x1B[0m"

#define DEFAULT_HISTORY_LIST_SIZE 24
#define DEFAULT_TAG_LIST_SIZE 20

extern TagHistory** g_TagHistoryList;
extern size_t g_CurrentHistoryCount;
extern size_t g_MaxHistoryCount;

extern double g_LastLongitude;
extern double g_LastLatitude;

void initializeHistoryList(size_t initialSize);
void freeHistoryList();
bool compareTwoTagsSimilar(char* tag1, char* tag2, uint8_t tag1Length, uint8_t tag2Length);
bool doesTagAlreadyExist(TagHistory* tagHistory, TagStruct *tag, uint16_t incrementOnAntenna);
void insertIntoTagList(TagHistory* tagHistory, TagStruct tag, uint16_t antenna);
void insertTagIntoLatestHistory(TagStruct tag, uint16_t antenna);
void addTag3(TagStruct* toAdd, uint16_t* epc, int len, bool little_endian);
void printTagsInCollection();
void resizeHistoryList(int toSize);
void resizeTagList(TagHistory* tagHistory, int toSize);
void sendToServer();
void updateTimeStampAndGPSOnRecentTags(double* latitude, double* longitude);
void performComparisonFromPreviousSession();
//void resetRecentScanCount();

#endif // TAG_DATA_H
