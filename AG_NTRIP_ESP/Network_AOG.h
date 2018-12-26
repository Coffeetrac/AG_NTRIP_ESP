// all about Network

#include <WiFi.h>
#include <AsyncUDP.h>

#include <WiFiAP.h>
#include <WiFiClient.h>


// Wifi variables & definitions

#define MAX_PACKAGE_SIZE 2048
char HTML_String[6000];
char HTTP_Header[150];

byte my_WiFi_Mode = 0;  // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
//---------------------------------------------------------------------
String _userAgent = "NTRIP CoffeetracNTRIPClient";    
// Allgemeine Variablen
String _base64Authorization;
String _accept = "*/*";

#define ACTION_SET_SSID        1  
#define ACTION_SET_NTRIPCAST   2
#define ACTION_SET_SENDPOS     3
#define ACTION_SET_RESTART     4
#define ACTION_SET_GGA         5
#define ACTION_SET_NMEAOUT     6

int action;

// Radiobutton Select your Position type
char position_type[4][26] = {"Position Off", "Position Fixed via String", "GGA Position from GPS"};

// Radiobutton Select the time between repetitions.
char repeatPos[3][8] = {"1 sec.", "5 sec.", "10 sec."};

// Radiobutton Baudrate of serial1
char baud_output[6][7] = {"  9600", " 14400", " 19200", " 38400", " 57600", "115200"};

// Radiobutton Select if NMEA are transmitted via UDP.
char sendNmea[2][4] = {"OFF", "ON"};

// Radiobutton Select if NTRIP Client is enabled. (Off to use only NMEA Transmission to AOG)
char ntripOn_type[2][4] = {"OFF", "ON"};
//---------------------------------------------------------------------
