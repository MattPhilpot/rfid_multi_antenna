#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "bluetooth/bluetooth.h"
#include "bluetooth/hci.h"
#include "bluetooth/hci_lib.h"
#include "util_functions.h"


void bluetooth_scan()
{
    int dev_id;
    int bluetooth_socket;

    dev_id = hci_get_route(NULL);
    bluetooth_socket = hci_open_dev(dev_id);
    if (dev_id < 0 || bluetooth_socket < 0)
    {
        controller_log("Error opening bluetooth socket?");
        exit(1);
    }

    //struct hci_request scan_params_rq = hci_inquiry(dev_id, )

}

