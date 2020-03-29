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
 * Version v2.3.1 29-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <MFRC522.h>

#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Network and UID Credentials
const char *ssid = "";     //Network SSID
const char *password = ""; //Network PASSWORD
const char *UID = "";      //UID Card Code
const char *API_KEY = "";  //API KEY

const char *dname = "allarme.local"; //Local address

// Set WebServer (HTTP & HTTPS)
BearSSL::ESP8266WebServerSecure server(443);
ESP8266WebServer serverHTTP(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();

// Alarm time counting
unsigned long alarmPreviousTime = 0;
// Define siren sound time in case of alarm
const long alarmTimeout = 180000;

// Door time counting
// Previous time
unsigned long doorExitPreviousTime = 0, doorEnterPreviousTime = 0;
// Defines the entry time since the door is opened
const long doorEnterTimeout = 10000;
// Defines the exit time since the alarm is set to active
const long doorExitTimeout = 10000;

// Tamper time counting
unsigned long tamperPreviousTime = 0;
// Time that allows the tamper line to be opened when the alarm is deactivated
const long tamperTimeout = 10000;

// Board Pin setup
const int doorPin = D1, sirenPin = D0, h24Pin = D2, activeAlarmPin = D8;

// Define flags
bool alarmActive = false, inAlarm = false, doorOpened = false, tamperOpened = false;

// Server certificate


// Server private key


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
        content += digitalRead(h24Pin) == HIGH ? "\"open\"" : "\"closed\"";
        content += ", \"active\":";
        content += alarmActive ? "true" : "false";
        content += ", \"alarm\":";
        content += inAlarm ? "true" : "false";
        content += "}";
      }

      server.send(200, "application/json", content);
    }
  }
  else
  {
    server.send(500);
  }
}

void secureRedirect()
{
  serverHTTP.sendHeader("Location", String("https://allarme.local"), true);
  serverHTTP.send(301, "text/plain", "");
}

void setup()
{
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(h24Pin, INPUT_PULLUP);
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
  WiFi.begin(ssid, password);
  while (!connectToWifi())
  {
    delay(60000);
    ESP.restart();
  }
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  serverHTTP.on("/", secureRedirect);
  serverHTTP.begin();
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
  serverHTTP.handleClient();
  server.handleClient();
  MDNS.update();
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
  if (content.substring(1) == UID) //change UID of the card that you want to give access
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
    if (digitalRead(h24Pin) == HIGH && tamperOpened == false)
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
    if (digitalRead(h24Pin) == HIGH)
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
      //setAlarm(false);
      digitalWrite(sirenPin, HIGH);
    }
  }
  else
  {
    alarmPreviousTime = millis();
    digitalWrite(sirenPin, HIGH);
  }
}
