#include "qmi_functions.h"
#include "qmi_util.h"

// context
typedef struct
{
    QmiDevice* device;
    QmiClientPds* client;
    GCancellable* cancellable;
    bool is_enabled;
    bool is_tracking;
    bool is_reporting;
} Context;
static Context* g_context;
static bool foundNMEA;

static bool isContextValid(Context* context)
{
    if (!g_context)
        return false;;

    if (!g_context->device)
        return false;

    if (!g_context->client)
        return false;

    return true;
}

static void context_free(Context* context)
{
    if (!context)
        return;
    if (context->client)
        g_object_unref(context->client);

    g_object_unref(context->cancellable);
    g_object_unref(context->device);
    g_slice_free(Context, context);
}

static void operation_shutdown(gboolean operation_status)
{
    //Cleanup context and finish async operation
    context_free(g_context);
    qmicli_async_operation_done(operation_status);
}

static void ser_location_disable_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsSetEventReportOutput* output = NULL;
    GError* error = NULL;

    output = qmi_client_pds_set_event_report_finish(client, res, &error);
    if (!output)
    {
        g_printerr("error: operation failed: %s\n", error->message);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_event_report_output_get_result(output, &error))
    {
        g_printerr("error: couldn't set event report: %s\n", error->message);
        qmi_message_pds_set_event_report_output_unref(output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_event_report_output_get_result(output, &error))
    {
        g_printerr("error: couldn't set event report enabled/disabled: %s\n", error->message);
        qmi_message_pds_set_event_report_output_unref(output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    qmi_message_pds_set_event_report_output_unref(output);

    g_print("GPS Reporting Stopped\n"); //done
    g_context->is_reporting = false;
    operation_shutdown(TRUE);
}

static void qmicli_pds_disable_service_event_reporting()
{
    g_print("Attempting to disable service event reporting\n");
    QmiMessagePdsSetEventReportInput* input;
    input = qmi_message_pds_set_event_report_input_new();
    qmi_message_pds_set_event_report_input_set_nmea_position_reporting(input, FALSE, NULL);
    qmi_client_pds_set_event_report(g_context->client, input, 5, g_context->cancellable,
                                    (GAsyncReadyCallback)ser_location_disable_ready, NULL);
    qmi_message_pds_set_event_report_input_unref(input);
}

static void location_event_report_indication_callback(QmiClientPds* client, QmiIndicationPdsEventReportOutput* output)
{
    QmiPdsPositionSessionStatus session_status;
    const gchar* nmea;


    if (qmi_indication_pds_event_report_output_get_position_session_status(output, &session_status, NULL))
    {
        g_print("[GPS] session status changed: '%s'\n",
                    qmi_pds_position_session_status_get_string(session_status));
    }

    if (qmi_indication_pds_event_report_output_get_nmea_position(output, &nmea, NULL))
    {
        bool success = qmi_parse_nmea(nmea);
        if (success && !foundNMEA)
        {
            foundNMEA = success;
            qmicli_pds_disable_service_event_reporting();
        }
    }

}

static void ser_location_enable_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsSetEventReportOutput* output = NULL;
    GError* error = NULL;

    output = qmi_client_pds_set_event_report_finish(client, res, &error);
    if (!output)
    {
        g_printerr("error: operation failed: %s\n", error->message);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_event_report_output_get_result(output, &error))
    {
        g_printerr("error: couldn't set event report: %s\n", error->message);
        qmi_message_pds_set_event_report_output_unref(output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_event_report_output_get_result(output, &error))
    {
        g_printerr("error: couldn't set event report enabled/disabled: %s\n", error->message);
        qmi_message_pds_set_event_report_output_unref(output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    qmi_message_pds_set_event_report_output_unref(output);

    foundNMEA = false;
    g_print("Adding location event report indication handling\n");
    g_signal_connect(client, "event-report", G_CALLBACK(location_event_report_indication_callback), NULL);
    g_print("GPS Reporting Started\n"); //done
    g_context->is_reporting = true;
}

static void qmicli_pds_enable_service_event_reporting()
{
    g_print("Attempting to enable service event reporting\n");

    // For now, only gather standard NMEA traces
    QmiMessagePdsSetEventReportInput* input;
    input = qmi_message_pds_set_event_report_input_new();
    qmi_message_pds_set_event_report_input_set_nmea_position_reporting(input, TRUE, NULL);
    qmi_client_pds_set_event_report(g_context->client, input, 5, g_context->cancellable,
                                    (GAsyncReadyCallback)ser_location_enable_ready, NULL);
    qmi_message_pds_set_event_report_input_unref(input);
}

static void auto_tracking_state_start_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsSetAutoTrackingStateOutput* output = NULL;
    GError* error = NULL;

    output = qmi_client_pds_set_auto_tracking_state_finish(client, res, &error);
    if (!output)
    {
        g_printerr("error: operation failed: %s\n", error->message);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_auto_tracking_state_output_get_result(output, &error))
    {
        if (!g_error_matches(error, QMI_PROTOCOL_ERROR, QMI_PROTOCOL_ERROR_NO_EFFECT))
        {

            qmi_message_pds_set_auto_tracking_state_output_unref(output);
            g_error_free(error);
            //communicate that GPS is tracking
            g_printerr("error: couldn't set auto tracking state: %s\n", error->message);
            g_context->is_tracking = false;
            operation_shutdown(FALSE);
            return;
        }
        g_error_free(error);
    }

    qmi_message_pds_set_auto_tracking_state_output_unref(output);

    //communicate that GPS is tracking
    g_print("Auto tracking enabled\n");
    g_context->is_tracking = true;

    if (qmicli_pds_option_get_lat_long_enabled())
        qmicli_pds_enable_service_event_reporting();
    else
        operation_shutdown(TRUE);
}

static void qmicli_pds_enable_auto_tracking()
{
    g_print("Attempting to enable auto tracking...\n");
    QmiMessagePdsSetAutoTrackingStateInput* input = NULL;

    //enable auto tracking for a continuous fix
    input = qmi_message_pds_set_auto_tracking_state_input_new();
    qmi_message_pds_set_auto_tracking_state_input_set_state(input, TRUE, NULL);
    qmi_client_pds_set_auto_tracking_state(g_context->client, input, 10, g_context->cancellable,
                                          (GAsyncReadyCallback)auto_tracking_state_start_ready, NULL);
    qmi_message_pds_set_auto_tracking_state_input_unref(input);
}

static void qmicli_pds_disable_auto_tracking()
{
    operation_shutdown(TRUE);
}

static void set_gps_service_state_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsSetGpsServiceStateOutput* gps_state_output = NULL;
    GError* error = NULL;

    gps_state_output = qmi_client_pds_set_gps_service_state_finish(client, res, &error);
    if (!gps_state_output)
    {
        g_printerr("error: operation failed: %s\n", error->message);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_set_gps_service_state_output_get_result(gps_state_output, &error))
    {
        g_printerr("error: couldn't set gps service state: %s\n", error->message);
        qmi_message_pds_set_gps_service_state_output_unref(gps_state_output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    qmi_message_pds_set_gps_service_state_output_unref(gps_state_output);
    if (qmicli_pds_option_auto_tracking_enabled())
        qmicli_pds_enable_auto_tracking();
    else
        qmicli_pds_disable_auto_tracking();
}

static void get_gps_service_state_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsGetGpsServiceStateOutput* gps_state_output;
    GError* error = NULL;

    gboolean gps_service_state;
    QmiPdsTrackingSessionState tracking_session_state;

    gps_state_output = qmi_client_pds_get_gps_service_state_finish(client, res, &error);
    if (!gps_state_output)
    {
        g_printerr("error: operation failed: %s\n", error->message);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    if (!qmi_message_pds_get_gps_service_state_output_get_result(gps_state_output, &error))
    {
        g_printerr("error: couldn't get gps service state: %s\n", error->message);
        qmi_message_pds_get_gps_service_state_output_unref(gps_state_output);
        g_error_free(error);
        operation_shutdown(FALSE);
        return;
    }

    qmi_message_pds_get_gps_service_state_output_get_state(gps_state_output, &gps_service_state, &tracking_session_state, NULL);

    g_print("[%s] Successfully got gps service state\n"
            "State: '%s'\n",
            qmi_device_get_path_display(g_context->device),
            qmi_pds_tracking_session_state_get_string(tracking_session_state));
    qmi_message_pds_get_gps_service_state_output_unref(gps_state_output);

    if (tracking_session_state == QMI_PDS_TRACKING_SESSION_STATE_ACTIVE)
    {
        if (qmicli_pds_option_auto_tracking_enabled())
            qmicli_pds_enable_auto_tracking();
        else
            qmicli_pds_disable_auto_tracking();
        return;
    }

    g_print("Attempting to enable gps service state\n");

    QmiMessagePdsSetGpsServiceStateInput* service_state_input;
    service_state_input = qmi_message_pds_set_gps_service_state_input_new();
    qmi_message_pds_set_gps_service_state_input_set_state(service_state_input, TRUE, NULL);

    qmi_client_pds_set_gps_service_state(client, service_state_input, 10, g_context->cancellable,
                                        (GAsyncReadyCallback)set_gps_service_state_ready, NULL);
    qmi_message_pds_set_gps_service_state_input_unref(service_state_input);
}

static void qmicli_pds_enable_gps_service()
{
    g_print("Asynchronously attempting to enable gps service...\n");
    qmi_client_pds_get_gps_service_state(g_context->client,
                                        NULL,
                                        10,
                                        g_context->cancellable,
                                        (GAsyncReadyCallback)get_gps_service_state_ready,
                                        NULL);
}


static void reset_ready(QmiClientPds* client, GAsyncResult* res)
{
    QmiMessagePdsResetOutput* output;
    GError* error = NULL;

    output = qmi_client_pds_reset_finish(client, res, &error);
}

static void qmicli_pds_disable_gps_service()
{
    operation_shutdown(TRUE);
}

void qmicli_pds_run(QmiDevice *device, QmiClientPds *client, GCancellable *cancellable)
{
    // Initialize context
    g_context = g_slice_new(Context);
    g_context->device = g_object_ref(device);
    g_context->client = g_object_ref(client);
    g_context->cancellable = g_object_ref(cancellable);

    g_context->is_enabled = false;
    g_context->is_tracking = false;
    g_context->is_reporting = false;

    if (qmicli_pds_option_gps_enabled())
        qmicli_pds_enable_gps_service();
    else
        qmicli_pds_disable_gps_service();
}

