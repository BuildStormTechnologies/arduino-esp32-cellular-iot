 /**
 * \copyright Copyright (c) 2019-2024, Buildstorm Pvt Ltd
 *
 * \file LibModem.h
 * \brief Modem library header file.
 *
 * 
 * The Modem library handles the modem operations for connecting to MQTT & HTTP.
 * 
 * The libraries have been tested on the ESP32 modules. 
 * Buildstorm explicitly denies responsibility for any hardware failures 
 * arising from the use of these libraries, whether directly or indirectly. 
 * 
 * EULA LICENSE:
 * This library is licensed under end user license EULA agreement.
 * The EULA is available at https://buildstorm.com/eula/
 * For any support contact us at hello@buildstorm.com 
 *
 */

#ifndef _LibModem_H_
#define _LibModem_H_

#include "LibModemTypes.h"

class LibModem
{

private:
public:
    void resart();
    bool isConnected();
    bool isMqttConnected();
    const char *IP();
    int8_t rssi();
    modemStates_et state();
    const char *stateString();
    const char *model();
    const char *revision();
    const char *imei();
};

#endif //_LibModem_H_
