/**
 * \copyright Copyright (c) 2019-2024, Buildstorm Pvt Ltd
 *
 * \file combined.ino
 * \brief main entry of the application.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"
#include "LibSystem.h"
#include "LibAws.h"
#include "LibJson.h"
#include "LibUtils.h"
#include "TFT_eSPI.h"
#include "icons.h"
#include "Free_Fonts.h"
#include "LibModem.h"
#include "LibWifi.h"

extern const char aws_root_ca_pem[];
extern const char certificate_pem_crt[];
extern const char private_pem_key[];

/* Config -----------------------------------------------------------------*/
// #define TFT_WIDTH 320
// #define TFT_HEIGHT 170

#define TFT_WIDTH 470
#define TFT_HEIGHT 304

// #define TFT_WIDTH 320
// #define TFT_HEIGHT 240

#define font FF6

#define pos1 1.5
#define pos2 1.88

// #define pos1 1
// #define pos2 1.41

// #define pos1 1
// #define pos2 1

//progress bar variables
int progress = 0;
int x = 26;
int y = TFT_HEIGHT-160;
int w = TFT_WIDTH - 58; // Adjusting width to fit within display
int h = 18;
int blocks = 0;
/* Config -----------------------------------------------------------------*/
TFT_eSPI tft = TFT_eSPI();

#define gray 0x6B6D


typedef enum
{
  DISPLAY_STATUS_OFF,
  DISPLAY_STATUS_NO_CHANGE,
  DISPLAY_STATUS_TOGGLE,
  DISPLAY_STATUS_ON,
} displayStatusCodes_et;

static uint8_t eventWIFIState = DISPLAY_STATUS_OFF;
static uint8_t eventBLEState = DISPLAY_STATUS_OFF;
static uint8_t eventMQTTState = DISPLAY_STATUS_OFF;
bool printStatus = false;

LibSystem SYS;
Aws AWS;
LibJson JSN;
LibUtils Utils;
LibModem Modem;
LibWifi Wifi;

/* Macros ------------------------------------------------------------------*/

#define STR_SHADOW_KEY_LED "LED"
#define STR_SHADOW_KEY_COLOR "COLOR"

#define DESIRED_LED_STATE 0
#define DESIRED_COLOR "WHITE"

/* Variables -----------------------------------------------------------------*/
static char gJobIdStr[LENGTH_JOB_ID] = {0};
static char gJobDocumentStr[LENGTH_JOB_DOCUMENT] = {0};
static bool gJobReceived_b8 = 0;

void app_init();
void app_loop();

int32_t gDesiredLedState_s32 = 0, gReportedLedState_s32 = 0;
char gDesiredColorStr[12] = DESIRED_COLOR, gReportedColorStr[12] = DESIRED_COLOR;

void classicShadowUpdateCallBack(uint8_t elementIndex_u8, const char *pKeyStr, const void *pValue);
void statusShadowUpdateCallBack(uint8_t elementIndex_u8, const char *pKeyStr, const void *pValue);

#define CLASSIC_SHADOW 0
#define STATUS_SHADOW 1
#define MAX_TYPES_OF_SHADOWS 2

const awsShadowElement_st classicShadowElements[] = {
    /* Shaodw Value type          Key         value          needToPublish   shadowUpdateType*/
    {SHADOW_VALUE_TYPE_STRING, "fw_ver", {.pStr = APP_VERSION}, true, SHADOW_UPDATE_TYPE_ALL},
    {SHADOW_VALUE_TYPE_INT, "LED",       {.val_i32 = 0},        true, SHADOW_UPDATE_TYPE_ALL},
    {SHADOW_VALUE_TYPE_STRING, "COLOR",  {.pStr = DESIRED_COLOR},       true, SHADOW_UPDATE_TYPE_ALL}};

