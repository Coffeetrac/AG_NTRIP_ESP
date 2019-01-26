//Core2: this task serves the Webpage and handles the GPS NMEAs

void Core2code( void * pvParameters ){
  
  DBG("\nTask2 running on core ");
  DBG((int)xPortGetCoreID(), 1);

  
// Start WiFi Client
 while (!EE_done){  // wait for eeprom data
  delay(10);
 }
 WiFi_Start_STA();
 if (my_WiFi_Mode == 0) WiFi_Start_AP(); // if failed start AP

 repeat_ser = millis();
  if ((NtripSettings.AHRSbyte == 1)|(NtripSettings.AHRSbyte == 3)) {   // Initialize the BNO055 if not done
	 if (imu_initialized == 0) {
		 initBNO055();
		 imu_initialized = 1;
	 }
	 else {		//  no IMU
		 imu_initialized = 0;
		 Head = 0;
		 Yaw = 0;
	 }
  }
  udp.listen(portMy);
  for(;;){ //main loop core2
		WiFi_Traffic();
		Serial_Traffic();

		//* Loop triggers every 100 msec and sends back gyro heading, and roll
		currentTime = millis();
		unsigned int time = currentTime;
  
		if (currentTime - lastTime >= LOOP_TIME)
		{
			dT = currentTime - lastTime;
			lastTime = currentTime;

			//BNO
			if (imu_initialized == 1) {
				readEulData(EulCount);  // Read the x/y/z adc values   
				// Calculate the Euler angles values in degrees
				Head = (float)EulCount[0];
				if (debugmode) DBG(EulCount[0]);
				Yaw = Head / 16.;
			}

			//MMA
			if ((NtripSettings.AHRSbyte == 2)|(NtripSettings.AHRSbyte == 3)) {
				// MMA8452 (1) Inclinometer
				if (accelerometer.acc_initialized == 0) {
					if (!accelerometer.init()) NtripSettings.AHRSbyte -= 2;
				} // Try to Initialize MMA8452
				accelerometer.getRawData(&x_, &y_, &z_);
				roll = x_; //Conversion uint to int
				if (roll > 8500)  roll = 8500;
				if (roll < -8500) roll = -8500;
				if (debugmode) DBG(roll);
				roll -= roll_corr;  // 
				rollK = map(roll, -8500, 8500, -480, 480); //16 counts per degree (good for 0 - +/-30 degrees) 
			}

			//Kalman filter
			Pc = P + varProcess;
			G = Pc / (Pc + varRoll);
			P = (1 - G) * Pc;
			Xp = XeRoll;
			Zp = Xp;
			XeRoll = G * (rollK - Zp) + Xp;


			

			//Build Autosteer Packet: Send to agopenGPS **** you must send 10 Byte or 5 Int

			int temp;
			//actual steer angle
			//temp = (100 * steerAngleActual);
			//IMUtoSend[2] = 5;
			//IMUtoSend[3] = (byte)(temp);

			//imu heading --- * 16 in degrees
			temp = Head;
			IMUtoSend[4] = (byte)(temp >> 8);
			IMUtoSend[5] = (byte)(temp);

			//Vehicle roll --- * 16 in degrees
			temp = XeRoll;
			IMUtoSend[6] = (byte)(temp >> 8);
			IMUtoSend[7] = (byte)(temp);

			//switch byte
			//IMUtoSend[8] = switchByte;

			//Build Autosteer Packet completed
			//Send_UDP();  //transmit to AOG
			udp.writeTo(IMUtoSend, IMUtoSendLenght, ipDestination, portDestination);


		}//end of timed loop getting and sending IMU data

    //delay(1);  
   }//end main loop core 2
}

//------------------------------------------------------------------------------------------
//Read serial GPS data
//-------------------------------------------------------------------------------------------
void Serial_Traffic(){

while (Serial1.available()) 
  { 
   c = Serial1.read();
   //Serial2.print(c);
   if (c == '$')      //if character is $=0x24 start new sentence
      {
        newSentence = true; // only store sentence, if time is over
        gpsBuffer[0] = '/0';
        i = 0;
      }
    
   if (c == 0x0D && newSentence)   //if character is CR, build UDP send buffer
      {
        char Sent_Buffer[]="???";
        gpsBuffer[i++] = 0x0D;  //last-1 byte =CR
        gpsBuffer[i++] = 0x0A;  //last-1 byte =CR
        //gpsBuffer[i++] = "/0";  //
        Sent_Buffer[0] = (char)gpsBuffer[3];
        Sent_Buffer[1] = (char)gpsBuffer[4];
        Sent_Buffer[2] = (char)gpsBuffer[5];

        
        if (strcmp(Sent_Buffer, "RMC")==0 || strcmp(Sent_Buffer, "GGA")==0 || strcmp(Sent_Buffer, "VTG")==0 || strcmp(Sent_Buffer, "ZDA")==0){
            switch (NtripSettings.send_UDP_AOG){
              case 1:
                 udp.writeTo(gpsBuffer, i, ipDestination, portDestination );    
               break;
              case 2:
                 for (byte n = 0; n < i; n++){  //print gpsBuffer to Bluetooth
                    SerialBT.print((char)gpsBuffer[n]);
                  }
               break;
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
	   if (debugmode) DBG(gpsBuffer[i]);
     }
  }

}  
