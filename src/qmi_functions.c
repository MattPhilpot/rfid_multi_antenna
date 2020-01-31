#include "qmi_functions.h"


#define PROGRAM_NAME    "qmicli"
#define PROGRAM_VERSION PACKAGE_VERSION


/* Globals */
static GMainLoop *loop;
static GCancellable *cancellable;
static QmiDevice *device;
static QmiClient *client;
//static QmiService service;
static gboolean operation_status;

static bool qmi_option_gps;
static bool qmi_option_autotracking;
static bool qmi_option_latlong;

/* Main options */
//static gchar *device_str;
//static gboolean get_service_version_info_flag;
//static gboolean get_wwan_iface_flag;
//static gboolean get_expected_data_format_flag;
//static gchar *set_expected_data_format_str;
//static gchar *device_set_instance_id_str;
//static gboolean device_open_version_info_flag;
//static gboolean device_open_sync_flag;
//static gchar *device_open_net_str;
//static gboolean device_open_proxy_flag;
//static gboolean device_open_mbim_flag;
//static gchar *client_cid_str;
//static gboolean client_no_release_cid_flag;
static gboolean verbose_flag;
static gboolean silent_flag;
//static gboolean version_flag;


static gboolean signals_handler (void)
{
    if (cancellable) {
        // Ignore consecutive requests of cancellation
        if (!g_cancellable_is_cancelled (cancellable))
        {
            g_printerr ("cancelling the operation...\n");
            g_cancellable_cancel (cancellable);
            // Re-set the signal handler to allow main loop cancellation on second signal
            return G_SOURCE_CONTINUE;
        }
    }

    if (loop && g_main_loop_is_running (loop))
    {
        g_printerr ("cancelling the main loop...\n");
        g_idle_add ((GSourceFunc) g_main_loop_quit, loop);
    }
    return G_SOURCE_REMOVE;
}

static void log_handler
(
    const gchar *log_domain,
    GLogLevelFlags log_level,
    const gchar *message,
    gpointer user_data
)
{
    const gchar *log_level_str;
    time_t now;
    gchar time_str[64];
    struct tm *local_time;
    gboolean err;

    /* Nothing to do if we're silent */
    if (silent_flag)
        return;

    now = time ((time_t *) NULL);
    local_time = localtime (&now);
    strftime (time_str, 64, "%d %b %Y, %H:%M:%S", local_time);
    err = FALSE;

    switch (log_level)
    {
    case G_LOG_LEVEL_WARNING:
        log_level_str = "-Warning **";
        err = TRUE;
        break;

    case G_LOG_LEVEL_CRITICAL:
    case G_LOG_FLAG_FATAL:
    case G_LOG_LEVEL_ERROR:
        log_level_str = "-Error **";
        err = TRUE;
        break;

    case G_LOG_LEVEL_DEBUG:
        log_level_str = "[Debug]";
        break;

    default:
        log_level_str = "";
        break;
    }

    if (!verbose_flag && !err)
        return;

    controller_log("[%s] %s %s\n",
               time_str,
               log_level_str,
               message);
}

/*****************************************************************************/
/* Running asynchronously */

static void release_client_ready (QmiDevice *dev, GAsyncResult *res)
{
    GError *error = NULL;

    if (!qmi_device_release_client_finish (dev, res, &error))
    {
        g_printerr ("error: couldn't release client: %s\n", error->message);
        g_error_free (error);
    }
    else
        g_debug ("Client released");

    g_main_loop_quit (loop);
}

void qmicli_async_operation_done (gboolean reported_operation_status)
{
    QmiDeviceReleaseClientFlags flags = QMI_DEVICE_RELEASE_CLIENT_FLAGS_NONE;

    // Keep the result of the operation
    operation_status = reported_operation_status;

    // Cleanup cancellation
    g_clear_object (&cancellable);

    // If no client was allocated (e.g. generic action), just quit
    if (!client)
    {
        g_main_loop_quit (loop);
        return;
    }

    // Always release the client
    flags |= QMI_DEVICE_RELEASE_CLIENT_FLAGS_RELEASE_CID;
    qmi_device_release_client (device, client, flags, 10, NULL, (GAsyncReadyCallback)release_client_ready, NULL);
}

static void allocate_client_ready
(
    QmiDevice *dev,
    GAsyncResult *res
)
{
    GError *error = NULL;

    client = qmi_device_allocate_client_finish (dev, res, &error);
    if (!client)
    {
        g_printerr("error: couldn't create client for the '%s' service: %s\n", qmi_service_get_string(QMI_SERVICE_PDS), error->message);
        g_error_free(error);
        qmicli_async_operation_done(FALSE);
        return;
    }

    qmicli_pds_run(dev, QMI_CLIENT_PDS(client), cancellable);
}

static void device_allocate_client (QmiDevice *dev)
{
    // As soon as we get the QmiDevice, create a client for the requested service
    qmi_device_allocate_client (dev, QMI_SERVICE_PDS, QMI_CID_NONE, 10, cancellable, (GAsyncReadyCallback)allocate_client_ready, NULL);
}

