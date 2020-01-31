#ifndef QMI_FUNCTIONS_H
#define QMI_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <glib-2.0/glib.h>
#include <glib-2.0/glib/gprintf.h>
#include <glib-2.0/gio/gio.h>

#include <libqmi-glib/libqmi-glib.h>


//Common
bool execute_qmi_gps_task(char* device_path, bool gps, bool autotracking, bool latlong);
void qmicli_async_operation_done(gboolean reported_operation_status);
void qmicli_async_operation_done_with_lat_long(gboolean reported_operation_status, double lat, double long);

//Options
bool qmicli_pds_option_gps_enabled();
bool qmicli_pds_option_auto_tracking_enabled();
bool qmicli_pds_option_get_lat_long_enabled();

//PDS Group
void qmicli_pds_run (QmiDevice *device, QmiClientPds *client, GCancellable *cancellable);


#endif // QMI_FUNCTIONS_H
