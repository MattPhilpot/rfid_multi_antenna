#ifndef UTIL_FUNCTIONS_H
#define UTIL_FUNCTIONS_H

#include <time.h>

extern void controller_log(const char* msg, ...);
extern void getCurrentLatitudeAndLongitude(double* latitude, double* longitude);
extern void getCurrentTime(time_t* time);
extern struct tm* getCurrentGMTTime(time_t* time);
extern void error(char *msg);

#endif // UTIL_FUNCTIONS_H