const awsShadowElement_st statusShadowElements[] = {
    /* Shaodw Value type          Key     value     needToPublish shadowUpdateType*/
    {SHADOW_VALUE_TYPE_FLOAT, "s01", {.val_f32 = 3.1}, true, SHADOW_UPDATE_TYPE_ALL},
    {SHADOW_VALUE_TYPE_FLOAT, "s02", {.val_f32 = 3.1}, true, SHADOW_UPDATE_TYPE_ALL},
};

shadowConfigTable_st shadowTable[MAX_TYPES_OF_SHADOWS] =
    {
        {
            ptrShadowName : NULL, // NUll for classis shadow
            maxElementCount_u8 : (sizeof(classicShadowElements) / sizeof(classicShadowElements[0])),
            callbackHandler : classicShadowUpdateCallBack,
            pShadowElementsTable : classicShadowElements,
        },

        {
            ptrShadowName : "STATUS", // Named shadow
            maxElementCount_u8 : (sizeof(statusShadowElements) / sizeof(statusShadowElements[0])),
            callbackHandler : statusShadowUpdateCallBack,
            pShadowElementsTable : statusShadowElements,
        },
};

void setup()
{
  Serial.begin(115200);
  app_init();
  disp_init();
  thingVersion();
  pinMode(LED0_PIN, OUTPUT);
  AWS.subscribe((char *)TEST_AWS_TOPIC_SUBSCRIBE, 0);
}

void loop() {
  app_loop();
  delay(200);
}

void classicShadowUpdateCallBack(uint8_t elementIndex_u8, const char *pKeyStr, const void *pValue)
{
    printf("\nClassic shadow: %s", pKeyStr);
    if (elementIndex_u8 == 1) // 0-FV, 1-LED, 2-COLOR
    {
        gDesiredLedState_s32 = *(uint8_t *)pValue;
        printf("\nDelta update desired Led : %d", gDesiredLedState_s32);
    }
    else if (elementIndex_u8 == 2) // 0-FV, 1-LED, 2-COLOR
    {
        if ((strcmp((char *)pValue, "WHITE") == 0) ||
            (strcmp((char *)pValue, "RED") == 0) ||
            (strcmp((char *)pValue, "GREEN") == 0) ||
            (strcmp((char *)pValue, "BLUE") == 0))
        {
            strcpy(gDesiredColorStr, (char *)pValue);
            printf("gDesiredColorStr:%s gReportedColorStr:%s", gDesiredColorStr, gReportedColorStr);
        }
    }
}

void statusShadowUpdateCallBack(uint8_t elementIndex_u8, const char *pKeyStr, const void *pValue)
{
    printf("\nStatus shadow: %s", pKeyStr);
    AWS.shadowUpdate(STATUS_SHADOW, (char *)pKeyStr, (void *)pValue, SHADOW_UPDATE_TYPE_REPORTED); // index 1->Status
}

