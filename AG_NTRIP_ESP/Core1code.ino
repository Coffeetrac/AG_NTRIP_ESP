//Core1:  NTRIP Client Code

void Core1code( void * pvParameters ){
 Serial.println();
 Serial.print("Task1 running on core ");
 Serial.println(xPortGetCoreID());


 while (!EE_done){  // wait for eeprom data
  delay(10);
 }

 setAuthorization(NtripSettings.ntripUser, NtripSettings.ntripPassword); // Create Auth-Code
 
 while (my_WiFi_Mode != 1){
   Serial.println("Waiting for WiFi Access");
   delay(5000);
  }
 
 delay(100);
 if (NtripSettings.enableNtrip==1 ) getSourcetable(); // print the Sourcetable to serial0 
 else Serial.println("Ntrip Client is switched OFF ");

for(;;){ // MAIN LOOP FOR THIS CORE
  
  //if (WiFi.status() != WL_CONNECTED) connectWiFi(); // reconnect if connection is lost
  // delay(5000);
 
 if (NtripSettings.enableNtrip==1){   
    if (NtripSettings.enableNtrip == 1 && showST == 0){
      getSourcetable(); // print the Sourcetable to serial0 
     }
    Serial.print("connecting to ");
    Serial.println(NtripSettings.host);
    
    while (!client_ntrip.connect(NtripSettings.host, NtripSettings.port)) {
       Serial.println("NTRIP Host connection failed");
       Serial.println("Can not connect to NTRIP Hoster");
       Serial.println("Check Network Name and Port");
       delay(2000);
     }

  // This will send the request to the server
  String requestMtp ="";
  requestMtp =(String("GET /") + NtripSettings.mountpoint + " HTTP/1.0\r\n" +
                 "User-Agent: " + _userAgent + "\r\n" +
                 "Accept: " + _accept + "\r\n" +
                 "Connection: close\r\n" +
                 "Authorization: Basic " + 
                 _base64Authorization + "\r\n"+ "\r\n"); 
  client_ntrip.print(requestMtp);
  Serial.print(requestMtp);
   
  unsigned long timeout = millis();
  while (client_ntrip.available() == 0) {
     if (millis() - timeout > 5000) {
         Serial.println(">>> Client Timeout - no response from host");
         client_ntrip.stop();
         delay(2000);
        }
    }
  String currentLine = "";
  
  
  while(client_ntrip.available()) {  
         char a = client_ntrip.read();
         currentLine += a;
         if(a == '\n') break; /// read first line only
   }
   
  if (currentLine.startsWith("ICY 200 OK")){
     Serial.print("Got OK: ");
     Serial.print(currentLine);
     //delay(100); // send an ACK
     Serial.println("try starting RTCM stream at serial1 (* = RTCM-Package,  G = GGA-sent");
     if (NtripSettings.sendGGAsentence){
        //Serial.println(" Sending GGA :");
        sendGGA(); //
       }
     

     repeatGGA = millis();
     while (client_ntrip.connected() && NtripSettings.enableNtrip == 1){
        getRtcmData();
        delay(1);  // helps the watchdog not to kill this loop
      }
    
     Serial.println();
     Serial.println("closing connection");
     if(NtripSettings.enableNtrip==0)Serial.println("Ntrip Client is now switched OFF "); 
   }
  else {
     Serial.print("Received: "); 
     Serial.print(currentLine);
     Serial.print("Error :  "); 
     
     if(currentLine.startsWith("HTTP/1.")) {
       error = currentLine.substring(9, currentLine.indexOf(' ', 9)).toInt();
     }
     Serial.println(errorToString(error));
     delay(3000);
    }
 }// end of enabled Ntrip
 else {
   delay(100);  // Nothing to do if NTRIP is disabled
   showST=0;
  }
} // End of (main core1)
} // End of core1code

//###########################################################################################
void getSourcetable(){
    // Use WiFiClient class to create TCP connections
    WiFiClient client_src;
    //const int httpPort = NtripSettings.port;
    if (!client_src.connect(NtripSettings.host, NtripSettings.port)) {
        Serial.println("connection failed, check if your WiFi has access to the Internet");
        return;
    }
    
    Serial.println("Requesting Sourcetable: ");
    Serial.println();
    
    // This will send the request to the server
    client_src.print(String("GET /") + " HTTP/1.0\r\n" +
                    "User-Agent: " + _userAgent + "\r\n" +
                    "Accept: " + _accept + "\r\n" +
                    "Connection: close\r\n\r\n");
    
    unsigned long timeout = millis();
    while (client_src.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout while requesting Sourcetable !!");
            Serial.println("Check Caster Name and Port !!");
            client_src.stop();
            return;
        }
       delay(1); //wdt 
    }

    // Read all the lines of the reply from server and print them to Serial
  for(int as=0; as < 2; as++){
   while(client_src.available()) {
        String Sourcetable = client_src.readStringUntil('\n');
        Serial.println(Sourcetable);
        delay(1);
     }
   delay(100);
  }
    Serial.println("------------------------------------------------------------------------------------------------");
    //Serial.println();
    Serial.println("closing connection");
    Serial.println();
    showST = 1;
}

