#ifndef RS2000_CONFIG_H
#define RS2000_CONFIG_H

#include <stdint.h>
#include "../ipj_util_lt.h"
#include "../impinj/iri_lt.h"
#include "../impinj/platform.h"

#define MAX_ANTENNA_COUNT 16

typedef struct
{
    uint32_t serial_number;
    uint32_t bootstrap_version;
    uint32_t bootstrap_crc;
    uint32_t application_version;
    uint32_t application_crc;
    uint32_t microprocessor_id;
    uint32_t microprocessor_id_bank1;
    uint32_t microprocessor_id_bank2;
    uint32_t microprocessor_id_bank3;
} RS2000_Infos;

typedef struct
{
    bool active;
} AntennaSettings;

typedef struct
{
    AntennaSettings antenna[MAX_ANTENNA_COUNT];
    unsigned int antennaPower;
} ActiveAntennas;

void resetAntennaConfig();
ipj_error applyAntennaConfig(ipj_iri_device* iri_device);
void setAntennaTransmitPower(uint32_t power);
uint32_t getAntennaTransmitPower();
void enableAntenna(uint16_t antenna);
void disableAntenna(uint16_t antenna);

void getRS2000Temperatures(ipj_iri_device* iri_device, uint32_t *internal_temp, uint32_t *pa_temp);
void setTagReportFlags(uint32_t flagBits, ipj_iri_device* iri_device);

void retrieveRS2000Information(ipj_iri_device *iri_device);
#endif // RS2000_CONFIG_H