static void device_open_ready
(
    QmiDevice *dev,
    GAsyncResult *res
)
{
    GError *error = NULL;

    if (!qmi_device_open_finish (dev, res, &error))
    {
        g_printerr("error: couldn't open the QmiDevice: %s\n", error->message);
        g_error_free(error);
        qmicli_async_operation_done(FALSE);
        return;
    }

    g_debug ("QMI Device at '%s' ready", qmi_device_get_path_display(dev));
    device_allocate_client(dev);
}

static void device_new_ready
(
    GObject* unused,
    GAsyncResult* res
)
{
    QmiDeviceOpenFlags open_flags = QMI_DEVICE_OPEN_FLAGS_SYNC;
    GError* error = NULL;

    device = qmi_device_new_finish(res, &error);
    if (!device)
    {
        g_printerr("error: couldn't create QmiDevice: %s\n", error->message);
        g_error_free(error);
        operation_status = FALSE;
        g_main_loop_quit(loop);
        return;
    }

    // Setup device open flags
    //if (device_open_version_info_flag) open_flags |= QMI_DEVICE_OPEN_FLAGS_VERSION_INFO;
    //if (device_open_sync_flag) open_flags |= QMI_DEVICE_OPEN_FLAGS_SYNC;
    //if (device_open_proxy_flag) open_flags |= QMI_DEVICE_OPEN_FLAGS_PROXY;
    //if (device_open_mbim_flag) open_flags |= QMI_DEVICE_OPEN_FLAGS_MBIM;

    // Open the device
    qmi_device_open(device, open_flags, 15,cancellable, (GAsyncReadyCallback)device_open_ready, NULL);
}

bool qmicli_pds_option_gps_enabled()
{
    return qmi_option_gps;
}

bool qmicli_pds_option_auto_tracking_enabled()
{
    return qmi_option_autotracking;
}

bool qmicli_pds_option_get_lat_long_enabled()
{
    return qmi_option_latlong;
}

bool execute_qmi_gps_task
(
    char* device_path,
    bool gps,
    bool autotracking,
    bool latlong
)
{
    qmi_option_gps = gps;
    qmi_option_autotracking = autotracking;
    qmi_option_latlong = latlong;

    GError *error = NULL;
    GFile *file;

    setlocale (LC_ALL, "");

    g_type_init ();

    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, log_handler, NULL);
    g_log_set_handler ("Qmi", G_LOG_LEVEL_MASK, log_handler, NULL);

    if (verbose_flag)
        qmi_utils_set_traces_enabled (TRUE);

    // No device path given?
    if (!device_path)
    {
        g_printerr ("error: no device path specified\n");
        return false;
    }

    // Build new GFile from the commandline arg
    file = g_file_new_for_path(device_path);

    // Create requirements for async options
    cancellable = g_cancellable_new ();
    loop = g_main_loop_new (NULL, FALSE);

    // Setup signals
    g_unix_signal_add(SIGINT,  (GSourceFunc) signals_handler, NULL);
    g_unix_signal_add(SIGHUP,  (GSourceFunc) signals_handler, NULL);
    g_unix_signal_add(SIGTERM, (GSourceFunc) signals_handler, NULL);

    // Launch QmiDevice creation
    qmi_device_new(file, cancellable, (GAsyncReadyCallback)device_new_ready, NULL);
    g_main_loop_run(loop);

    if (cancellable)
        g_object_unref (cancellable);
    if (client)
        g_object_unref (client);
    if (device)
        g_object_unref (device);

    g_main_loop_unref (loop);
    g_object_unref (file);

    return (operation_status ? true : false);
}

/*
bool isGPSActive()
{
    return operation_status == TRUE;
}

void start_main_loop()
{
    if (!g_loop)
        g_loop = g_main_loop_new(NULL, FALSE);
    if (g_main_loop_is_running(g_loop))
        return;
    g_print("Starting main loop\n");
    g_main_loop_run(g_loop);
}

void quit_main_loop()
{
    g_print("Quitting main loop\n");
    g_main_loop_quit(g_loop);
    //g_main_loop_unref(g_loop);
}
*/
/*
bool init_QMI_GPS(char* device_path)
{
    //GError* error = NULL;
    GFile* gFile;
    GOptionContext* context;

    setlocale(LC_ALL, "");

    g_type_init();


    //if (!g_cancellable) g_cancellable = g_cancellable_new();


    if (g_device != NULL)
        device_allocate_client(g_device);
    else
    {


        g_log_set_handler(NULL, G_LOG_LEVEL_MASK, log_handler, NULL);
        g_log_set_handler("Qmi", G_LOG_LEVEL_MASK, log_handler, NULL);
        qmi_utils_set_traces_enabled(TRUE);

        gFile = g_file_new_for_path(device_path);

        g_unix_signal_add(SIGINT, (GSourceFunc) signals_handler, NULL);
        g_unix_signal_add(SIGHUP, (GSourceFunc) signals_handler, NULL);
        g_unix_signal_add(SIGTERM, (GSourceFunc) signals_handler, NULL);

        qmi_device_new(gFile, g_cancellable, (GAsyncReadyCallback)device_new_ready, NULL);
    }

    start_main_loop();
    g_object_unref(gFile);
    return qmicli_pds_get_is_tracking();
}
*/

