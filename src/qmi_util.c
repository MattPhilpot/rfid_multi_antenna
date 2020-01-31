#include <nmealib/info.h>
#include <nmealib/nmath.h>
#include <nmealib/parser.h>
#include "qmi_functions.h"
#include "qmi_util.h"

static double _latitude;
static double _longitude;

static char* device_path;
static uint16_t device_path_length;

bool qmi_parse_nmea(char *buffer)
{
    if (!buffer)
        return false;

    if (strlen(buffer) < 6 || strncmp("$GNGNS", buffer, 6) != 0)
        return false;


    g_print("[NMEA] %s", buffer);
    //char* buffer_copy = malloc(strlen(buffer) * sizeof(char));
    //strncpy(buffer_copy, buffer, strlen(buffer));
    int it;
    NmeaInfo info;
    NmeaParser parser;
    NmeaPosition dpos;
    bool found_latlong = false;

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
            found_latlong = true;
        }

        //printf("%03d, Lat: %f, Lon: %f, Sig: %d, Fix: %d\n", it, nmeaMathRadianToDegree(dpos.lat), nmeaMathRadianToDegree(dpos.lon), info.sig, info.fix);
    }

    //free(buffer_copy);
    memset(buffer, '\0', strlen(buffer) + 1);
    nmeaParserDestroy(&parser);

    return found_latlong;
}

bool initialize_QMI_PDS(char* path, uint16_t len)
{
    controller_log("Initialization begin of qmi gps [%s]\n", path);
    device_path = path;
    device_path_length = len;

    if (!device_path)
    {
        //means we need to search for compatible device
    }

    uint8_t init_attempt = 0;
    bool init_successful;

    do
    {
        controller_log("Initialization attempt #%d\n", init_attempt + 1);
        init_successful = execute_qmi_gps_task(device_path, true, true, false); //"/dev/cdc-wdm0"
    }
    while (init_attempt++ < 3 && !init_successful);

    controller_log("Initialization finish of qmi gps [%d]\n", init_successful);
    return init_successful;
}

bool get_current_gps(double* longitude, double* latitude)
{
    controller_log("Getting current gps location\n");
    _latitude = 0.0;
    _longitude = 0.0;

    bool success = execute_qmi_gps_task(device_path, true, true, true); //"/dev/cdc-wdm0"

    if (success)
    {
        *longitude = _longitude;
        *latitude = _latitude;
        //controller_log("Found lat/long [%lf][%lf]\n", _latitude, _longitude);
        controller_log("Found lat/long\n");
    }

    return success;
}

/*
bool isGPSActive()
{
    return operation_status == TRUE;
}

void finished_init_QMI_GPS()
{
    g_print("Quitting main loop\n");
    g_main_loop_quit(loop);
    //g_main_loop_unref()
}

void init_QMI_GPS()
{
    GError* error = NULL;
    GFile* gFile;
    GOptionContext* context;

    setlocale(LC_ALL, "");

    g_type_init();

    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, log_handler, NULL);
    g_log_set_handler("Qmi", G_LOG_LEVEL_MASK, log_handler, NULL);
    qmi_utils_set_traces_enabled(TRUE);

    gFile = g_file_new_for_path("/dev/cdc-wdm0");

    cancellable = g_cancellable_new();
    loop = g_main_loop_new(NULL, FALSE);

    g_unix_signal_add(SIGINT, (GSourceFunc) signals_handler, NULL);
    g_unix_signal_add(SIGHUP, (GSourceFunc) signals_handler, NULL);
    g_unix_signal_add(SIGTERM, (GSourceFunc) signals_handler, NULL);

    qmi_device_new(gFile, cancellable, (GAsyncReadyCallback)device_new_ready, NULL);
    g_main_loop_run(loop);
}
*/

