//Core1:  NTRIP Client Code

void Core1code( void * pvParameters ){

  DBG("\nTask1 running on core ");
  DBG((int)xPortGetCoreID(),1);


  while (!EE_done){  // wait for eeprom data
     delay(10);
   }

  while (my_WiFi_Mode != WIFI_STA){
     DBG("Waiting for WiFi Access\n");
     delay(5000);
   }

  lifesign  = millis();  //init timers 
  repeatGGA = millis();  // 

  DBG("\nRTCM/NMEA Baudrate: ");
  DBG(NtripSettings.baudOut, 1);

for(;;){ // MAIN LOOP FOR THIS CORE
  
   
  if (NtripSettings.enableNtrip==1 && my_WiFi_Mode == WIFI_STA){
     if (restart == 0){
        DBG("\nRequesting Sourcetable:\n\n");
        if(!getSourcetable()) DBG("SourceTable request error !!\n");
        DBG("try starting RTCM stream !!!!!\n");
        if (!startStream()) DBG("Stream request error\n");
        DBG("RTCM stream started at serial1 (* = RTCM-Package,  G = GGA-sent)\n");
        restart = 1;
      }     
     if (!getRtcmData()){
        //DBG("\nstopped receiving data \n");
        DBG("\nCan not reach hoster, internet connection broken\n");
        delay(5000);
        DBG("\nTrying to reconnect\n"); 
        restart=0;
     }  
     
   }
 else {
     DBG("Ntrip Client is switched OFF\n");
     if (my_WiFi_Mode != WIFI_STA) DBG("No WiFi connection\n");
     delay(6000);
   }

 if (WiFi.status() != WL_CONNECTED) {
    my_WiFi_Mode = 0;
    DBG("WiFi offline, trying to reconnect\n");
    WiFi_Start_STA();
    if (my_WiFi_Mode == WIFI_STA) restart = 0;
  }

} // End of (main core1)
} // End of core1code

//###########################################################################################

//###########################################################################################
bool getSourcetable(){
  
   if (!connectCaster()){
     DBG("NTRIP Host connection failed\n");
     DBG("Can not connect to NTRIP Hoster\n");
     DBG("Check Network Name and Port\n");
     delay(2000);
     return false;
   }    
    // This will send the request to the server
     ntripCl.print(String("GET /") + " HTTP/1.0\r\n" +
                    "User-Agent: " + _userAgent + "\r\n" +
                    "Accept: " + _accept + "\r\n" +
                    "Connection: close\r\n\r\n");
    
    unsigned long timeout = millis();
    while (!ntripCl.available()) {
        if (millis() - timeout > 5000) {
            DBG(">>> Client Timeout while requesting Sourcetable !!\n");
            DBG("Check Caster Name and Port !!\n");
            ntripCl.stop();
            return false;
        }
       delay(1); //wdt 
    }
   
   String currentLine = readLine(); //reads to strmBuf
   if (currentLine.startsWith("SOURCETABLE 200 OK")){
     DBG(currentLine, 1);
     delay(5);
     for(int as=0; as < 2; as++){
       while(ntripCl.available()) {
         currentLine = ntripCl.readStringUntil('\n');
         DBG(currentLine, 1);
         delay(1);
       }
       delay(100); // wait for additional Data
     }
     DBG("---------------------------------------------------------------------------------------------\n");
     ntripCl.stop();
     return true;
   }
   else{
     DBG(currentLine, 1);
     return false;
   }  
}

