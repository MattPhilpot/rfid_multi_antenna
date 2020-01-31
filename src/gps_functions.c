#include <stdio.h>
#include "curl/curl.h"
#include <stdlib.h>
#include <string.h>
#include "gps_functions.h"
#include "util_functions.h"
#include "nmealib/info.h"
#include "nmealib/nmath.h"
#include "nmealib/parser.h"

double _latitude = 0;
double _longitude = 0;

struct string {
    char *ptr;
    size_t len;
};

static void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        controller_log("malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        controller_log("realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    // TODO Use NMEA parsing
    parseNMEAString(s->ptr);
    //printf("%s\n", s->ptr);

    if (_latitude != 0 && _longitude != 0)
        return 0;

    return size * nmemb;
    //return 0;
}

void parseNMEAString(char *buffer)
{
    NmeaInfo info;
    NmeaParser parser;
    NmeaPosition dpos;

    nmeaInfoClear(&info);
    nmeaParserInit(&parser, 0);

    FILE *stream;
    stream = fmemopen(buffer, strlen(buffer), "r");
    char line[84];
    while (fgets(line, 84, stream) != NULL) {
        //printf("%s", line);
        nmeaParserParse(&parser, line, strlen(line), &info);

        nmeaMathInfoToPosition(&info, &dpos);

        double lat, lon;

        lat = nmeaMathRadianToDegree(dpos.lat);
        lon = nmeaMathRadianToDegree(dpos.lon);

        if (lat != 0 && lon != 0) {
            _latitude = lat;
            _longitude = lon;
        }

        //printf("%03d, Lat: %f, Lon: %f, Sig: %d, Fix: %d\n", it, nmeaMathRadianToDegree(dpos.lat), nmeaMathRadianToDegree(dpos.lon), info.sig, info.fix);
    }

    nmeaParserDestroy(&parser);
}

