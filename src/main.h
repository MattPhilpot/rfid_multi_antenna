#ifndef MAIN_H
#define MAIN_H

#include "tagstruct.h"

bool notbool(bool input);
void error(char *msg);
void resizeEPCList(size_t toSize);
void initEPCList(size_t initialSize);
void freeEPCList();
bool doesTagAlreadyExist(TagStruct *tag, uint16_t incrementOnAntenna);
void insertIntoTagList(TagStruct tag, uint16_t antenna);
void addTag3(TagStruct* toAdd, uint16_t* epc, int len, bool little_endian);
void printTagsInCollection();
void updateTimeStampAndGPSOnRecentTags();
void resetRecentScanCount();
void sendToServer();
void setActiveAntennas();
void setTagFlagBits(uint32_t flagBits);

#endif // MAIN_H



