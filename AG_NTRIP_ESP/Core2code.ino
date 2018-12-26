//Core2: this task serves the Webpage and handles the GPS NMEAs

void Core2code( void * pvParameters ){
  Serial.println();
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  
// Start WiFi Client
 while (!EE_done){  // wait for eeprom data
  delay(10);
 }
 WiFi_Start_STA();
 if (my_WiFi_Mode == 0) WiFi_Start_AP(); // if failed start AP

 repeat_ser = millis();
 
  for(;;){
    WiFi_Traffic();
    Serial_Traffic();
    delay(10);  
   }
}

 //------------------------------------------------------------------------------------------
//Read serial GPS data
//-------------------------------------------------------------------------------------------
void Serial_Traffic(){

while (Serial1.available()) 
  {  
   byte c = Serial1.read();
   if (c == 0x24)      //if character is '$' new sentence
      {
        newSentence = true; // only store sentence, if time is over
        i = 0;
      }
    
   if (c == 0x0D && newSentence)   //if character is CR, build UDP send buffer
      {
        char Sent_Buffer[]="???";
        gpsBuffer[i++] = 0x0D;  //last-1 byte =CR
        gpsBuffer[i++] = 0x0A;  //last-1 byte =CR
        Sent_Buffer[0] = (char)gpsBuffer[3];
        Sent_Buffer[1] = (char)gpsBuffer[4];
        Sent_Buffer[2] = (char)gpsBuffer[5];

        if (strcmp(Sent_Buffer, "RMC")==0 || strcmp(Sent_Buffer, "GGA")==0 || strcmp(Sent_Buffer, "VTG")==0){
            if (NtripSettings.send_UDP_AOG  == 1) {
              udp.writeTo(gpsBuffer, i, ipDestination, portDestination );    
            }
            
              
            if (NtripSettings.sendGGAsentence == 2 ){
               if (strcmp(Sent_Buffer, "GGA") == 0){
                  for (byte n = 0; n <= i; n++){
                    lastSentence[n] = gpsBuffer[n];
                   }
                  repeat_ser = millis(); //Reset timer
                }
              } 
          }
            i = 0;
            newSentence = false;
     }
   
   if (newSentence && i < 100) 
     {
       gpsBuffer[i++] = c;
     }
  }
}  
