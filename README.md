# Arduino AWS Cellular IoT

The Buildstorm platform simplifies the integration of AWS IoT core features onto ESP32 Hardware, spanning from basic IoT functionalities to advanced features like OTA updates and provisioning.

The architecture is based on the core IDF APIs, complemented by a C++ wrapper tailored for application use, guaranteeing non-blocking operation of all APIs. Each user action, including HTTP requests, MQTT publishing, Shadow updates, and OTA, is logged as an event and seamlessly managed in the background. To maintain this seamless operation, the platform effectively runs a system task.

This establishes a robust foundation for your upcoming IoT project.

Supported features:
1. MQTT pub/sub
2. Shadow updates
3. Jobs
4. Web OTA using AWS jobs and S3 bucket
5. Local websever OTA
6. Provision by claim
7. BLE Device Provisioning


## Refrences
1. [Cellular Solution](https://buildstorm.com/solutions/esp32-cellular-aws-iot/)
2. [ESP32 Aws IoT Platform](https://buildstorm.com/solutions/esp32-on-aws-iot-platform/)
3. [Mobile App: User Guide](https://buildstorm.com/blog/mobile-app-user-manual/)


---
## Example Setup
1. Copy the thing/claim certificates to `Aws_IoT\examples\example\certificates.c` file
2. Update the following WiFi and AWS parameters in `app_config.h` of the example
3. This configuration will be same accross all examples

```
#define AWS_IOT_MQTT_HOST "YOUR AWS HOST"
#define AWS_IOT_MQTT_PORT  8883

#define MODEM_POWERKEY_GPIO_PIN 5
#define MODEM_RESETKEY_GPIO_PIN 4
#define MODEM_RX_UART_PIN 16
#define MODEM_TX_UART_PIN 17
#define MODEM_UART_NUM 2

#define APN "airtelgprs.com"
#define USERID ""
#define PASSWORD ""
```

---
## SoC Compatibility

While our Arduino IoT platform currently supports OTA on Wifi as arduino does not support the sdkconfig changes. Sdkconfg can be updated on the idf and thus allowing us to put the UART ISR in IRAM. This helps to perform the OTA over modem capabilities for all ESP32 SoCs. You can explore the IDF version of our platform [ESP32-idf AWS IoT Platform](https://github.com/BuildStormTechnologies/esp32_cellular_aws_iot).

---
## Arduino setting
1. ESP32 settings

    ![esp32 arduino settings](<images/esp32 arduino settings.png>)
<br />
<br />
<br />

2. ESP32S3 settings

   ![esp32s3 arduino settings](<images/esp32s3 arduino settings.png>)