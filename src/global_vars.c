#include "tag_data.h"
#include "configuration.h";

TagHistory** g_TagHistoryList = NULL;
size_t g_CurrentHistoryCount = 0;
size_t g_MaxHistoryCount = 0;
double g_LastLongitude = 0.0;
double g_LastLatitude = 0.0;

ControllerConfiguration g_ControllerConfiguration;


///--------HEARTBEAT related things-----------
bool heartbeat_startup_success = false;
time_t heartbeat_startup_time;
time_t heartbeat_running_last_success_time;
///-------------------------------------------

///--------CONFIGURATION GET/GOT things-------
bool configuration_get_force_update = false;
time_t configuration_get_last_update;
///-------------------------------------------
