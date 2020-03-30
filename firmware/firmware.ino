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
 * Version v2.4 30-03-2020
 */

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServerSecure.h>
#include <MFRC522.h>

#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Network and UID Credentials
const char *ssid = "";               //Network SSID
const char *password = "";           //Network PASSWORD
const char *UID = "";                //UID Card Code
const char *API_KEY = "";            //API KEY
const char *IFTTT_STATUS_URL = "";   //IFTTT Webhook URL for send status via Telegram
const char *IFTTT_ALARM_URL = "";    //IFTTT Webhook URL for send alarm status via Telegram
const char *dname = "allarme.local"; //Local address

// Set WebServer (HTTPS)
BearSSL::ESP8266WebServerSecure server(2000); //HTTPS PORT

// Alarm time counting
unsigned long alarmPreviousTime = 0;
// Define siren sound time in case of alarm
const long alarmTimeout = 10000;

// Door time counting
// Previous time
unsigned long doorExitPreviousTime = 0, doorEnterPreviousTime = 0;
// Defines the entry time since the door is opened
const long doorEnterTimeout = 20000;
// Defines the exit time since the alarm is set to active
const long doorExitTimeout = 5000;

// Tamper time counting
unsigned long tamperPreviousTime = 0;
// Time that allows the tamper line to be opened when the alarm is deactivated
const long tamperTimeout = 1000;

// Board Pin setup
const int doorPin = D1, sirenPin = D2, h24Pin = A0, activeAlarmPin = D8;

// Define flags
bool alarmActive = false, inAlarm = false, doorOpened = false, tamperOpened = false;

// Server certificate
static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFWTCCBEGgAwIBAgISA3PmKsx4+/CRjV2SaOUm1X6+MA0GCSqGSIb3DQEBCwUA
MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD
ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0yMDAzMTgwODU1MjFaFw0y
MDA2MTYwODU1MjFaMBwxGjAYBgNVBAMTEWZzY2dyb3VwLmRkbnMubmV0MIIBIjAN
BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmKJsKzot928k1/+3TDPMV2/6+aML
chAPJUvIe7xXFySls5QrJ1GfPX+8oT27zxIgCzCrBsia88DXKuq3bvdRBUu92xYK
zDi1ljEJldri2P0/BS8IlsbhMNUtklhCZmNY+mOD3WDU8CKU/TA2i7LWHyWO6KSA
HRASYtqJTjSrbRqfeVFKzPnz2NvGcsr+fS55vb22bK1J0I6NJ4QLkFspg4uP/BWi
D0tXRNGqgzKkN8ikKDPcteqHIm9PbVwL5DiZRq+MiDBU0yCfT/GfV7ytAuqiTyBj
Hublz+qDvw5mN309dsfIUmOuBIyyD/HZkhCCQpZqCQNivhOD2BvS9H/+hQIDAQAB
o4ICZTCCAmEwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggr
BgEFBQcDAjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTF0Smq0nD5lhSGimNbqdxw
49rPNjAfBgNVHSMEGDAWgBSoSmpjBH3duubRObemRWXv86jsoTBvBggrBgEFBQcB
AQRjMGEwLgYIKwYBBQUHMAGGImh0dHA6Ly9vY3NwLmludC14My5sZXRzZW5jcnlw
dC5vcmcwLwYIKwYBBQUHMAKGI2h0dHA6Ly9jZXJ0LmludC14My5sZXRzZW5jcnlw
dC5vcmcvMBwGA1UdEQQVMBOCEWZzY2dyb3VwLmRkbnMubmV0MEwGA1UdIARFMEMw
CAYGZ4EMAQIBMDcGCysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0dHA6Ly9j
cHMubGV0c2VuY3J5cHQub3JnMIIBAwYKKwYBBAHWeQIEAgSB9ASB8QDvAHYAb1N2
rDHwMRnYmQCkURX/dxUcEdkCwQApBo2yCJo32RMAAAFw7RGj0QAABAMARzBFAiA5
w4tgfUCgmIDyOR7bOGe6ijx+aBJ+V12lehEzX9hbyAIhAN6BeV8GTOWkdg6VRl3h
rwPBe8Tm9xj0GuyVryBOsR8RAHUAB7dcG+V9aP/xsMYdIxXHuuZXfFeUt2ruvGE6
GmnTohwAAAFw7RGj/AAABAMARjBEAiAV79ci38J+Ly/Jiz3RU77cF37vsJ9g3BVX
QTPhPSS6kwIgLS3avBpohDJg7HBuiffu/bbwWQBioRRLvKOkKEBIDmEwDQYJKoZI
hvcNAQELBQADggEBAEJ9Lr+ZjMopHqttfwamF/7DAN6lxFd2lXY0msBSUHe1rir0
UPMd5ZTE5tgywreUp+HGRd4gtn9jaSn1RJBflfmwcuChe5OKACNv4GfXJ1Tp1LOS
mCf3YOHPrn6WSYmgRp9iQiF9CQVEJqoxPgSXqc8n2ldSPabq6o5ZXYoOhxC9L9w3
gY51uUdqR5z9hz2Oi7qm0FcpN09zgiMs1H+j5xetYusfz6RN/QNEdzKN36W9fl2w
RQYbtqJ3ZkVIqU3YRMUvovxBhX2U1P8Qi3hRxxSC00gjlZhnETRNVYSaCsVY9tXp
ioHwAe5tVJwP24y3AhlVCjkJCkLtXEYOiQVB9+c=
-----END CERTIFICATE-----)EOF";

