#include <stdio.h>
#include <time.h>

void getUTC(char **utc)
{
    struct tm *gmt;
    gmt = getCurrentGMTTime(NULL);

    char _utc[21];
    sprintf(_utc, "%04d-%02d-%02dT%02d:%02d:%02dZ", gmt->tm_year + 1900, (gmt->tm_mon) + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
    *utc = strdup(_utc);
}

void getFromTimeT(char **utc, time_t time)
{
    struct tm *gmt;
    gmt = getCurrentGMTTime(&time);

    char _utc[21];
    sprintf(_utc, "%04d-%02d-%,acp02dT%02d:%02d:%02dZ", gmt->tm_year + 1900, (gmt->tm_mon) + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
    *utc = strdup(_utc);
}

