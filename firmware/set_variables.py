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
    data[26] = "const char *ssid = \"" + ssid.strip() + "\"; //Network SSID\n"
    data[27] = "const char *password = \"" + \
        pw.strip() + "\"; //Network PASSWORD\n"

def setUID(data):
    uid = input("Insert your UID card: ")
    data[28] = "const char *UID = \"" + uid.strip() + "\"; //UID Card Code\n"

def generateApiKey(data):
    api_key = secrets.token_urlsafe(32)
    data[29] = "const char *API_KEY = \"" + api_key + "\"; //API KEY\n"
    print("Your api key is: " + api_key)
    print("Warning: Keep it with care otherwise you will not be able to interact with the APIs.")
    input("Click ENTER to continue...")

if __name__ == "__main__":
    print("Opening file...")
    data = openFile()
    choice = -1
    while not -1 < choice < 5:
        clearConsole()
        print("NodeMCU Alarm - Script for set firmware variables")
        print("Select a choice: \n  1 - Set all variables\n  2 - Set Wi-Fi\n  3 - Set RFID UID\n  4 - Generate API KEY\n  0 - Save and Exit")
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
        
        if not choice == 0:
            choice = -1

    clearConsole()
    print("Saving file...")
    saveFile(data)