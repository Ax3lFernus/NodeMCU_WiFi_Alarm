/*
 *  /$$   /$$                 /$$           /$$      /$$  /$$$$$$  /$$   /$$        /$$$$$$  /$$                                  
 * | $$$ | $$                | $$          | $$$    /$$$ /$$__  $$| $$  | $$       /$$__  $$| $$                                  
 * | $$$$| $$  /$$$$$$   /$$$$$$$  /$$$$$$ | $$$$  /$$$$| $$  \__/| $$  | $$      | $$  \ $$| $$  /$$$$$$   /$$$$$$  /$$$$$$/$$$$ 
 * | $$ $$ $$ /$$__  $$ /$$__  $$ /$$__  $$| $$ $$/$$ $$| $$      | $$  | $$      | $$$$$$$$| $$ |____  $$ /$$__  $$| $$_  $$_  $$
 * | $$  $$$$| $$  \ $$| $$  | $$| $$$$$$$$| $$  $$$| $$| $$      | $$  | $$      | $$__  $$| $$  /$$$$$$$| $$  \__/| $$ \ $$ \ $$
 * | $$\  $$$| $$  | $$| $$  | $$| $$_____/| $$\  $ | $$| $$    $$| $$  | $$      | $$  | $$| $$ /$$__  $$| $$      | $$ | $$ | $$
 * | $$ \  $$|  $$$$$$/|  $$$$$$$|  $$$$$$$| $$ \/  | $$|  $$$$$$/|  $$$$$$/      | $$  | $$| $$|  $$$$$$$| $$      | $$ | $$ | $$
 * |__/  \__/ \______/  \_______/ \_______/|__/     |__/ \______/  \______/       |__/  |__/|__/ \_______/|__/      |__/ |__/ |__/
 * 
 * NodeMcu Alarm Firmware
 * Developed by Alessandro Annese 
 * GitHub: Ax3lFernus
 * E-Mail: a.annese99@gmail.com
 * Version v2.6.1 31-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <MFRC522.h>

// Network and UID Credentials
static const char *ssid = "";             //Network SSID
static const char *password = "";         //Network PASSWORD
static const char *UID_1 = "";            //1st UID Card Code
static const char *UID_2 = "";            //2nd UID Card Code
static const char *API_KEY = "";          //API KEY
static const char *IFTTT_STATUS_URL = ""; //IFTTT Webhook URL for send status via Telegram
static const char *IFTTT_ALARM_URL = "";  //IFTTT Webhook URL for send alarm alert via Telegram
static const char *dname = "";            //Board domain address

// Create MFRC522 instance
MFRC522 mfrc522(D4, D3);

// Set WebServer (HTTPS) & Client
BearSSL::ESP8266WebServerSecure server(2000); //HTTPS PORT
HTTPClient http;

// Alarm time counting
unsigned long alarmPreviousTime = 0;
// Define siren sound time in case of alarm
static const long alarmTimeout = 30000;

// Door time counting
// Previous time
unsigned long doorExitPreviousTime = 0, doorEnterPreviousTime = 0;
// Defines the entry time since the door is opened
static const long doorEnterTimeout = 10000;
// Defines the exit time since the alarm is set to active
static const long doorExitTimeout = 10000;

// Tamper time counting
unsigned long tamperPreviousTime = 0;
// Time that allows the tamper line to be opened when the alarm is deactivated
static const long tamperTimeout = 20000;

// Board Pin setup
static const int h24Pin = A0, doorPin = D1, sirenPin = D2, activeAlarmPin = D0;

// Define flags
bool alarmActive = false, inAlarm = false, doorOpened = false, tamperOpened = false, alarmAlert = true;

// Server certificate

// Private key

void setup()
{
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(h24Pin, INPUT);
  pinMode(sirenPin, OUTPUT);
  pinMode(activeAlarmPin, OUTPUT);
  digitalWrite(sirenPin, HIGH);
  digitalWrite(activeAlarmPin, LOW);

  Serial.begin(115200);
  if (API_KEY == "" && ssid == "")
  {
    Serial.println("\nERROR: Missing variables.");
    while (true)
    {
      digitalWrite(activeAlarmPin, HIGH);
      delay(500);
      digitalWrite(activeAlarmPin, LOW);
      delay(500);
    };
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (!connectToWifi())
  {
    delay(60000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("\nConnected.");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  server.on("/", showWebpage);
  server.onNotFound(showWebpage);
  server.begin();
  Serial.println("\nAlarm ready.");
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  tamperPreviousTime = millis();
}

void loop()
{
  rfidCardScanner();
  alarmCheck();
  sirenCheck();
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();
}

void rfidCardScanner()
{
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  if (content.substring(1) == UID_1 || content.substring(1) == UID_2)
  {
    Serial.println(" Access Granted ");
    if (inAlarm == true)
    {
      setAlarm(false);
      digitalWrite(sirenPin, HIGH);
    }
    else
      setAlarm(!alarmActive);
    delay(1000);
  }
  else
  {
    Serial.println(" Access Denied ");
    delay(3000);
  }
}

/**
  * Check the alarm status.
  * If alarm on, check the pins
  * Else do nothing
  */
