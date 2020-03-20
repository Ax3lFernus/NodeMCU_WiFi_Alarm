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
 * Version v1.0 18-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Network Credentials
const char* ssid     = "FASTWEB-RG6JPF";
const char* password = "WBNB7DKIDC";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Pin setup
const int doorPin = D1, boardLed = D0, h24Pin = D2, activeAlarmLed = D3;

bool alarmActive = false, inAlarm = false;

// Alarm time counting
// Current time
unsigned long alarmCurrentTime = millis();
// Previous time
unsigned long alarmPreviousTime = 0; 
// Define timeout time in milliseconds (example: 60000ms = 60s)
const long alarmTimeout = 10000;

void setup() {
  pinMode(doorPin,INPUT_PULLUP);
  pinMode(h24Pin,INPUT_PULLUP);
  pinMode(boardLed,OUTPUT);
  pinMode(activeAlarmLed,OUTPUT);
  digitalWrite(boardLed, LOW);
  digitalWrite(boardLed, HIGH);
  
  Serial.begin(115200);
  
  // Connect to Wi-Fi network with SSID and password
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  // Print local IP address and start web server
  //Serial.println("");
  Serial.println("WiFi connected.");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  
  if(digitalRead(h24Pin) == HIGH){
    inAlarm = true;
  }
  
  if(alarmActive){
    digitalWrite(activeAlarmLed, HIGH);
    
    if(digitalRead(doorPin) == HIGH){
      inAlarm = true;
    }
  }else{
    digitalWrite(boardLed, HIGH);
    digitalWrite(activeAlarmLed, LOW);
  }

  alarmCurrentTime = millis();
  if(inAlarm){
    if(alarmCurrentTime - alarmPreviousTime <= alarmTimeout){
      digitalWrite(boardLed, LOW);
    }else{
      inAlarm = false;
      alarmActive = false;  
      digitalWrite(boardLed, HIGH);
    }
  }else{
    alarmPreviousTime = alarmCurrentTime;
    digitalWrite(boardLed, HIGH);
  }
  
  if (client) {                             // If a new client connects,
    //Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {            
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
            if (header.indexOf("GET /1/on") >= 0) {
              alarmActive = true; //Set alarm flag true (Alarm is active)
              //digitalWrite(boardLed, LOW);
              client.println("{\"result\":true,\"msg\":\"Alarm activated\"}");
            }else if(header.indexOf("GET /1/off") >= 0){
              alarmActive = false; //Set alarm flag false (Alarm is deactivated)
              inAlarm = false;
              //digitalWrite(boardLed, HIGH);
              client.println("{\"result\":true,\"msg\":\"Alarm deactivated\"}");
            }else if(header.indexOf("GET /status") >=0){
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
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    //Serial.println("Client disconnected.");
    //Serial.println("");
  }
}
