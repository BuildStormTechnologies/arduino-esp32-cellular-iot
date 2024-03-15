/**
 * \copyright Copyright (c) 2019-2024, Buildstorm Pvt Ltd
 *
 * \file app_config.h
 * \brief app_config header file.
 *
 * The app_config Contains macros for the application.
 *
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define TEST_WIFI_SSID "Your SSID"
#define TEST_WIFI_PASSWORD "wifi password"
#define AWS_IOT_THING_NAME "thing name"
#define AWS_IOT_MQTT_HOST "Your host"
#define AWS_IOT_MQTT_PORT 8883
#define APP_VERSION "1.0.0"

#define TEST_AWS_TOPIC_PUBLISH "testPub/ESP32"
#define TEST_AWS_TOPIC_SUBSCRIBE "testSub/ESP32"


#define LED0_PIN 2

#define MODEM_POWERKEY_GPIO_PIN 5
#define MODEM_RESETKEY_GPIO_PIN 4
#define MODEM_RX_UART_PIN 16
#define MODEM_TX_UART_PIN 17
#define MODEM_UART_NUM 2

#define APN "airtelgprs.com"
#define USERID ""
#define PASSWORD ""



#ifdef __cplusplus
}
#endif

#endif //_APP_CONFIG_H_