void app_init()
{

  systemInitConfig_st sysConfig = {0};

  sysConfig.systemEventCallbackHandler = app_eventsCallBackHandler;
  sysConfig.pAppVersionStr = (char *)APP_VERSION;
  sysConfig.pWifiSsidStr = (char *)TEST_WIFI_SSID;
  sysConfig.pWifiPwdStr = (char *)TEST_WIFI_PASSWORD;

  sysConfig.s_mqttClientConfig.maxPubMsgToStore_u8 = 5;
  sysConfig.s_mqttClientConfig.maxSubMsgToStore_u8 = 6;
  sysConfig.s_mqttClientConfig.maxSubscribeTopics_u8 = 5;
  sysConfig.s_mqttClientConfig.maxJobs_u8 = 2;
  sysConfig.s_mqttClientConfig.pThingNameStr = (char *)AWS_THING_NAME;
  sysConfig.s_mqttClientConfig.pHostNameStr = (char *)AWS_IOT_MQTT_HOST;
  sysConfig.s_mqttClientConfig.port_u16 = AWS_IOT_MQTT_PORT;
  sysConfig.s_mqttClientConfig.pRootCaStr = (char *)aws_root_ca_pem;
  sysConfig.s_mqttClientConfig.pThingCertStr = (char *)certificate_pem_crt;
  sysConfig.s_mqttClientConfig.pThingPrivateKeyStr = (char *)private_pem_key;

  sysConfig.s_modemConfig.model = QUECTEL_EC200U;
  sysConfig.s_modemConfig.uartPortNum_u8 = MODEM_UART_NUM;
  sysConfig.s_modemConfig.rxPin_u8 = MODEM_RX_UART_PIN;
  sysConfig.s_modemConfig.txPin_u8 = MODEM_TX_UART_PIN;
  sysConfig.s_modemConfig.resetKeyPin_u8 = MODEM_RESETKEY_GPIO_PIN;



  if (SYS.init(&sysConfig))
  {
    SYS.start();
    if (SYS.getMode() == SYSTEM_MODE_NORMAL)
    {
      AWS.shadowRegister(shadowTable, MAX_TYPES_OF_SHADOWS);
      if (AWS.jobRegister("blink", 0, app_jobHandlerLed))
      {
        printf("\nblink job reg success");
      }
      else
      {
        printf("blink job reg failed");
      }
    }
  }
  else
  {
    while (1)
    {
      printf("\n System Init failed, verify the init config ....");
      delay(5000);
    }
  }
}

void app_loop()
{

  static uint32_t nextPubTime = 0;
  static signed int count_u32 = 3;
  switch (SYS.getMode())
  {
  case SYSTEM_MODE_NORMAL:
    if (AWS.isConnected())
    {
      mqttMsg_st s_mqttMsg = {0};
      app_jobCheck();
      if (AWS.read(&s_mqttMsg))
      {
        printf("\n Received: [%s]:%s", s_mqttMsg.topicStr, s_mqttMsg.payloadStr);
      }
      if ((millis() > nextPubTime))
      {
        if(count_u32==3)
        {
              tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
        }
        s_mqttMsg.payloadLen_u16 = sprintf(s_mqttMsg.payloadStr, "{\"Hello from QUECTEL_EC200U\": %d}", count_u32--);
        strcpy(s_mqttMsg.topicStr, TEST_AWS_TOPIC_PUBLISH);
        s_mqttMsg.qos_e = QOS0_AT_MOST_ONCE;
        AWS.publish(&s_mqttMsg);
        nextPubTime = millis() + 1000;
        printf("\n %s\n", s_mqttMsg.payloadStr);
        messagePubScreen(s_mqttMsg.payloadStr);
        if(count_u32==0)
        {
          nextPubTime = millis() + 5000000;
        }
      }
      if (gDesiredLedState_s32 != gReportedLedState_s32)
      {
        gReportedLedState_s32 = gDesiredLedState_s32;
        digitalWrite(LED0_PIN, gDesiredLedState_s32);
        printf("\ngDesiredLedState_s32:%d gReportedLedState_s32:%d", gDesiredLedState_s32, gReportedLedState_s32);
        AWS.shadowUpdate(CLASSIC_SHADOW, STR_SHADOW_KEY_LED, &gReportedLedState_s32, SHADOW_UPDATE_TYPE_REPORTED);
      }
      else if (strcmp(gDesiredColorStr, gReportedColorStr) != 0)
      {
        strcpy(gReportedColorStr, gDesiredColorStr);
        printf("\ngDesiredColorStr:%s gReportedColorStr:%s", gDesiredColorStr, gReportedColorStr);
        AWS.shadowUpdate(CLASSIC_SHADOW, STR_SHADOW_KEY_COLOR, gReportedColorStr, SHADOW_UPDATE_TYPE_REPORTED);
        shadowScreen(gReportedLedState_s32, gReportedColorStr);
      } 
      if(count_u32==0 || APP_VERSION=="1.0.1")
      {
        shadowScreen(gReportedLedState_s32, gReportedColorStr);
      }  
    }
    aicon_status();
  break;

  case SYSTEM_MODE_OTA:

    if (SYS.getMode() == SYSTEM_MODE_OTA) 
    {
      static int i=0;
      if (SYS.getOtaPercentage() == 0)
      {
        if (i == 0) 
        {
          OTARequestScreen();
          i++;
        }
      } 
      else 
      {
        progressbar(SYS.getOtaPercentage());
      }
    }
    if(SYS.getOtaPercentage() == 100)
    {
      rebootingScreen();
      delay(10000);
    }
    aicon_status();
      break;
    default:
      break;
  }
}

