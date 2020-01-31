#include <unistd.h>
#include "rs2000_config.h"
#include "../util_functions.h"

ActiveAntennas _antennaSettings;
RS2000_Infos _rs2000Info;

void resetAntennaConfig()
{
    int i;
    for (i = 0; i < MAX_ANTENNA_COUNT; ++i)
    {
        if (i < 3) //no more than 4 antennas right now
            _antennaSettings.antenna[i].active = true;
        else
            _antennaSettings.antenna[i].active = false;
    }
}

ipj_error applyAntennaConfig(ipj_iri_device* iri_device)
{
    int antennaPos;
    int sequencePosition = 0;
    ipj_error error = E_IPJ_ERROR_SUCCESS;

    for (antennaPos = 0; antennaPos < MAX_ANTENNA_COUNT; ++antennaPos)
    {
        if (_antennaSettings.antenna[antennaPos].active)
            error = ipj_set(iri_device, E_IPJ_KEY_ANTENNA_SEQUENCE, 0, sequencePosition++, antennaPos + 1);
        else
            error = ipj_set(iri_device, E_IPJ_KEY_ANTENNA_SEQUENCE, 0, (MAX_ANTENNA_COUNT-1) - antennaPos + sequencePosition, 0);
        if (error > 0)
                return error;
    }

    uint32_t devicePower = 0;
    ipj_get_value(iri_device, E_IPJ_KEY_ANTENNA_TX_POWER, &devicePower);

    if (devicePower != 0 && devicePower != _antennaSettings.antennaPower)
        error = ipj_set_value(iri_device, E_IPJ_KEY_ANTENNA_TX_POWER, _antennaSettings.antennaPower);

    return error;
}

void setAntennaTransmitPower(uint32_t power)
{
    _antennaSettings.antennaPower = power;
}

uint32_t getAntennaTransmitPower()
{
    return _antennaSettings.antennaPower;
}

void enableAntenna(uint16_t antenna)
{
    if (antenna > 0 && antenna <= MAX_ANTENNA_COUNT)
        _antennaSettings.antenna[antenna - 1].active = true;
}

void disableAntenna(uint16_t antenna)
{
    if (antenna > 0 && antenna <= MAX_ANTENNA_COUNT)
        _antennaSettings.antenna[antenna - 1].active = false;
}

/*----------------------------------------------------------------------*/

void getRS2000Temperatures(ipj_iri_device* iri_device, uint32_t *internal_temp, uint32_t *pa_temp)
{
    sleep(1);
    if (internal_temp)
    {
        ipj_get_value(iri_device, E_IPJ_KEY_TEMPERATURE_INTERNAL, internal_temp);
        controller_log("RS2000 Internal Temperature: %d C\n", *internal_temp);
    }

    if (pa_temp)
    {
        ipj_get_value(iri_device, E_IPJ_KEY_TEMPERATURE_PA, pa_temp);
        controller_log("RS2000 PA Temperature: %d C\n", *pa_temp);
    }
}

void setTagReportFlags(uint32_t flagBits, ipj_iri_device* iri_device)
{
    ipj_set_value(iri_device, E_IPJ_KEY_REPORT_CONTROL_TAG, flagBits);
}

//------------------------------------------------------------------------------
void retrieveRS2000Information(ipj_iri_device *iri_device)
{
    ipj_get_value(iri_device, E_IPJ_KEY_SERIAL_NUMBER, &_rs2000Info.serial_number);

    ipj_get_value(iri_device, E_IPJ_KEY_BOOTSTRAP_VERSION, &_rs2000Info.serial_number);

    ipj_get_value(iri_device, E_IPJ_KEY_BOOTSTRAP_CRC, &_rs2000Info.serial_number);

    ipj_get_value(iri_device, E_IPJ_KEY_APPLICATION_VERSION, &_rs2000Info.serial_number);

    ipj_get_value(iri_device, E_IPJ_KEY_APPLICATION_CRC, &_rs2000Info.serial_number);

    ipj_get_value(iri_device, E_IPJ_KEY_MICROPROCESSOR_ID, &_rs2000Info.serial_number);

    ipj_get(iri_device, E_IPJ_KEY_MICROPROCESSOR_ID, 0, 1, &_rs2000Info.serial_number);

    ipj_get(iri_device, E_IPJ_KEY_MICROPROCESSOR_ID, 0, 2, &_rs2000Info.serial_number);

    ipj_get(iri_device, E_IPJ_KEY_MICROPROCESSOR_ID, 0, 3, &_rs2000Info.serial_number);
}