//###########################################################################################
void getRtcmData(){
    long timeout = millis();
    while (client_ntrip.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout no RTCM Respond from Caster");
            if(sizeof(lastSentence) < 50 && (NtripSettings.sendGGAsentence >1)) Serial.println("Invalid NMEA String from GPS");
            if(NtripSettings.sendGGAsentence == 1) Serial.println("Invalid fixed NMEA String");
            if(NtripSettings.sendGGAsentence == 0) Serial.println("Check if your Provider requires your Position");
            client_ntrip.stop();
            return;
        }
      delay(1); // WDT prevent
     }
   
   // Read all the bytes of the reply from server and print them to Serial
   String currentLine = "";
   
   while(client_ntrip.available()&& NtripSettings.enableNtrip == 1) {
        char a = client_ntrip.read();
        currentLine += a;            
        Serial1.print(a);
        if (NtripSettings.sendGGAsentence > 0){
         if (millis() - repeatGGA > (NtripSettings.GGAfreq * 1000)) {
            sendGGA(); //
            repeatGGA = millis();
           }
       }  
        delay(1); //wdt
    }
    
    Serial.print("*"); // sign of life
}
//###########################################################################################
void sendGGA(){
    Serial.print("G");
    // Serial.print("Sending GGA: ");
    // This will send the Position to the server, required by VRS - Virtual Reference Station - Systems
    if (NtripSettings.sendGGAsentence == 1){
       client_ntrip.print(NtripSettings.GGAsentence);
       client_ntrip.print("\r\n");
      }
    if (NtripSettings.sendGGAsentence > 1){
       client_ntrip.print(lastSentence);
       //Serial.println();
       Serial1.print(lastSentence); // 
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
//###########################################################################################
String errorToString(int error)
{
    switch(error) {
    case 100: //HTTP_CODE_CONTINUE = 100
        return F("continue");
    case 200: //HTTP_CODE_OK = 200
        return F("OK");
    case 201: //HTTP_CODE_CREATED = 201
        return F("created");
    case 202: //HTTP_CODE_ACCEPTED = 202
        return F("accepted");
    case 203: //HTTP_CODE_NON_AUTHORITATIVE_INFORMATION
        return F("nonauthorinfo");
    case 204: //HTTP_CODE_NO_CONTENT
        return F("no content");
    case 400: //HTTP_CODE_BAD_REQUEST = 400
        return F("bad request");
    case 401: //HTTP_CODE_UNAUTHORIZED = 401
        return F("unauthorized - Check name+pw");
    case 402:
        return F("payment required");
    case 403:
        return F("forbidden");
    case 404:
        return F("not found");
    default:
        return String();
    }
}

//############################################################################################
/// HTTP codes see RFC7231
/*
    HTTP_CODE_CONTINUE = 100,
    HTTP_CODE_SWITCHING_PROTOCOLS = 101,
    HTTP_CODE_PROCESSING = 102,
    HTTP_CODE_OK = 200,
    HTTP_CODE_CREATED = 201,
    HTTP_CODE_ACCEPTED = 202,
    HTTP_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_CODE_NO_CONTENT = 204,
    HTTP_CODE_RESET_CONTENT = 205,
    HTTP_CODE_PARTIAL_CONTENT = 206,
    HTTP_CODE_MULTI_STATUS = 207,
    HTTP_CODE_ALREADY_REPORTED = 208,
    HTTP_CODE_IM_USED = 226,
    HTTP_CODE_MULTIPLE_CHOICES = 300,
    HTTP_CODE_MOVED_PERMANENTLY = 301,
    HTTP_CODE_FOUND = 302,
    HTTP_CODE_SEE_OTHER = 303,
    HTTP_CODE_NOT_MODIFIED = 304,
    HTTP_CODE_USE_PROXY = 305,
    HTTP_CODE_TEMPORARY_REDIRECT = 307,
    HTTP_CODE_PERMANENT_REDIRECT = 308,
    HTTP_CODE_BAD_REQUEST = 400,
    HTTP_CODE_UNAUTHORIZED = 401,
    HTTP_CODE_PAYMENT_REQUIRED = 402,
    HTTP_CODE_FORBIDDEN = 403,
    HTTP_CODE_NOT_FOUND = 404,
    HTTP_CODE_METHOD_NOT_ALLOWED = 405,
    HTTP_CODE_NOT_ACCEPTABLE = 406,
    HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_CODE_REQUEST_TIMEOUT = 408,
    HTTP_CODE_CONFLICT = 409,
    HTTP_CODE_GONE = 410,
    HTTP_CODE_LENGTH_REQUIRED = 411,
    HTTP_CODE_PRECONDITION_FAILED = 412,
    HTTP_CODE_PAYLOAD_TOO_LARGE = 413,
    HTTP_CODE_URI_TOO_LONG = 414,
    HTTP_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_CODE_RANGE_NOT_SATISFIABLE = 416,
    HTTP_CODE_EXPECTATION_FAILED = 417,
    HTTP_CODE_MISDIRECTED_REQUEST = 421,
    HTTP_CODE_UNPROCESSABLE_ENTITY = 422,
    HTTP_CODE_LOCKED = 423,
    HTTP_CODE_FAILED_DEPENDENCY = 424,
    HTTP_CODE_UPGRADE_REQUIRED = 426,
    HTTP_CODE_PRECONDITION_REQUIRED = 428,
    HTTP_CODE_TOO_MANY_REQUESTS = 429,
    HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_CODE_INTERNAL_SERVER_ERROR = 500,
    HTTP_CODE_NOT_IMPLEMENTED = 501,
    HTTP_CODE_BAD_GATEWAY = 502,
    HTTP_CODE_SERVICE_UNAVAILABLE = 503,
    HTTP_CODE_GATEWAY_TIMEOUT = 504,
    HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_CODE_VARIANT_ALSO_NEGOTIATES = 506,
    HTTP_CODE_INSUFFICIENT_STORAGE = 507,
    HTTP_CODE_LOOP_DETECTED = 508,
    HTTP_CODE_NOT_EXTENDED = 510,
    HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511
*/