jobsStatus_et app_jobHandlerLed(const job_st *ps_job)
{
  printf("\n [app_jobHandlerLed] %s : %s", ps_job->idStr, ps_job->documentStr);

  // copy gJobIdStr & gJobDocumentStr
  strcpy(gJobIdStr, ps_job->idStr);
  strcpy(gJobDocumentStr, ps_job->documentStr);

  // handle the operation in application task
  gJobReceived_b8 = true;

  return JOB_STATUS_IN_PROGRESS;
}

void app_jobCheck()
{
  // job received? perform operation
  if (gJobReceived_b8)
  {
    printf("\n app_jobCheck job received");
    jobReceivedScreen();
    gJobReceived_b8 = 0;

    uint32_t countValue_u32 = 0;
    uint32_t onTime_u32 = 0;
    uint32_t offTime_u32 = 0;
    char countStr[8] = {0};
    char onTimeStr[8] = {0};
    char offTimeStr[8] = {0};

#define MAX_KEYS 3
    // variable for extracting json key-value pair
    tagStructure_st jobKeyValuePair[MAX_KEYS] = {
      {(char*)"count", countStr},
      {(char*)"onTime", onTimeStr},
      {(char*)"offTime", offTimeStr}
    };

    // parse job document to initialize count value
    if (JSN.extractString(gJobDocumentStr, jobKeyValuePair, MAX_KEYS, 0))
    {
      countValue_u32 = Utils.getNumI32(countStr);
      onTime_u32 = Utils.getNumI32(onTimeStr);
      offTime_u32 = Utils.getNumI32(offTimeStr);

      printf("\n blink %d times => ON:%dms OFF:%dms", countValue_u32, onTime_u32, offTime_u32);
      // jobData("Blink", countValue_u32, onTime_u32, offTime_u32);
      jobData(gJobDocumentStr);
      countValue_u32 = countValue_u32 & 0x0F; // Limit to 15 loops
      for (uint32_t i = 0; i < countValue_u32; i++)
      {
        if (onTime_u32)
        {
          digitalWrite(LED0_PIN, HIGH);
          delay(onTime_u32);
        }
        if (offTime_u32)
        {
          digitalWrite(LED0_PIN, LOW);
          delay(offTime_u32);
        }
      }

      AWS.jobUpdateStatus(gJobIdStr, JOB_STATUS_SUCCESSED);
      jobSuccessScreen();
    }
    else
    {
      printf("\n Error: Invalid job document: %s", gJobDocumentStr);
      AWS.jobUpdateStatus(gJobIdStr, JOB_STATUS_FAILED);
    }
  }
}

void app_eventsCallBackHandler(systemEvents_et event_e)
{
  switch (event_e)
  {
  case EVENT_WIFI_CONNECTED:
    eventWIFIState = DISPLAY_STATUS_ON;
    printStatus = true;
    // printf("\nEVENT_WIFI_CONNECTED");
    break;
  case EVENT_WIFI_DISCONNECTED:
    eventWIFIState = DISPLAY_STATUS_OFF;
    printStatus = true;
    // printf("\nEVENT_WIFI_DISCONNECTED");
    break;

  case EVENT_MQTT_CONNECTED:
    eventMQTTState = DISPLAY_STATUS_ON;
    printStatus = true;
    printf("\nEVENT_AWS_CONNECTED");
    break;
  case EVENT_MQTT_DISCONNECTED:
    eventMQTTState = DISPLAY_STATUS_OFF;
    printStatus = true;
    printf("\nEVENT_AWS_DISCONNECTED");
    break;
  }
}

