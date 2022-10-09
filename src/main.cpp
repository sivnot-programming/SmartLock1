#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "defines.h"
#include "Credentials.h"
#include "dynamicParams.h"
ESP_WiFiManager_Lite *ESP_WiFiManager;

/* Cek Koneksi Internet */
const char *remote_host = "www.google.com";
String status_connected = "config";
boolean retry = false;

int button = D1; // push button is connected
int temp = HIGH; // temporary variable for reading the button pin status

/* Fungsi JSON Web Service */
const char *host = "https://smartlock.sivnot.com/";
const int httpsPort = 443;                                                 // HTTPS= 443 and HTTP = 80
String GetAddress = "GetData.php?function=get_statusalat_id";              // https://smartlock.sivnot.com/GetData.php?
String GetAllStatusAddress = "GetData.php?function=get_statusalat";        // function=get_statusalat_id&id=1
String UpdateStatusAlatAddress = "GetData.php?function=update_statusalat"; // GetAddress = "GetData.php";
String GetAllUsersAddress = "GetData.php?function=get_users";
String UpdateUsersAddress = "GetData.php?function=update_users";
//----------------------------------------
// SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "F8 EE 7A 63 7B 55 68 B6 C9 37 0C D9 46 74 1B 58 F8 2E E0 FF";
uint8_t readnumber(void);
String cekStatusAlat(String Url, String SerialNumber);
String LinkGet, getData, fullURL, fullURLGET;
String payload, boardCodeName, chipID, macAddres, statusAlat;
String sn;        //--> ID in Database
HTTPClient https; //--> Declare object of class HTTPClient
WiFiClientSecure wifiClient;

void heartBeatPrint()
{
  // Ping ping = new Ping();
  static int num = 1;
  if (WiFi.status() == WL_CONNECTED)
  {
    if (retry)
    {
      Serial.println("yes"); // H means connected to WiFi
      Serial.print("Pinging host ");
      Serial.println(remote_host);
      if (Ping.ping(remote_host))
      {
        Serial.println("Internet Connected!!");
        status_connected = "yes";
      }
      else
      {
        Serial.println("No Internet :(");
        status_connected = "no";
      }
    }
  }
  else
  {
    if (ESP_WiFiManager->isConfigMode())
    {
      Serial.println("config"); // C means in Config Mode
      status_connected = "config";
    }
    else
    {
      Serial.println("no"); // F means not connected to WiFi
      status_connected = "no";
    }
  }
  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void check_status()
{
  static unsigned long checkstatus_timeout = 0;
  // KH
#define HEARTBEAT_INTERVAL 20000L
  // Print hearbeat every HEARTBEAT_INTERVAL (20) seconds.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = millis() + HEARTBEAT_INTERVAL;
  }
}

#if USING_CUSTOMS_STYLE
const char NewCustomsStyle[] /*PROGMEM*/ = "<style>div,input{padding:5px;font-size:1em;}input{width:95%;}body{text-align: center;}\
button{background-color:blue;color:white;line-height:2.4rem;font-size:1.2rem;width:100%;}fieldset{border-radius:0.3rem;margin:0px;}</style>";
#endif

