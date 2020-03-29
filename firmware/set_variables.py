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
# Version v1.3.1 29-03-2020
#
import string, secrets
from os import system, name

def clearConsole(): 
    if name == 'nt': 
        _ = system('cls')  
    else: 
        _ = system('clear') 

def openFile():
    with open('firmware.ino', 'r') as file:
        # read a list of lines into data
        data = file.readlines()
        file.close()
    return data

def saveFile(data):
    with open('firmware.ino', 'w') as file:
        file.writelines(data)
        file.close()

def setWiFi(data):
    ssid = input("Insert your Wi-Fi SSID: ")
    pw = input("Insert your Wi-Fi password: ")
    data[30] = "const char *ssid = \"" + ssid.strip() + "\"; //Network SSID\n"
    data[31] = "const char *password = \"" + \
        pw.strip() + "\"; //Network PASSWORD\n"

def setUID(data):
    uid = input("Insert your UID card: ")
    data[32] = "const char *UID = \"" + uid.strip() + "\"; //UID Card Code\n"

def generateApiKey(data):
    api_key = secrets.token_urlsafe(32)
    data[33] = "const char *API_KEY = \"" + api_key + "\"; //API KEY\n"
    print("Your api key is: " + api_key)
    print("Warning: Keep it with care otherwise you will not be able to interact with the APIs.")
    input("Click ENTER to continue...")

def setCertificates(data):
    certificate_path = input("Insert certificate path: ")
    with open(certificate_path, 'r') as file:
        certificate = file.read()
        file.close()
    key_path = input("Insert key path: ")
    with open(key_path, 'r') as file:
        key = file.read()
        file.close()
    index = 0
    while(not "bool connectToWifi()" in data[index]):
        if "static const char server" in data[index]:
            while not ")EOF\";" in data[index]:
                del data[index]
        index = index + 1

    data[76] = "static const char serverCert[] PROGMEM = R\"EOF(\n" + certificate + ")EOF\";\n"
    data[79] = "static const char serverKey[] PROGMEM = R\"EOF(\n" + key + ")EOF\";\n"

if __name__ == "__main__":
    print("Opening file...")
    data = openFile()
    choice = -1
    while not -1 < choice < 6:
        clearConsole()
        print("NodeMCU Alarm - Script for set firmware variables")
        print("Select a choice: \n  1 - Set all variables\n  2 - Set Wi-Fi\n  3 - Set RFID UID\n  4 - Generate API KEY\n  5 - Certificate set\n  0 - Save and Exit")
        choice = input("Choice: ")
        try:
            choice = int(choice)
            clearConsole()
        except ValueError:
            choice = -1
        if choice == 1:
            setWiFi(data)
            setUID(data)
            generateApiKey(data)
        elif choice == 2:
            setWiFi(data)
        elif choice == 3:
            setUID(data)
        elif choice == 4:
            generateApiKey(data)
        elif choice == 5:
            setCertificates(data)
        
        if not choice == 0:
            choice = -1

    clearConsole()
    print("Saving file...")
    saveFile(data)