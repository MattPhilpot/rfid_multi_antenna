#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mac_functions.h"
#include "configuration.h"
#include "memory_helper/memory_helper_macros.h"

static cfg_opt_t g_opts[] = {
                CFG_SIMPLE_STR("fsc_gps_url", &g_ControllerConfiguration.gps_url),
                CFG_SIMPLE_INT("fsc_gps_timeout", &g_ControllerConfiguration.gps_timeout),

                CFG_SIMPLE_STR("fsc_server_url", &g_ControllerConfiguration.server_url),
                CFG_SIMPLE_STR("fsc_server_user", &g_ControllerConfiguration.server_user),
                CFG_SIMPLE_STR("fsc_server_password", &g_ControllerConfiguration.server_password),

                CFG_SIMPLE_INT("fsc_heartbeat_interval", &g_ControllerConfiguration.heartbeat_interval),

                CFG_SIMPLE_INT("fsc_rfid_scan_time", &g_ControllerConfiguration.scan_time),
                CFG_SIMPLE_INT("fsc_rfid_scan_interval_time", &g_ControllerConfiguration.scan_interval_time),
                CFG_SIMPLE_INT("fsc_rfid_antenna_power", &g_ControllerConfiguration.antenna_power),
                CFG_SIMPLE_STR("fsc_rfid_active_antennas", &g_ControllerConfiguration.active_antennas),
//CFGT_INT
                CFG_SIMPLE_INT("fsc_history_upload_frequency", &g_ControllerConfiguration.history_upload_frequency),
                CFG_SIMPLE_INT("controller.history.max.session.count", &g_ControllerConfiguration.history_storage_max_count),
                CFG_SIMPLE_BOOL("needToUpdateConfig", &g_ControllerConfiguration.needToUpdateConfig),
                CFG_END()};

void getConfigValues()
{
    cfg_t *cfg;
    setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	cfg = cfg_init(g_opts, 0);
	cfg_parse(cfg, "default.conf");
	cfg_free(cfg);
	//return true;
}

static void *convertValue(cfg_type_t from, cfg_type_t to, void* value)
{
    if (from == to || (to != CFGT_INT && from != CFGT_STR))
        return value;

    int *retVal = malloc(sizeof(int));
    sscanf((char*) value, "%d", retVal);

    return retVal;
}

bool setConfigValue(cfg_type_t type,  const char* key, void* value)
{
    if (!key)
        return false;

    bool matchFound = false;
    bool endOfConf = false;
    int i = 0;

    do
    {
        matchFound = strcmp(g_opts[i++].name, key) == 0;
        endOfConf = g_opts[i].type == CFGT_NONE;
    }
    while(!endOfConf && !matchFound);

    if (!matchFound)
        return false;

    cfg_t *cfg;
    setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	cfg = cfg_init(g_opts, 0);
	//cfg_parse(cfg, "default.conf");

    void* newValue = convertValue(type, g_opts[i - 1].type, value);

	switch(g_opts[i - 1].type)
	{
    case CFGT_INT:
        cfg_setint(cfg, key, *(long int*)newValue);
        break;
    case CFGT_FLOAT:
        cfg_setfloat(cfg, key, *(double*)newValue);
        break;
    case CFGT_BOOL:
        if (value)
            cfg_setbool(cfg, key, cfg_true);
        else
            cfg_setbool(cfg, key, cfg_false);
        break;
    default:
    case CFGT_STR:
        cfg_setstr(cfg, key, value);
        break;
	}
	if (newValue)
        SAFE_FREE(newValue);

    {
        FILE *fp = fopen("default.conf", "w");
        cfg_print(cfg, fp);
        fclose(fp);
    }
	cfg_free(cfg);
	return true;
}

char* getHeartbeatURLAddress()
{
    char* endpoint_ext = "assetDevice/v1/heartbeat";
    char* retVal = malloc(strlen(g_ControllerConfiguration.server_url) + strlen(endpoint_ext) + 1);
    strcpy(retVal, g_ControllerConfiguration.server_url);
    strcat(retVal, endpoint_ext);
    return retVal;
}

char* getGetConfigURLAddressWithParamters()
{
    char* deviceIdentifier;
    getMAC(&deviceIdentifier);

    char* endpoint_ext = "assetDevice/v1/getConfig?deviceIdentifier=";
    char* retVal = malloc(strlen(g_ControllerConfiguration.server_url) + strlen(endpoint_ext) + strlen(deviceIdentifier) + 1);
    strcpy(retVal, g_ControllerConfiguration.server_url);
    strcat(retVal, endpoint_ext);
    strcat(retVal, deviceIdentifier);

    SAFE_FREE(deviceIdentifier);
    return retVal;
}

char* getGotConfigURLAddressWithParameters()
{
    char* deviceId;
    getMAC(&deviceId);
    char* endpoint_ext = "assetDevice/v1/gotConfig?deviceIdentifier=";
    char* retVal = malloc(strlen(g_ControllerConfiguration.server_url) + strlen(endpoint_ext) + strlen(deviceId) + 1);
    strcpy(retVal, g_ControllerConfiguration.server_url);
    strcat(retVal, endpoint_ext);
    strcat(retVal, deviceId);

    SAFE_FREE(deviceId);
    return retVal;
}
