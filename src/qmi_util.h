#ifndef QMI_UTIL_H
#define QMI_UTIL_H

bool qmi_parse_nmea(char *buffer);
bool initialize_QMI_PDS(char* path, uint16_t len);
bool get_current_gps(double* longitude, double* latitude);

#endif
