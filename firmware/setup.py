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
# Version v1.5 01-04-2020
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


def setApiKey(data):
    api_key = input(
        "Insert API Key (if empty this script will generate it for you - best solution): ")
    if api_key.strip() == "":
        api_key = secrets.token_urlsafe(32)
        print("Your api key is: " + api_key)
        print("\nWarning: Keep it with care otherwise you will not be able to interact with the APIs.")
        input("Click ENTER to continue...")
    data[31] = "static const char *API_KEY = \"" + api_key + "\"; //API KEY\n"


def setStatusURL(data):
    url = input(
        "Enter the url to communicate with IFTTT in order to send notifications of the alarm status via telegram (in https): ")
    data[32] = "static const char *IFTTT_STATUS_URL = \"" + \
        url.strip() + "\"; //IFTTT Webhook URL for send status via Telegram\n"
    setFingerprint(data)


def setFingerprint(data):
    fingerprint = input("Enter the IFTTT Certificate Fingerprint (SHA-256): ")
    if ":" in fingerprint:
        fingerprint = fingerprint.replace(":", " ")
    elif "," in fingerprint:
        fingerprint = fingerprint.replace(",", " ")
    elif not " " in fingerprint.strip():
        fingerprint = ' '.join(
            a+b for a, b in zip(fingerprint[::2], fingerprint[1::2]))
    data[33] = "const char iftttFingerprint[] PROGMEM = \"" + \
        fingerprint.strip().upper() + "\"; //IFTTT certificate fingerprint\n"
    fingerSetted = 1


def setDomainName(data):
    url = input("Enter the domain name of the NodeMCU card: ")
    data[34] = "static const char *dname = \"" + \
        url.strip() + "\"; //Board domain address\n"


def setStaticIP(data):
    ip = input("Enter the IP address (es 192.168.1.5): ")
    if ip.strip() != "":
        gateway = input("Enter the gateway (default 192.168.1.254): ")
        if gateway.strip() == "":
            gateway = "192.168.1.254"
        subnet = input("Enter the subnet mask (default 255.255.255.0): ")
        if subnet.strip() == "":
            subnet = "255.255.255.0"
        dns = input("Enter the dns server (default 8.8.8.8): ")
        if dns.strip() == "":
            dns = "8.8.8.8"
        data[35] = "IPAddress ip(" + ip.strip().replace('.', ',') + \
            "); //Set static IP address. If 0.0.0.0 the board will use DHCP.\n"
        data[36] = "IPAddress gateway(" + gateway.strip().replace(
            '.', ',') + "); //Set the gateway (only if IP was manually set).\n"
        data[37] = "IPAddress subnet(" + subnet.strip().replace('.', ',') + \
            "); //Set the subnet mask (only if IP was manually set).\n"
        data[38] = "IPAddress dns(" + dns.strip().replace('.',
                                                          ',') + "); //Set the DNS IP\n"


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
    data[50] = "static const long alarmTimeout = " + \
        str(alarmTime * 1000) + ";\n"
    data[56] = "static const long doorEnterTimeout = " + \
        str(doorEnter * 1000) + ";\n"
    data[58] = "static const long doorExitTimeout = " + \
        str(doorExit * 1000) + ";\n"
    data[63] = "static const long tamperTimeout = " + \
        str(tamperTime * 1000) + ";\n"


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

    data[72] = "static const char serverCert[] PROGMEM = R\"EOF(\n" + \
        certificate + ")EOF\";\n"
    data[74] = "static const char serverKey[] PROGMEM = R\"EOF(\n" + \
        key + ")EOF\";\n"


if __name__ == "__main__":
    print("Opening the file...")
    data = openFile()
    choice = -1
    while not -1 < choice < 10:
        clearConsole()
        print("NodeMCU Alarm - Script for set firmware variables\n")
        print("Select a choice: ")
        print("--Board variables:\n  1 - Set Wi-Fi\n  2 - Set card UID\n  3 - Set alarm timers\n  4 - Set API KEY\n  5 - Set SSL certificate\n  6 - Set board domain")
        print("--Optional variables:\n  7 - Set IFTT Url for alarm status\n  8 - Set static IP address\n  9 - Set all variables")
        print("0 - Save and Exit")
        choice = input("Choice (0 for Save and Exit): ")
        try:
            choice = int(choice)
            clearConsole()
        except ValueError:
            choice = -1
        if choice == 1:
            setWiFi(data)
        elif choice == 2:
            setUID(data)
        elif choice == 3:
            setTimers(data)
        elif choice == 4:
            setApiKey(data)
        elif choice == 5:
            setCertificates(data)
        elif choice == 6:
            setDomainName(data)
        elif choice == 7:
            setStatusURL(data)
        elif choice == 8:
            setStaticIP(data)
        elif choice == 9:
            setWiFi(data)
            setUID(data)
            setTimers(data)
            setApiKey(data)
            setCertificates(data)
            setDomainName(data)
            setStatusURL(data)
            setStaticIP(data)
        if not choice == 0:
            choice = -1

    clearConsole()
    print("Saving the file...")
    saveFile(data)