//###########################################################################################
bool startStream(){
 
 // Reconnect for getting RTCM Stream
 if (!connectCaster()){  //reconnect for stream
   DBG("NTRIP Host connection failed\n");
   DBG("Can not connect to NTRIP Hoster\n");
   DBG("Check Network Name and Port\n");
   delay(2000);
   return false;
 }
 
 // This will send the request to the server
 String requestMtp =(String("GET /") + NtripSettings.mountpoint + " HTTP/1.0\r\n" +
                      "User-Agent: " + _userAgent + "\r\n");
 if (strlen(NtripSettings.ntripUser)==0){
   requestMtp += (String("Accept: ") + _accept + "\r\n");
   requestMtp += String("Connection: close\r\n"); 
  }                    
 else{
   requestMtp += String("Authorization: Basic "); 
   requestMtp += _base64Authorization;
   requestMtp += String("\r\n");
 }
 requestMtp += String("\r\n");
 
 ntripCl.print(requestMtp);
 // DBG(requestMtp);
 
 if (NtripSettings.sendGGAsentence > 0) {
    sendGGA(); // 
    repeatGGA = millis();
    DBG("", 1); //NL
  }
 
 delay(10);
 
 unsigned long timeout = millis();
 while (!ntripCl.available()) {
    if (millis() - timeout > 15000) {
         DBG("\n>>> Client Timeout - no response from host\n");
         ntripCl.stop();
         delay(2000);
         return false;
        }
     delay(1); //wdt
    } 
 delay(5);
 String currentLine;
 if (!(currentLine = readLine())) return false; //read answer 
 if (!(currentLine.startsWith("ICY 200 OK"))) {
  DBG("Received Error: ");
  DBG(currentLine, 1);
  return false;
 }
 //DBG("Connection established\n");
 cnt=0; // reset counter
 return true;
}

//###########################################################################################
bool getRtcmData(){
    long timeout = millis();
    while (ntripCl.available() == 0) {
        if (millis() - timeout > 5000) {
            DBG("\n>>> Client Timeout no RTCM Respond from Caster\n");
            if(sizeof(lastSentence) < 50 && (NtripSettings.sendGGAsentence >1)) DBG("Invalid NMEA String from GPS\n");
            if(NtripSettings.sendGGAsentence == 1) DBG("Maybe invalid fixed NMEA String\n");
            if(NtripSettings.sendGGAsentence == 0) DBG("Check if your Provider requires your Position\n");
            return false;
        }
      delay(1); // WDT prevent
     }
   
   // Read all the bytes of the reply from server and print them to Serial
   while(ntripCl.available()&& NtripSettings.enableNtrip == 1) {
      char a = ntripCl.read();
      Serial1.print(a);
      if (NtripSettings.sendGGAsentence > 0){
        if (millis() - repeatGGA > (NtripSettings.GGAfreq * 1000)) {
          sendGGA(); //
          repeatGGA = millis();
        }
      }
      if (millis() - lifesign > 1000) {
        DBG("*"); // Sectic - Data receiving
        if(cnt++ >=59) {
          DBG("", 1); //NL
          cnt=0;
        }
        lifesign = millis();
      }      
      delay(1);   
   }
  return true;
}
//###########################################################################################
bool connectCaster(){

  setAuthorization(NtripSettings.ntripUser, NtripSettings.ntripPassword); // Create Auth-Code
 
  return ntripCl.connect(NtripSettings.host, NtripSettings.port);
}
//###########################################################################################
char* readLine(){
  int i=0;
  // Read a line of the reply from server and print them to Serial
  //Serial.println("start Line read: ");  
  while(ntripCl.available()) {
    strmBuf[i] = ntripCl.read();
    //Serial.print(strmBuf[i]);
    if( strmBuf[i] == '\n' || i>=511) break;
    i++;            
  }
  strmBuf[i]= '\0';  //terminate string
  return (char*)strmBuf;
  
}
//###########################################################################################
void sendGGA(){
    DBG("G");
    // DBG("Sending GGA: ");
    // This will send the Position to the server, required by VRS - Virtual Reference Station - Systems
    if (NtripSettings.sendGGAsentence == 1){
       ntripCl.print(NtripSettings.GGAsentence);
       ntripCl.print("\r\n");
      }
    if (NtripSettings.sendGGAsentence > 1){
       ntripCl.print(lastSentence);
       //DBG(lastSentence, 1); // 
      }
}
       
//###########################################################################################
void setAuthorization(const char * user, const char * password)
{
    if(user && password) {
        String auth = user;
        auth += ":";
        auth += password;
        _base64Authorization = base64::encode(auth);
    }
}
