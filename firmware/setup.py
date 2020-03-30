#
#  /$$   /$$                 /$$           /$$      /$$  /$$$$$$  /$$   /$$        /$$$$$$  /$$
# | $$$ | $$                | $$          | $$$    /$$$ /$$__  $$| $$  | $$       /$$__  $$| $$
# | $$$$| $$  /$$$$$$   /$$$$$$$  /$$$$$$ | $$$$  /$$$$| $$  \__/| $$  | $$      | $$  \ $$| $$  /$$$$$$   /$$$$$$  /$$$$$$/$$$$
# | $$ $$ $$ /$$__  $$ /$$__  $$ /$$__  $$| $$ $$/$$ $$| $$      | $$  | $$      | $$$$$$$$| $$ |____  $$ /$$__  $$| $$_  $$_  $$
# | $$  $$$$| $$  \ $$| $$  | $$| $$$$$$$$| $$  $$$| $$| $$      | $$  | $$      | $$__  $$| $$  /$$$$$$$| $$  \__/| $$ \ $$ \ $$
# | $$\  $$$| $$  | $$| $$  | $$| $$_____/| $$\  $ | $$| $$    $$| $$  | $$      | $$  | $$| $$ /$$__  $$| $$      | $$ | $$ | $$
# | $$ \  $$|  $$$$$$/|  $$$$$$$|  $$$$$$$| $$ \/  | $$|  $$$$$$/|  $$$$$$/      | $$  | $$| $$|  $$$$$$$| $$      | $$ | $$ | $$
# |__/  \__/ \______/  \_______/ \_______/|__/     |__/ \______/  \______/       |__/  |__/|__/ \_______/|__/      |__/ |__/ |__/
#
# NodeMcu Alarm Set Firmware Variables
# Developed by Alessandro Annese
# GitHub: Ax3lFernus
# E-Mail: a.annese99@gmail.com
# Version v1.4.2 30-03-2020
#
import string
import secrets
from os import system, name


def clearConsole():
    if name == 'nt':
        _ = system('cls')
    else:
        _ = system('clear')


def openFile():
    with open('firmware.ino', 'r') as file:
        data = file.readlines()
        file.close()
    return data


def saveFile(data):
    with open('firmware.ino', 'w') as file:
        file.writelines(data)
        file.close()


def setWiFi(data):
    ssid = input("Insert your Wi-Fi SSID: ")
    pw = input("Insert your Wi-Fi Password: ")
    data[27] = "static const char *ssid = \"" + \
        ssid.strip() + "\"; //Network SSID\n"
    data[28] = "static const char *password = \"" + \
        pw.strip() + "\"; //Network PASSWORD\n"


def setUID(data):
    uid = input("Enter the UID of the 1st card (empty for none): ")
    data[29] = "static const char *UID_1 = \"" + \
        uid.strip() + "\"; //1st UID Card Code\n"
    uid = input("Enter the UID of the 2st card (empty for none): ")
    data[30] = "static const char *UID_2 = \"" + \
        uid.strip() + "\"; //2nd UID Card Code\n"


def generateApiKey(data):
    api_key = secrets.token_urlsafe(32)
    data[31] = "static const char *API_KEY = \"" + api_key + "\"; //API KEY\n"
    print("Your api key is: " + api_key)
    print("\nWarning: Keep it with care otherwise you will not be able to interact with the APIs.")
    input("Click ENTER to continue...")


def setCertificates(data):
    certificate_path = input("Insert certificate path: ")
    with open(certificate_path, 'r') as file:
        certificate = file.read()
        file.close()
    key_path = input("Insert private key path: ")
    with open(key_path, 'r') as file:
        key = file.read()
        file.close()
    index = 0
    while(not "void setup()" in data[index]):
        if "static const char server" in data[index]:
            while not ")EOF\";" in data[index]:
                del data[index]
        index = index + 1

    data[68] = "static const char serverCert[] PROGMEM = R\"EOF(\n" + \
        certificate + ")EOF\";\n"
    data[70] = "static const char serverKey[] PROGMEM = R\"EOF(\n" + \
        key + ")EOF\";\n"


def setTimers(data):
    print("For each timer, if left empty, the default value will be set.\nEach input is to be entered in seconds.")
    alarmTime = input("Insert alarm time (180): ")
    try:
        alarmTime = int(alarmTime)
    except ValueError:
        alarmTime = 180
    doorEnter = input("Insert door enter time (10): ")
    try:
        doorEnter = int(doorEnter)
    except ValueError:
        doorEnter = 10
    doorExit = input("Insert door exit time (10): ")
    try:
        doorExit = int(doorExit)
    except ValueError:
        doorExit = 10
    tamperTime = input("Insert tamper time (180): ")
    try:
        tamperTime = int(tamperTime)
    except ValueError:
        tamperTime = 180
    data[46] = "static const long alarmTimeout = " + \
        str(alarmTime * 1000) + ";\n"
    data[52] = "static const long doorEnterTimeout = " + \
        str(doorEnter * 1000) + ";\n"
    data[54] = "static const long doorExitTimeout = " + \
        str(doorExit * 1000) + ";\n"
    data[59] = "static const long tamperTimeout = " + \
        str(tamperTime * 1000) + ";\n"


def setStatusURL(data):
    url = input(
        "Enter the url to communicate with IFTTT in order to send notifications of the alarm status via telegram (without https): ")
    data[32] = "static const char *IFTTT_STATUS_URL = \"" + \
        url.strip() + "\"; //IFTTT Webhook URL for send status via Telegram\n"


def setAlarmURL(data):
    url = input(
        "Enter the url to communicate with IFTTT in order to send the alarm alert via telegram (without https): ")
    data[33] = "static const char *IFTTT_ALARM_URL = \"" + \
        url.strip() + "\"; //IFTTT Webhook URL for send alarm alert via Telegram\n"


def setDomainName(data):
    url = input("Enter the domain name of the NodeMCU card: ")
    data[34] = "static const char *dname = \"" + \
        url.strip() + "\"; //Board domain address\n"


if __name__ == "__main__":
    print("Opening the file...")
    data = openFile()
    choice = -1
    while not -1 < choice < 10:
        clearConsole()
        print("NodeMCU Alarm - Script for set firmware variables")
        print("Select a choice: \n  1 - Set all\n  2 - Set Wi-Fi\n  3 - Set card UID\n  4 - Set alarm timers\n  5 - Generate API KEY\n  6 - Set up the certificate\n  7 - Set IFTT Url for alarm status\n  8 - Set IFTT Url for alarm alert\n  9 - Set the domain name of the board\n  0 - Save and Exit")
        choice = input("Choice: ")
        try:
            choice = int(choice)
            clearConsole()
        except ValueError:
            choice = -1
        if choice == 1:
            setWiFi(data)
            setUID(data)
            setDomainName(data)
            setTimers(data)
            generateApiKey(data)
            setCertificates(data)
            setStatusURL(data)
            setAlarmURL(data)
        elif choice == 2:
            setWiFi(data)
        elif choice == 3:
            setUID(data)
        elif choice == 4:
            setTimers(data)
        elif choice == 5:
            generateApiKey(data)
        elif choice == 6:
            setCertificates(data)
        elif choice == 7:
            setStatusURL(data)
        elif choice == 8:
            setAlarmURL(data)
        elif choice == 9:
            setDomainName(data)
        if not choice == 0:
            choice = -1

    clearConsole()
    print("Saving the file...")
    saveFile(data)
