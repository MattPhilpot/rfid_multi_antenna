#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>
#include "util_functions.h"
#include "tagstruct.h"
#include "gps_functions.h"
#include "tag_data.h"
#include "ipj_util_lt.h"
#include "impinj/iri_lt.h"
#include "impinj/platform.h"
#include "impinj_config/antenna_config.h"
#include "configuration.h"
#include "memory_helper/memory_helper_macros.h"
#include "qmi_util.h"


#include <fcntl.h>
#include <termios.h>

#define IPJ_UTIL_RETURN_ON_ERROR(e, msg) \
if (e)                                   \
{                                        \
    IPJ_UTIL_PRINT_ERROR(e, msg);        \
    return e;                            \
}

//static memory allocation for iri device
static ipj_iri_device g_iri_device = {0};
static FILE *fp;
static time_t g_nextHeartbeat;


void error( char *msg ) {
  perror(  msg );
  exit(1);
}

///---------------------UTIL FUNCTIONS---------------------------------------
void controller_log(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    //This function will determine whether to print to debug or write to log file
    if (fp != NULL)
        vfprintf(fp, msg, args);
    else
        vprintf(msg, args);
    va_end(args);
}

void getCurrentLatitudeAndLongitude(double* latitude, double* longitude)
{
    get_current_gps(longitude, latitude);
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
///--------------------------------------------------------------------------

void setActiveAntennas()
{
    int i;
    for (i = 0; i < strlen(g_ControllerConfiguration.active_antennas); ++i)
    {
        if (g_ControllerConfiguration.active_antennas[i] == 'Y')
            enableAntenna(i + 1);
        else
            disableAntenna(i + 1);
    }
    setAntennaTransmitPower(g_ControllerConfiguration.antenna_power);
    applyAntennaConfig(&g_iri_device);
}

static void getTemperatures(double *cpuTemperature, double *rfidTemperature)
{

}

static char* doGetConfigFromServer()
{
    char* json;
    char* result;
    char* url_with_params = getGetConfigURLAddressWithParamters();
    curlGet(url_with_params, "60admin", "test1", &result);
    controller_log("%s\n", result);

    SAFE_FREE(url_with_params);
    return result;
}

static void doGotConfigFromServer()
{
    char* json;
    char* result;
    char* url_with_params = getGotConfigURLAddressWithParameters();
    curlGet(url_with_params, "60admin", "test1", &result);
    controller_log("%s\n", result);

    SAFE_FREE(result);
    SAFE_FREE(url_with_params);
}

static void processGetConfigFromServer()
{
    char* config = doGetConfigFromServer();
    if (parseJsonResult(config))
        doGotConfigFromServer();
}

static char* doSendHeartBeat(short mode)
{
    //double latitude = 32.8887335;
    //double longitude = -96.9647598;
    double latitude = 0.0;
    double longitude = 0.0;
    char *time;
    char *json;
    char *deviceIdentifier;
    getUTC(&time);
    getCurrentLatitudeAndLongitude(&latitude, &longitude);
    getMAC(&deviceIdentifier);

    json = getHeartBeatJson(deviceIdentifier, time, latitude, longitude, mode);
    controller_log("Heartbeat json: %s\n", json);

    char *result;
    char* url = getHeartbeatURLAddress();
    controller_log("Posting Heartbeat to: %s\n", url);
    curlPostJson(url, "60admin", "test1", json, &result);
    controller_log("%s\n", result);

    SAFE_FREE(url);
    SAFE_FREE(time);
    SAFE_FREE(deviceIdentifier);
    SAFE_FREE(json);
    return result;
}

///--------------HEARTBEAT related code--------------------------------------
static void processHeartBeat(time_t* currentTime)
{
    if (g_nextHeartbeat - *currentTime > 0)
        return;

    g_nextHeartbeat = *currentTime + g_ControllerConfiguration.heartbeat_interval;
    char* response = doSendHeartBeat(DEVICE_STATUS_RUNNING);
    //process reponse
    if (parseJsonResult(response) && g_ControllerConfiguration.needToUpdateConfig)
        processGetConfigFromServer();
}

static void processStartupHeartBeat()
{
    char* response = doSendHeartBeat(DEVICE_STATUS_STARTING);
    processGetConfigFromServer();
    SAFE_FREE(response);
}

static void processShutdownHeartBeat()
{
    char* response = doSendHeartBeat(DEVICE_STATUS_STOPPING);
}
///--------------------------------------------------------------------------

static void doGPSGetAndTagUpdate(bool gps_enabled)
{
    double longitude = 0.0;
    double latitude = 0.0;
    if (gps_enabled)
    {
        controller_log("Begin get gps fix\n");
        getCurrentLatitudeAndLongitude(&latitude, &longitude);
        controller_log("Finish get gps fix\n");
    }
    updateTimeStampAndGPSOnRecentTags(&latitude, &longitude);
}

int programMain(int argc, char *argv[])
{
    bool gps_enabled = false;
    ipj_error error = E_IPJ_ERROR_SUCCESS;
    time_t currentTime;

    //initialize memory space for tag list
    initializeHistoryList(DEFAULT_HISTORY_LIST_SIZE);

    //initialize qmi gps
    initialize_QMI_PDS("/dev/cdc-wdm0", 13);

    //get initial config settings
    getConfigValues();

    //Here we should start the init process. Send a service start heartbeat. (and try to detect the connected antennas?)
    processStartupHeartBeat();

    while(1)
    {
        ipj_error error = E_IPJ_ERROR_SUCCESS; //reset error to success
        getCurrentTime(&currentTime); //get time of loop start on each iteration
        processHeartBeat(&currentTime); //if required, do heartbeat
        getConfigValues(); //load latest conf values

        error = ipj_util_setup(&g_iri_device, "/dev/ttyAMA0");
        IPJ_UTIL_RETURN_ON_ERROR(error, "ipj_util_setup"); //fixme - this shouldn't exit the main loopes

        // get temperature of components
        uint32_t rs2000_temp = 0;
        uint32_t pa_temp = 0;
        getRS2000Temperatures(&g_iri_device, &rs2000_temp, &pa_temp);

        beginNewHistorySession(); //start new history session

        ///--------- Inventory Section ---------
        //Always set the settings, because <reasons>
        setTagReportFlags(E_IPJ_TAG_FLAG_BIT_EPC | E_IPJ_TAG_FLAG_BIT_ANTENNA | E_IPJ_TAG_FLAG_BIT_RSSI, &g_iri_device); //What values do we want returned?
        setActiveAntennas(); //this is where we set active/unactive antennas

        error = ipj_util_perform_inventory(&g_iri_device, g_ControllerConfiguration.scan_time); //perform dat inventory for x milliseconds
        controller_log("\n");
        if (error > 0) IPJ_UTIL_PRINT_ERROR(error, "ipj_util_perform_inventory");
        error = ipj_util_cleanup(&g_iri_device, &error); //cleanup device and set boolean for re setting up the device
        if (error > 0) IPJ_UTIL_PRINT_ERROR(error, "ipj_util_cleanup");
        ///-------------------------------------

        doGPSGetAndTagUpdate(gps_enabled); //get gps lat/long and apply to most recent session
        performComparisonFromPreviousSession();

        /*
        sendToServer();
        freeHistoryList();
        initializeHistoryList(DEFAULT_HISTORY_LIST_SIZE);
        */

        if (g_ControllerConfiguration.scan_interval_time > 0);
        {
            //this should sleep for 10 seconds at a time and go through the loop, checking to see if it needs to heartbeat
            controller_log("Sleep for [%d] seconds\n", g_ControllerConfiguration.scan_interval_time);
            sleep(g_ControllerConfiguration.scan_interval_time);
        }
    }

cleanup:
    //save tags to storage
    //write error logs

    return 0;
}

int main(int argc, char* argv[])
{
    fp = NULL;
    pid_t process_id = 0;
    pid_t sid = 0;

    process_id = fork(); //Create child process

    if (process_id < 0) //Indication of fork failure
    {
        controller_log("fork for child process failed [%d]\n", process_id);
        exit(1); //return failure in exit code
    }

    if (process_id > 0) //PARENT PROCESS. Need to kill it
    {
        controller_log("process_id of child process: %d\n", process_id);
        exit(0); //return success in exit status
    }

    umask(0); //unmask the file mode
    sid = setsid(); //set new session

    if (sid < 0)
    {
        exit(1); //return failure
    }

    chdir("/"); //change the current working directory to root

    close(STDIN_FILENO); //close stdin
    close(STDOUT_FILENO); //close stdout
    close(STDERR_FILENO); //close stderr

    fp = fopen("controller_log.txt", "w+"); //open a log file in write mode

    /*
    while (1)
    {
        sleep(1); //don't block context switches, let the process sleep for some time
    }
    */
    programMain(argc, argv);

    fclose(fp);

    return 0;
}