void alarmCheck()
{
  // H24 Line protection
  if ((!(millis() - tamperPreviousTime <= tamperTimeout)))
    if (analogRead(h24Pin) < 800 && tamperOpened == false)
    {
      inAlarm = true;
      tamperOpened = true;
    }

  // Alarm active control
  if (alarmActive && inAlarm == false)
  {
    if (!(millis() - doorExitPreviousTime <= doorExitTimeout))
    {
      if (digitalRead(doorPin) == HIGH && doorOpened == false)
      {
        doorEnterPreviousTime = millis();
        doorOpened = true;
      }

      if ((!(millis() - doorEnterPreviousTime <= doorEnterTimeout)) && doorOpened == true)
      {
        inAlarm = true;
      }
    }
  }
}

/**
  * Set the alarm status.
  * Parameters: value -> true: Set the alarm on, false: Set the alarm off
  */
void setAlarm(bool value)
{
  doorOpened = false;
  tamperOpened = false;
  if (value)
  { //SET ALARM TO ON (with exit time)
    if (analogRead(h24Pin) < 800)
    { // If h24Pin is open, send acustic feedback via siren
      digitalWrite(sirenPin, LOW);
      delay(500);
      digitalWrite(sirenPin, HIGH);
      delay(500);
      digitalWrite(sirenPin, LOW);
      delay(500);
      digitalWrite(sirenPin, HIGH);
      delay(500);
      digitalWrite(sirenPin, LOW);
      delay(500);
      digitalWrite(sirenPin, HIGH);
    }
    else
    {
      alarmActive = true;
      tamperPreviousTime = 0;
      digitalWrite(activeAlarmPin, HIGH);
      doorExitPreviousTime = millis();
    }
  }
  else
  { //SET ALARM TO OFF
    inAlarm = false;
    alarmActive = false;
    digitalWrite(activeAlarmPin, LOW);
    tamperPreviousTime = millis();
  }
  sendAlarmStatus();
}

/**
  * Check the siren status.
  * If inAlarm is true starts the siren
  * Else do nothing
  */
void sirenCheck()
{
  // In alarm status
  if (inAlarm)
  {
    //Alarm time counting
    if (millis() - alarmPreviousTime <= alarmTimeout)
    {
      digitalWrite(sirenPin, LOW);
    }
    else
    {
      digitalWrite(sirenPin, HIGH);
    }

    if (alarmAlert)
    {
      alarmAlert = false;
      sendAlarmAlert();
    }
  }
  else
  {
    alarmAlert = true;
    alarmPreviousTime = millis();
    digitalWrite(sirenPin, HIGH);
  }
}

bool connectToWifi()
{
  byte timeout = 50;

  Serial.print("\n\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected to WiFi");
      Serial.print("Server can be accessed at https://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname))
      {
        Serial.print(" or at https://");
        Serial.print(dname);
      }
      return true;
    }
    delay(5000);
    Serial.print(".");
  }

  Serial.println("\nFailed to connect to WiFi");
  Serial.println("Check network status and access data");
  Serial.println("Push RST to try again");
  return false;
}

void showWebpage()
{
  String content = "";
  if (server.hasArg("key"))
  {
    if (String(API_KEY).equals(server.arg("key")))
    {
      String op = server.arg("operation");
      if (op == "on")
      {
        setAlarm(true);
        content += "{\"result\":";
        if (!alarmActive)
        {
          content += "false,\"msg\":\"Error. Check the tamper line.\"}";
        }
        else
        {
          content += "true,\"msg\":\"Alarm activated\"}";
        }
      }
      else if (op == "off")
      {
        setAlarm(false);
        content += "{\"result\":true,\"msg\":\"Alarm deactivated\"}";
      }
      else if (op == "status")
      {
        content += "{\"door\":";
        content += digitalRead(doorPin) == HIGH ? "\"open\"" : "\"closed\"";
        content += ", \"h24\":";
        content += analogRead(h24Pin) < 800 ? "\"open\"" : "\"closed\"";
        content += ", \"active\":";
        content += alarmActive ? "true" : "false";
        content += ", \"alarm\":";
        content += inAlarm ? "true" : "false";
        content += "}";
      }
      else if (op == "ifttt")
      {
        sendAlarmStatus();
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", content);
    }
  }
  else
  {
    server.send(500);
  }
}

void sendAlarmStatus()
{
  if (IFTTT_STATUS_URL != "")
  {
    String url = "value1=";
    url += alarmActive ? "Attiva" : "Disattiva";
    url += "&value2=";
    url += digitalRead(doorPin) == HIGH ? "Aperta" : "Chiusa";
    url += "&value3=";
    url += analogRead(h24Pin) < 800 ? "Aperta" : "Chiusa";
    http.begin(IFTTT_STATUS_URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(url);
    http.end();
  }
}

void sendAlarmAlert()
{
  if (IFTTT_ALARM_URL != "")
  {
    String url = "value1=";
    url += inAlarm ? "Allarme in corso" : "Nessun'allarme";
    http.begin(IFTTT_ALARM_URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.POST(url);
    http.end();
  }
}