void disp_init()
{
  tft.init();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

    tft.setSwapBytes(true);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(4);
 

}

void drawHeaderFooter()
{
    tft.setTextColor(TFT_SKYBLUE);
    tft.setFreeFont(font);
    tft.setTextSize(1);
    tft.setCursor(10, 20);
    tft.print("AWS IoT");
    tft.drawRect(5, TFT_HEIGHT-33, TFT_WIDTH-10, 26, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("BLE Provisioning", TFT_WIDTH/2, TFT_HEIGHT-22);
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
}

void aicon_status()
{
  static bool blinkState = false;
  static int i=0;
  if (!(Modem.isConnected()))
  {
    if (blinkState)
    {
        tft.pushImage(TFT_WIDTH-30, 2, 25, 25, internetDiscon_icon);
    }
    else
    {
        tft.fillRect(TFT_WIDTH-30, 2, 25, 25, TFT_BLACK);
    }
    if(i==0)
    {
    conInScreen();
    i++;
    }
  }
  else
  {
      tft.pushImage(TFT_WIDTH-30, 2, 25, 25, internet_icon);
  }

  if(Wifi.isConnected())
  {
    tft.pushImage(TFT_WIDTH-30, 2, 25, 25, internet_icon);
  }

  if (((Modem.isConnected()==true) && Modem.isMqttConnected()!= true))
  {
    if (blinkState)
    {
        tft.pushImage(TFT_WIDTH-60, 4, 25, 25, awsDiscon_icon);
    }
    else
    {
        tft.fillRect(TFT_WIDTH-60, 4, 25, 25, TFT_BLACK);
    }
    if(i==1)
    {
      conAwsScreen();
      i++;
    }
  }
  else
  {
      tft.pushImage(TFT_WIDTH-60, 4, 25, 25, awsDiscon_icon);
  }
  if(Modem.isMqttConnected()||eventMQTTState==DISPLAY_STATUS_ON)
  {
     tft.pushImage(TFT_WIDTH-60, 4, 25, 25, awsCon_icon);
  }
  
  blinkState = !blinkState;
}

void thingVersion()
{
    tft.setTextColor(TFT_SKYBLUE);
    tft.setFreeFont(font);
    tft.setTextSize(1);
    tft.setCursor(10, 20);
    tft.print(AWS.getThingName());
    tft.setFreeFont(FF0);
    tft.setFreeFont(font);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Buildstorm Technologies", TFT_WIDTH/2, TFT_HEIGHT-22);
    tft.drawRect(5, TFT_HEIGHT-33, TFT_WIDTH-10, 26, TFT_WHITE);
}

void shadowScreen(int32_t ledState, char *txtColor)
{
  static uint8_t i=0;
  if(i==0)
  {
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
    i++;
  }
  tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
  tft.setFreeFont(FF6);
  int color = stringToColor(txtColor);
  tft.setTextColor(color);
  tft.drawString("fw_ver :", 50 * pos1, 45 * pos2);   
  tft.drawString("LED    :", 50 * pos1, 70 * pos2);   
  tft.drawString("COLOR  :", 50 * pos1, 95 * pos2);  
  tft.drawString("MODEM  :", 50 * pos1, 120 * pos2);  
  static int32_t prevLedState = -1;
  static char prevTxtColor[20] = "";

  // Compare the current values with the previous ones
  if (ledState != prevLedState || strcmp(txtColor, prevTxtColor) != 0) {
    // Update the display only if the value has changed
    tft.fillRect(130, 110, 200, 40, TFT_BLACK);
    tft.drawString(String(ledState), 140 * pos1, 70 * pos2);     
    prevLedState = ledState;
  }

  if (strcmp(txtColor, prevTxtColor) != 0) {
    // Update the display only if the value has changed
    tft.fillRect(130,150, 200, 40, TFT_BLACK);
    tft.drawString(txtColor, 140 * pos1, 95 * pos2);            
    strcpy(prevTxtColor, txtColor);
  }
  
  tft.drawString(String(APP_VERSION), 145 * pos1, 45 * pos2);
  tft.drawString(String(MODEM_NAME), 180 * pos1, 120 * pos2);
  thingVersion();
  tft.setFreeFont(font);
  delay(500);
}

int32_t stringToColor(char *colorStr)
{
  int32_t tftColor = {0};

  if (strcmp(colorStr, "RED") == 0)
  {
    tftColor = TFT_RED;
  }
  else if (strcmp(colorStr, "GREEN") == 0)
  {
    tftColor = TFT_DARKGREEN;
  }
  else if (strcmp(colorStr, "BLUE") == 0)
  {
    tftColor = TFT_BLUE;
  }
  else
  {
    tftColor = TFT_WHITE;
  }

  return tftColor;
}

void jobReceivedScreen()
{
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
    // tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
    tft.setFreeFont(FF6);
    tft.drawString("Job Received", TFT_WIDTH/2, TFT_HEIGHT/2); 
    delay(1000);
}

void messagePubScreen(String message)
{
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
    // tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.setFreeFont(FF6);
    tft.fillRect(TFT_WIDTH-35,TFT_HEIGHT/2 -8, 10, 20, TFT_BLACK);
    tft.drawString(String(message), TFT_WIDTH/2, TFT_HEIGHT/2);
    thingVersion();
    delay(1000);
}

void jobData(String job)
{
    tft.setFreeFont(FF6);
    tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.drawString(String(job), TFT_WIDTH/2, TFT_HEIGHT/2);
    delay(1000);
}

void OTARequestScreen()
{
  tft.drawRect  (  0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(font);
  // tft.fillRect(TFT_WIDTH-35,TFT_HEIGHT/2 -8, 10, 20, TFT_BLACK);
  tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
  tft.drawString("OTA Requested", TFT_WIDTH / 2, TFT_HEIGHT / 2);
}

void progressbar(int progress)
{
    static int i=0;
    if(i==0)
    {
      tft.fillRect(60, 120, 340, 150, TFT_BLACK);
      tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
      i++;
    }
    tft.setFreeFont(&Orbitron_Light_24);
    tft.fillRect((TFT_WIDTH / 2) -40, 60, 80, 40, TFT_BLACK);
    tft.drawString(String(progress) + "%", TFT_WIDTH / 2, 80);


    int blockWidth = 4.9;
    int blocks = progress;
    tft.drawRoundRect(x, y, w, h, 3, TFT_WHITE);

    for (int i = 0; i < blocks; i++) {
        tft.fillRect(x + 6 + i * blockWidth, y + 4, blockWidth-1, 10, TFT_GREEN);
    }
}

void rebootingScreen()
{
   tft.drawRect  (  0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
   tft.setFreeFont(font);
  //  tft.fillRect(60, 50, 340, 150, TFT_BLACK);
   tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
   tft.drawString("Rebooting...", TFT_WIDTH/2, TFT_HEIGHT/2);
   aicon_status();
   delay(5000);
}

void jobSuccessScreen()
{
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
    // tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
    tft.setFreeFont(FF6);
    tft.drawString("Job Success", TFT_WIDTH/2, TFT_HEIGHT/2); 
    delay(1000);
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
}

void conInScreen()
{
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
    // tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
    tft.setFreeFont(FF6);
    tft.drawString("Connecting to Internet..", TFT_WIDTH/2, TFT_HEIGHT/2); 
    delay(1000);
}

void conAwsScreen()
{
    tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);
    // tft.fillRect(60, 120, 340, 150, TFT_BLACK);
    tft.fillRect(10,40, TFT_WIDTH-15, 225, TFT_BLACK);
    tft.setFreeFont(FF6);
    tft.drawString("Connecting to AWS..", TFT_WIDTH/2, TFT_HEIGHT/2); 
    delay(1000);
}

