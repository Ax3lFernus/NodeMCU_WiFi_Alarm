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
# Version v1.0 29-03-2020
#
import string
import secrets

def generateApiKey(length=10):
    return secrets.token_urlsafe(length)

def main():
    with open('firmware.ino', 'r') as file:
        # read a list of lines into data
        data = file.readlines()
        file.close()
    ssid = input("Insert your Wi-Fi SSID: ")
    pw = input("Insert your Wi-Fi password: ")
    uid = input("Insert your UID card: ")
    api_key = generateApiKey(32)

    data[26] = "const char *ssid = \"" + ssid.strip() + "\"; //Network SSID\n"
    data[27] = "const char *password = \"" + pw.strip() + "\"; //Network PASSWORD\n"
    data[28] = "const char *UID = \"" + uid.strip() + "\"; //UID Card Code\n"
    data[29] = "const char *API_KEY = \"" + api_key + "\"; //API KEY\n"

    with open('firmware.ino', 'w') as file:
        file.writelines( data )
        file.close()
    print("Your api key is: " + api_key)
    print("Warning: Keep it with care otherwise you will not be able to interact with the APIs.")

if __name__ == "__main__":
    main()