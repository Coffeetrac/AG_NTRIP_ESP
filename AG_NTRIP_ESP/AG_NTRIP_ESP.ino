TaskHandle_t Core1;
TaskHandle_t Core2;
// ESP32 Ntrip Client by Coffeetrac
// 10.12.2018 W.Eder
// 
//##########################################################################################################
//### Setup Zone ###########################################################################################
//### Just Default values ##################################################################################
struct Storage{
  
  char ssid[24]        = "yourSSID";          // WiFi network Client name
  char password[24]    = "YourPassword";      // WiFi network password

  // Ntrip Caster Data
  
  char host[40]        = "195.200.70.200";    // Server IP
  int  port            = 2101;                // Server Port
  char mountpoint[40]  = "FPS_BY_RTCM3_2G";   // Mountpoint
  char ntripUser[40]   = "NTRIPUsername";     // Username
  char ntripPassword[40]= "NTRIPPassword";    // Password

  byte sendGGAsentence = 1; // 0 = No Sentence will be sended
                            // 1 = fixed Sentence from GGAsentence below will be sended
                            // 2 = GGA from GPS will be sended
  
  byte GGAfreq =10;         // time in seconds between GGA Packets
  char GGAsentence[100] = "$GPGGA,051353.171,4751.637,N,01224.003,E,1,12,1.0,0.0,M,0.0,M,,*6B"; //hc create via www.nmeagen.org
  
  long baudOut = 19200;     // Baudrate of RTCM Port

  byte send_UDP_AOG  = 1;   // 1 = Transmission of NMEA Sentences to AOG via Ethernet-UDP
  byte enableNtrip   = 1;   // 1 = NTRIP Client enabled
}; Storage NtripSettings;
//##########################################################################################################
//### End of Setup Zone ####################################################################################
//##########################################################################################################

// IO pins --------------------------------
#define RX0      3
#define TX0      1

#define RX1     26 //34  //26
#define TX1     12 //32  //25

#define RX2     17
#define TX2     16

#define restoreDefault_PIN 36  // set to 1 during boot, to restore the default values

#include <WiFi.h>
#include <base64.h>


//libraries -------------------------------
#include "Network_AOG.h"
#include "EEPROM.h"

//Accesspoint name and password:
const char* ssid_ap     = "NTRIP_Client_ESP_Net";
const char* password_ap = "passport";


//static IP
IPAddress myip(192, 168, 1, 79);  // Roofcontrol module
IPAddress gwip(192, 168, 1, 1);  // Gateway & Accesspoint IP
IPAddress mask(255, 255, 255, 0);
IPAddress myDNS(8, 8, 8, 8);      //optional

unsigned int portMy = 5577; //this is port of this module: Autosteer = 5577
unsigned int portAOG = 8888; // port to listen for AOG

//IP address to send UDP data to:
IPAddress ipDestination(192, 168, 1, 255);
unsigned int portDestination = 9999; // Port of AOG that listens

// Variables ------------------------------
// program flow
bool AP_running=0, EE_done = 0, showST=0;
int value = 0; 
unsigned long repeat_ser;   
int error = 0;
unsigned long repeatGGA;

// GPS-Bridge
int i,j,k,l;
byte gpsBuffer[100];
char imuBuffer[20];
bool newSentence = false;
bool newIMUSentence = false;
char lastSentence[100]="";

//Array to send data back to AgOpenGPS
byte toSend[100]; 
  
// Instances ------------------------------
WiFiServer server(80);
WiFiClient client_page;
WiFiClient client_ntrip;
AsyncUDP udp;

// Setup procedure ------------------------
void setup() {
  Serial.begin(115200); 
  //Serial.begin(115200,SERIAL_8N1,RX0,TX0);
  Serial1.begin (NtripSettings.baudOut,SERIAL_8N1,RX1,TX1);  
  //Serial2.begin(19200,SERIAL_8N1,RX2,TX2); 

  pinMode(restoreDefault_PIN, INPUT);  //
  //------------------------------------------------------------------------------------------------------------  
  //create a task that will be executed in the Core1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(Core1code, "Core1", 10000, NULL, 1, &Core1, 0);
  delay(500); 
  //create a task that will be executed in the Core2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(Core2code, "Core2", 10000, NULL, 1, &Core2, 1); 
  delay(500); 
  //------------------------------------------------------------------------------------------------------------
  byte get_state  = digitalRead(restoreDefault_PIN);
  if (get_state) Serial.print("State: restoring default values ! ");
  if (!get_state) Serial.print("State: read default values from EEPROM ");
  
  if (EEprom_empty_check()==1 || get_state) { //first start?
    EEprom_write_all();     //write default data
   }
  if (EEprom_empty_check()==2) { //data available
    EEprom_read_all();
   }
  //EEprom_show_memory();  //
  EE_done =1;   
}

void loop() {
  
}
