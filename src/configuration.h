#ifndef CONTROLLER_CONFIGURATION_H
#define CONTROLLER_CONFIGURATION_H

#include <time.h>
#include <confuse.h>
#include "impinj_config/rs2000_config.h"

#define DEVICE_STATUS_RUNNING 4
#define DEVICE_STATUS_STARTING 1
#define DEVICE_STATUS_STOPPING 2

typedef struct ControllerConfiguration
{
    bool needToUpdateConfig;

    ///--- GPS CONFIG ---
    char* gps_url;
    long int gps_timeout;

    ///--- SERVER CONFIG ---
    char* server_url;
    char* server_user;
    char* server_password;

    ///--- HEARTBEAT CONFIG ---
    long int heartbeat_interval;

    ///--- RFID CONFIG ---
    long int scan_time;
    long int scan_interval_time;
    long int antenna_power;
    //bool active_antennas[MAX_ANTENNA_COUNT];
    char* active_antennas;

    ///--- HISTORY CONFIG ---
    long int history_storage_max_count; //-1 = unlimited, 0 = none, >0 = finite
    long int history_upload_frequency; // currently only 1 is supported (reasons)
} ControllerConfiguration;

///--------HEARTBEAT related things-----------
bool heartbeat_startup_success;
time_t heartbeat_startup_time;
time_t heartbeat_running_last_success_time;
///-------------------------------------------

///--------CONFIGURATION GET/GOT things-------
bool configuration_get_force_update;
time_t configuration_get_last_update;
///-------------------------------------------

extern ControllerConfiguration g_ControllerConfiguration;

///--------CONFIGURATION GET/SET things-------
void getConfigValues();
bool setConfigValue(cfg_type_t type,  const char* key, void* value);
char* getHeartbeatURLAddress();
char* getGetConfigURLAddressWithParamters();
char* getGotConfigURLAddressWithParameters();
///-------------------------------------------

#endif // CONTROLLER_CONFIGURATION_H