// Server private key
static const char serverKey[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCYomwrOi33byTX
/7dMM8xXb/r5owtyEA8lS8h7vFcXJKWzlCsnUZ89f7yhPbvPEiALMKsGyJrzwNcq
6rdu91EFS73bFgrMOLWWMQmV2uLY/T8FLwiWxuEw1S2SWEJmY1j6Y4PdYNTwIpT9
MDaLstYfJY7opIAdEBJi2olONKttGp95UUrM+fPY28Zyyv59Lnm9vbZsrUnQjo0n
hAuQWymDi4/8FaIPS1dE0aqDMqQ3yKQoM9y16ocib09tXAvkOJlGr4yIMFTTIJ9P
8Z9XvK0C6qJPIGMe5uXP6oO/DmY3fT12x8hSY64EjLIP8dmSEIJClmoJA2K+E4PY
G9L0f/6FAgMBAAECggEAAmEv4gmla1DkiGiQdUPueOOCTG+OD2BwOALjNIima/Iy
kibqeFG1s5oA+RuTCMKt1LbUx8WLYub8A8uGghihfv1kD50bnHz62cVYcqlntX3X
RG+sCNu/aYk9TWWKXXnJipfQF7y5fSfw3jSo+QsX+wLVJEviQnG8c6WYE+vOy2Pj
bzo7ursz9AYcWzbl8MO+HHFdrz9DSn8WNLz1a7mcUQDiDBVhOa6noepogub2ECyv
hKl17j8M8sIFLl1ACtdzO77gPwlPNYn3IXvGLKUO/g8E653qVpOqYthCJQohaGBy
6op6u4RcvkR7j/ZpWL/Ds0Hq0VowjtNl3b0sTGsBoQKBgQDWAE2b1J7WWd68iUI4
gThdv29XJ9iHInlhJwf6s54rKR6IkZ5tDcHgi6t4F4qBDd2MPjBojxdSZpQOzJ3F
Jh8AJ94CussoRI/gqb6fjnA3ecJePFko/ENrjeh9aFAXqa/HItlMuUh8XQmf9Gm8
j/1maNAe0NkeSztn+5QtYCsUYQKBgQC2lvnMKwv1KckEUZMB0zN2dVLJmWey5Juq
svXrO9B2pss5KJbTBnPmc3xnlbGKq3QImyhzCPOagVxdKC7cDDYETpVogEY7cKS/
CbEMlbCFCmA04YYIVyDDGlJ0QYC/TMINKW1BaJnYPOimIk2YTSY52T56ATrji1i3
642WhglcpQKBgHHdsW3EAUv+f6SDyCd2oj5HmP+f5PdqYXlPIlDD99nxRBzYlppi
IpHO30rEA99NOF/hyzInSPqAODFzsFLxSLd1fwymxWhEquWYjCQrzwvFjqVDqRFE
wL7ewCDSOyOMF0p3AXZwc+AwROnGS+iYGPW/uVVcPNqAnzhhxWMrc2ChAoGBAJCd
pX26/n6QOOFNxtSkNdd19zsrYIHFJTIfCj3lIyhG1SERb9v1+WgmQfs9CeCeCYoj
cfciuOa/3Kr//5VTqpjgsN+S8Y10m1ef0nX4PVYyMJ7RnLoQYCeHOpVPqCxcHDoW
fK1YrrkhDNdAxhqByGv7D8vMG5tLq9oIHB2DTfjtAoGAUTVX/7bGOQ/WJ5sZsrM3
f5HXN/TgJrm+puAQ7U42R6/FAKpwumKLChAXc5XbKXwtbPb+IBE5tEZGsoQBl3o6
w8Tfvy9aFBzEr1Ba5+AouTjDW5ZpPenV++1GT3VSV1PDecBTX+8ZpY4qknqJ1+Wk
MKEL3PWHtuLx7jy6dn3TyiM=
-----END PRIVATE KEY-----)EOF";

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
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", content);
    }
  }
  else
  {
    server.send(500);
  }
}

void setup()
{
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(h24Pin, INPUT);
  pinMode(sirenPin, OUTPUT);
  pinMode(activeAlarmPin, OUTPUT);
  digitalWrite(sirenPin, LOW);
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
      digitalWrite(sirenPin, LOW);
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
      digitalWrite(sirenPin, HIGH);
      delay(500);
      digitalWrite(sirenPin, LOW);
      delay(500);
      digitalWrite(sirenPin, HIGH);
      delay(500);
      digitalWrite(sirenPin, LOW);
      delay(500);
      digitalWrite(sirenPin, HIGH);
      delay(500);
      digitalWrite(sirenPin, LOW);
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
      digitalWrite(sirenPin, HIGH);
    }
    else
    {
      //setAlarm(false);
      digitalWrite(sirenPin, LOW);
    }
  }
  else
  {
    alarmPreviousTime = millis();
    digitalWrite(sirenPin, LOW);
  }
}