/* Fungsi JSON Web Service */
uint8_t readnumber(void)
{
  uint8_t num = 0;

  while (num == 0)
  {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

String cekStatusAlat(String Url, String SerialNumber)
{
  //  char* statusAlat;
  getData = "sn=" + String(SerialNumber);
  Serial.println(" ----------------cek Status Alat-----------------");
  Serial.print("Request Link : ");
  Serial.println(Url + "&" + getData);
  fullURLGET = Url + "&" + getData;
  //  HTTPClient https; //--> Declare object of class HTTPClient
  wifiClient.setFingerprint(fingerprint);
  wifiClient.setTimeout(10000);        // 10 Seconds
  https.begin(wifiClient, fullURLGET); //--> Specify request GET destination
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  //  int httpCodeGet = https.POST(getData);
  int httpCodeGet = https.GET();
  https.GET();
  //  int httpCodeGet = https.GET(getData);
  Serial.print("Get Data JSON: ");
  Serial.println(https.getString());
  // Serial.print("httpCodeGet: ");
  // Serial.println(httpCodeGet);
  if (httpCodeGet > 0)
  {
    Serial.print("Response Code : ");
    Serial.println(httpCodeGet); //--> Print HTTP return code
    if (httpCodeGet == HTTP_CODE_OK)
    {
      payload = https.getString();
      Serial.print("payload = ");
      Serial.println(payload);
    }
  }
  else
  {
    Serial.println();
    Serial.printf("[HTTP] ... failed, error: %s\n", https.errorToString(httpCodeGet).c_str());
  }
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    //    char* errorString = "deserializeJson() failed: "+error.f_str();
    return "-1";
  }
  JsonObject data_0 = doc["data"][0];
  const char *data_0_id = data_0["id"];             // "1"
  const char *data_0_status = data_0["status"];     // "1"
  const char *data_0_mode = data_0["mode"];         // "0"
  const char *data_0_textmode = data_0["textmode"]; // "Presensi"
  int data_0_statuscode = data_0["statuscode"];     // 200

  // Serial.print("data status: ");
  // Serial.println(data_0_status);
  // Serial.print("data mode: ");
  // Serial.println(data_0_mode);
  String statusAlat = data_0_mode;

  return statusAlat;
}

void setup()
{
  // ESP_WiFiManager->resetSettings();
  // Debug console
  Serial.begin(115200);
  // setupLED();
  // ledOFF();
  pinMode(button, INPUT);
  digitalWrite(button, HIGH); // activate arduino internal pull up
  while (!Serial)
    ;
  delay(200);
  // Serial.print(F("\nStarting ESP_WiFi using "));
  // Serial.print(FS_Name);
  // Serial.print(F(" on "));
  // Serial.println(ARDUINO_BOARD);
  // Serial.println(ESP_WIFI_MANAGER_LITE_VERSION);

#if USING_MRD
  Serial.println(ESP_MULTI_RESET_DETECTOR_VERSION);
#else
  Serial.println(ESP_DOUBLE_RESET_DETECTOR_VERSION);
#endif

  ESP_WiFiManager = new ESP_WiFiManager_Lite();
  //  ESP_WiFiManager.REQUIRE_ONE_SET_SSID_PW=false;
  String esp_chip = String(ESP_getChipId(), HEX);
  String AP_SSID = "MySmartLock_" + esp_chip;
  String AP_PWD = "123456789";
  Serial.print("AP_SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP_PWD: ");
  Serial.println(AP_PWD);
  // Set customized AP SSID and PWD
  ESP_WiFiManager->setConfigPortal(AP_SSID, AP_PWD);
  // Optional to change default AP IP(192.168.4.1) and channel(10)
  // ESP_WiFiManager->setConfigPortalIP(IPAddress(192, 168, 120, 1));
  ESP_WiFiManager->setConfigPortalChannel(0);
  uint8_t ip1 = 192;
  uint8_t ip2 = 168;
  uint8_t ip3 = 100;
  uint8_t ip4 = 1;
  /* IP : 192.168.100.1 */
  ESP_WiFiManager->setConfigPortalIP(IPAddress(ip1, ip2, ip3, ip4));
#if USING_CUSTOMS_STYLE
  ESP_WiFiManager->setCustomsStyle(NewCustomsStyle);
#endif

#if USING_CUSTOMS_HEAD_ELEMENT
  ESP_WiFiManager->setCustomsHeadElement("<style>html{filter: invert(10%);}</style>");
#endif

#if USING_CORS_FEATURE
  ESP_WiFiManager->setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // Set customized DHCP HostName
  //  ESP_WiFiManager->begin(HOST_NAME);
  // Or use default Hostname "ESP32-WIFI-XXXXXX"
  // ESP_WiFiManager->begin();
  // Set customized DHCP HostName
  String DHCP_Host_Name = "MySmartLock_" + String(ESP_getChipId(), HEX);
  // ESP_WiFiManager->setConfigPortalIP(IPAddress(192, 168, 100, 1));
  ESP_WiFiManager->begin("MySmartLock_");

  boardCodeName = ESP_WiFiManager->getBoardName();
  chipID = String(ESP_getChipId(), HEX);
  macAddres = WiFi.macAddress();
  chipID.toUpperCase();
  sn = chipID; //--> ID in Database
}

#if USE_DYNAMIC_PARAMETERS
void displayCredentials()
{
  Serial.println(F("\nYour stored Credentials :"));

  for (uint16_t i = 0; i < NUM_MENU_ITEMS; i++)
  {
    Serial.print(myMenuItems[i].displayName);
    Serial.print(F(" = "));
    Serial.println(myMenuItems[i].pdata);
  }
}

void displayCredentialsInLoop()
{
  static bool displayedCredentials = false;
  if (!displayedCredentials)
  {
    for (int i = 0; i < NUM_MENU_ITEMS; i++)
    {
      if (!strlen(myMenuItems[i].pdata))
      {
        break;
      }
      if (i == (NUM_MENU_ITEMS - 1))
      {
        displayedCredentials = true;
        // displayCredentials();
      }
    }
  }
}
#endif

void loop()
{
  ESP_WiFiManager->run();
  check_status();
  if (status_connected == "yes")
  {
    retry = false;
    // Serial.println("Connected Network");
    Serial.println("c");
    // sn = chipID; //--> ID in Database
    // //  https://smartlock.sivnot.com/GetData.php?function=get_statusalat_id&id=1
    // //  GetAddress = "GetData.php";
    // GetAddress = "GetData.php?function=get_statusalat_id";
    LinkGet = host + GetAddress; //--> Make a Specify request destination
    statusAlat = cekStatusAlat(LinkGet, sn);
    if (statusAlat == "1")
    {
      // ledBlue();
      //    MyEnrollLoop();
      Serial.print("MODE ALAT: ");
      Serial.println("ENROLLLLLLLLLLLLLLLL");
      delay(500);
      // ledOFF();
    }
    else if (statusAlat == "2")
    {
      // ledGreen();
      delay(500);
      Serial.print("MODE ALAT: ");
      Serial.println("Cek USERRRRRRRRRRRR");
      // ledOFF();
    }
    else if (statusAlat == "3")
    {
      // ledRed();
      delay(500);
      Serial.print("MODE ALAT: ");
      Serial.println("Buka Pintuuuuuuuuuuuuuuu");
      // ledOFF();
    }
    else
    {
      // ledRed();
      // delay(100);
      // ledOFF();
      // delay(100);
      // ledRed();
      // delay(100);
      Serial.print("MODE ALAT: ");
      Serial.println("NGAPAIN YAAAAAAAAAAAAA");
      // ledOFF();
    }
    // Serial.println("----------------Closing Connection----------------");
    https.end(); //--> Close connection
                 //  Serial.println();
    Serial.println("Please wait 1 seconds for the next connection.");
    //  Serial.println();
    delay(1000); //--> GET Data at every 3 seconds
  }
  else if (status_connected == "no")
  {
    // Serial.println("Not-Connected Network");
    retry = true;
    Serial.print("nc");
    https.end(); //--> Close connection
    delay(1000);
  }
  else
  {
    // Serial.println("Need Configuration");
    retry = true;
    Serial.print("nc");
    https.end(); //--> Close connection
    delay(1000);
  }

  // temp = digitalRead(button);
  // if (temp == HIGH)
  // {
  //   ledOFF();
  // }
  // else
  // {
  //   ledRed();
  // }
  // delay(500);
}
