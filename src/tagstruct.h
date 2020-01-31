#ifndef TAGSTRUCT_H_INCLUDED
#define TAGSTRUCT_H_INCLUDED

#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include "impinj_config/rs2000_config.h"

#define MAX_EPC_LENGTH 48

typedef struct TagStruct
{
    uint8_t tagSize;
    uint8_t rssi[MAX_ANTENNA_COUNT];
    uint16_t foundCount[MAX_ANTENNA_COUNT];
    char epc[MAX_EPC_LENGTH];
} TagStruct;

typedef struct TagHistory
{
    time_t scanDateTime;
    double longitude;
    double latitude;
    uint16_t listSize;
    uint16_t itemsInList;
    bool itemChangeFromLastHistory;
    TagStruct** tagList;
} TagHistory;

#endif // TAGSTRUCT_H_INCLUDED
