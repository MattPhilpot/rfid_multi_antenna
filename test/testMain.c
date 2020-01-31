#include "unity/unity.h"
#include "../src/util_functions.h"

void getCurrentLatitudeAndLongitude(double* latitude, double* longitude)
{
    getLocation("telnet://192.168.1.1", 11010, latitude, longitude);
}

void getCurrentTime(time_t* now)
{
    time(now);
    //this is to be doubly sure that the time_t variable was filled
    sleep(1);
}

struct tm* getCurrentGMTTime(time_t* time)
{
    if (!time)
    {
        time_t now;
        getCurrentGMTTime(&now);
        return gmtime(&now);
    }
    return gmtime(time);
}



int main(int argc, const char * argv[])
{
    test_tag_functions_main();
    test_tag_data_main();
}
