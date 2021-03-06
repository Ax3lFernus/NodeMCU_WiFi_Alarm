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
 * Version v2.2 29-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Network and UID Credentials
const char *ssid = "";     //Network SSID
const char *password = ""; //Network PASSWORD
const char *UID = "";      //UID Card Code
const char *API_KEY = "";  //API KEY

// Set WebServer Port to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Alarm time counting
unsigned long alarmPreviousTime = 0;
// Define siren sound time in case of alarm (in milliseconds, example: 10000ms = 10s)
const long alarmTimeout = 180000;

// Door time counting
// Previous time
unsigned long doorExitPreviousTime = 0, doorEnterPreviousTime = 0;
// Defines the entry time since the door is opened (in milliseconds, example: 10000ms = 10s)
const long doorEnterTimeout = 10000;
// Defines the exit time since the alarm is set to active (in milliseconds)
const long doorExitTimeout = 10000;

// Tamper time counting
unsigned long tamperPreviousTime = 0;
// Time that allows the tamper line to be opened when the alarm is deactivated (in milliseconds, example: 10000ms = 10s)
const long tamperTimeout = 10000;

// Board Pin setup
const int doorPin = D1, sirenPin = D0, h24Pin = D2, activeAlarmPin = D8;

// Define flags
bool alarmActive = false, inAlarm = false, doorOpened = false, tamperOpened = false;

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
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println(" Alarm ready.");
  server.begin();
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  tamperPreviousTime = millis();
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients
  rfidCardScanner();
  alarmCheck();
  sirenCheck();
  if (client)
  {                          // If a new client connects,
    String currentLine = ""; // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Access-Control-Max-Age: 2520");
            client.println("Access-Control-Allow-Methods: GET, PUT, DELETE, XMODIFY");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();

            // Verify if are present commands in HTTP header
            if (header.indexOf("GET /1/on") >= 0)
            {
              setAlarm(true);
              client.print("{\"result\":");
              if (!alarmActive)
              {
                client.print("false,\"msg\":\"Error. Check the tamper line.\"}");
              }
              else
              {
                client.print("true,\"msg\":\"Alarm activated\"}");
              }
            }
            else if (header.indexOf("GET /1/off") >= 0)
            {
              setAlarm(false);
              client.print("{\"result\":true,\"msg\":\"Alarm deactivated\"}");
            }
            else if (header.indexOf("GET /status") >= 0)
            {
              client.print("{\"door\":");
              client.print(digitalRead(doorPin) == HIGH ? "\"open\"" : "\"closed\"");
              client.print(", \"h24\":");
              client.print(digitalRead(h24Pin) == HIGH ? "\"open\"" : "\"closed\"");
              client.print(", \"active\":");
              client.print(alarmActive ? "true" : "false");
              client.print(", \"alarm\":");
              client.print(inAlarm ? "true" : "false");
              client.print("}");
            }
            // The HTTP response ends with another blank line
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
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
