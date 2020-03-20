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
 * Version v2.0 20-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
int statuss = 0;                  //MFRC STATUS
int out = 0;                      //MFRC OUT

// Network Credentials
const char *ssid = "";     //Network SSID
const char *password = ""; //Network PASSWORD

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
// Current time
unsigned long alarmCurrentTime = millis();
// Previous time
unsigned long alarmPreviousTime = 0;
// Define siren sound time in case of alarm (in milliseconds, example: 60000ms = 60s)
const long alarmTimeout = 10000;

// Board Pin setup
const int doorPin = D1, boardLed = D0, h24Pin = D2, activeAlarmLed = D8;

// Define flags
bool alarmActive = false, inAlarm = false;

void setup()
{
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(h24Pin, INPUT_PULLUP);
  pinMode(boardLed, OUTPUT);
  pinMode(activeAlarmLed, OUTPUT);
  digitalWrite(boardLed, LOW);
  digitalWrite(boardLed, HIGH);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("Connected. Alarm ready.");
  server.begin();
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients
  rfidCardScanner();
  // H24 Line protection
  if (digitalRead(h24Pin) == HIGH)
  {
    inAlarm = true;
  }

  // Alarm active control
  if (alarmActive)
  {
    digitalWrite(activeAlarmLed, HIGH);

    if (digitalRead(doorPin) == HIGH)
    { //If door is opened, start the siren alarm
      inAlarm = true;
    }
  }
  else
  {
    digitalWrite(boardLed, HIGH);
    digitalWrite(activeAlarmLed, LOW);
  }

  alarmCurrentTime = millis();
  if (inAlarm)
  {
    if (alarmCurrentTime - alarmPreviousTime <= alarmTimeout)
    {
      digitalWrite(boardLed, LOW);
    }
    else
    {
      inAlarm = false;
      alarmActive = false;
      digitalWrite(boardLed, HIGH);
    }
  }
  else
  {
    alarmPreviousTime = alarmCurrentTime;
    digitalWrite(boardLed, HIGH);
  }

  if (client)
  { // If a new client connects,
    //Serial.println("New Client.");          // print a message out in the serial port
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
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Verify if are present commands in HTTP header
            if (header.indexOf("GET /1/on") >= 0)
            {
              alarmActive = true; //Set alarm flag true (Alarm is active)
              client.println("{\"result\":true,\"msg\":\"Alarm activated\"}");
            }
            else if (header.indexOf("GET /1/off") >= 0)
            {
              alarmActive = false; //Set alarm flag false (Alarm is deactivated)
              inAlarm = false;     //Set the alarm status to false (Siren doesn't sound)
              client.println("{\"result\":true,\"msg\":\"Alarm deactivated\"}");
            }
            else if (header.indexOf("GET /status") >= 0)
            {
              client.println(digitalRead(doorPin) == HIGH ? "{\"door\":\"open\"" : "{\"door\":\"closed\"");
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
  if (content.substring(1) == "92 92 AA 89") //change UID of the card that you want to give access
  {
    Serial.println(" Access Granted ");
    if (inAlarm == true)
    {
      inAlarm = false;
      alarmActive = false;
      alarmPreviousTime = 0;
      digitalWrite(boardLed, HIGH);
    }
    else
      alarmActive = !alarmActive;
    statuss = 1;
    delay(1000);
  }
  else
  {
    Serial.println(" Access Denied ");
    delay(3000);
  }
}
