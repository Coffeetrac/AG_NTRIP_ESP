
//---------------------------------------------------------------------
void WiFi_Start_STA() {
  unsigned long timeout;

  WiFi.mode(WIFI_STA);   //  Workstation
  
  if (!WiFi.config(myip, gwip, mask, myDNS)) 
   {
    DBG("STA Failed to configure\n");
   }
  
  WiFi.begin(NtripSettings.ssid, NtripSettings.password);
  timeout = millis() + 12000L;
  while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
    delay(50);
    DBG(".");
  }
  
  DBG("", 1); //NL
  if (WiFi.status() == WL_CONNECTED) 
   {
    server.begin();
    my_WiFi_Mode = WIFI_STA;
    DBG("WiFi Client successfully connected to : ");
    DBG(NtripSettings.ssid, 1);
    DBG("Connected IP - Address : ");
    DBG( WiFi.localIP(), 1);
   } 
  else 
   {
    WiFi.mode(WIFI_OFF);
    DBG("WLAN-Client-Connection failed\n");
   }
  
}

//---------------------------------------------------------------------
void WiFi_Start_AP() {
  WiFi.mode(WIFI_AP);   // Accesspoint
  WiFi.softAP(ssid_ap, password_ap);
  while (!SYSTEM_EVENT_AP_START) // wait until AP has started
   {
    delay(100);
    DBG(".");
   }
   
  WiFi.softAPConfig(myip, myip, mask);  // set fix IP for AP
  IPAddress getmyIP = WiFi.softAPIP();
    
  server.begin();
  my_WiFi_Mode = WIFI_AP;
  DBG("Accesspoint started - Name : ");
  DBG(ssid_ap);
  DBG( " IP address: ");
  DBG(getmyIP, 1);
}
//---------------------------------------------------------------------

//---------------------------------------------------------------------
void Send_UDP()
{
    //Send Packet
    //udp.listen(portMy);
    udp.writeTo(toSend, sizeof(toSend), ipDestination, portDestination );
    //udp.listen(portAOG);
}
//---------------------------------------------------------------------
void WiFi_Traffic() {

  char my_char;
  int htmlPtr = 0;
  int myIdx;
  int myIndex;
  unsigned long my_timeout;

   
  // Check if a client has connected
  client_page = server.available();
  
  if (!client_page)  return;

  DBG("New Client:\n");           // print a message out the serial port
  
  my_timeout = millis() + 250L;
  while (!client_page.available() && (millis() < my_timeout) ) delay(10);
  delay(10);
  if (millis() > my_timeout)  
    {
      DBG("Client connection timeout!\n");
      return;
    }
  //---------------------------------------------------------------------
  //htmlPtr = 0;
  char c;
  if (client_page) {                        // if you get a client,
    //DBG("New Client.\n");                   // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client_page.connected()) {       // loop while the client's connected
      if (client_page.available()) {        // if there's bytes to read from the client,
        char c = client_page.read();        // read a byte, then
        DBG(c);                             // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
           
            make_HTML01();  // create Page array
           //---------------------------------------------------------------------
           // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
           // and a content-type so the client knows what's coming, then a blank line:
           strcpy(HTTP_Header , "HTTP/1.1 200 OK\r\n");
           strcat(HTTP_Header, "Content-Length: ");
           strcati(HTTP_Header, strlen(HTML_String));
           strcat(HTTP_Header, "\r\n");
           strcat(HTTP_Header, "Content-Type: text/html\r\n");
           strcat(HTTP_Header, "Connection: close\r\n");
           strcat(HTTP_Header, "\r\n");

           client_page.print(HTTP_Header);
           delay(20);
           send_HTML();
           
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') 
           { // if you got anything else but a carriage return character,
             currentLine += c;      // add it to the end of the currentLine
             if (currentLine.endsWith("HTTP")) 
               {
                if (currentLine.startsWith("GET ")) 
                 {
                  currentLine.toCharArray(HTML_String,currentLine.length());
                  DBG("", 1); //NL
                  exhibit ("Request : ", HTML_String);
                  process_Request();
                 }  
               }
           }//end else
      } //end client available
    } //end while client.connected
    // close the connection:
    client_page.stop();
    DBG("Pagelength : ");
    DBG((long)strlen(HTML_String));
    DBG("   --> Client Disconnected\n");
 }// end if client 
}
//---------------------------------------------------------------------
// Process given values
//---------------------------------------------------------------------
void process_Request()
{ 
  int myIndex;

  if (Find_Start ("/?", HTML_String) < 0 && Find_Start ("GET / HTTP", HTML_String) < 0 )
    {
      //nothing to process
      return;
    }
  action = Pick_Parameter_Zahl("ACTION=", HTML_String);

  // WiFi access data
  if ( action == ACTION_SET_SSID) {

    myIndex = Find_End("SSID_MY=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<24;i++) NtripSettings.ssid[i]=0x00;
       Pick_Text(NtripSettings.ssid, &HTML_String[myIndex], 24);
       exhibit ("SSID  : ", NtripSettings.ssid);
      }
    myIndex = Find_End("Password_MY=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<24;i++) NtripSettings.password[i]=0x00;
       Pick_Text(NtripSettings.password, &HTML_String[myIndex], 24);
       exhibit ("Password  : ", NtripSettings.password);
       EEprom_write_all();
      }
  }


  if ( action == ACTION_SET_NTRIPCAST) {
    myIndex = Find_End("CASTER=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<40;i++) NtripSettings.host[i]=0x00;
       Pick_Text(NtripSettings.host, &HTML_String[myIndex], 40);
       exhibit ("Caster:   ", NtripSettings.host);
      }
    
    NtripSettings.port = Pick_Parameter_Zahl("CASTERPORT=", HTML_String);
    exhibit ("Port  : ", NtripSettings.port);
    
    myIndex = Find_End("MOUNTPOINT=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<40;i++) NtripSettings.mountpoint[i]=0x00;
       Pick_Text(NtripSettings.mountpoint, &HTML_String[myIndex], 40);
       exhibit ("Caster:   ", NtripSettings.mountpoint);
      }
        
    myIndex = Find_End("CASTERUSER=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<40;i++) NtripSettings.ntripUser[i]=0x00;
       Pick_Text(NtripSettings.ntripUser, &HTML_String[myIndex], 40);
       exhibit ("Username : ", NtripSettings.ntripUser);
      }
    myIndex = Find_End("CASTERPWD=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<40;i++) NtripSettings.ntripPassword[i]=0x00;
       Pick_Text(NtripSettings.ntripPassword, &HTML_String[myIndex], 40);
       exhibit ("Password  : ", NtripSettings.ntripPassword);
       EEprom_write_all();
       connectCaster(); //reconnect Caster + calc _Auth
      }     
   }  

  if ( action == ACTION_SET_SENDPOS) {
    NtripSettings.sendGGAsentence = Pick_Parameter_Zahl("POSITION_TYPE=", HTML_String);
    if (Pick_Parameter_Zahl("REPEATTIME=", HTML_String)==0) NtripSettings.GGAfreq =1;
    if (Pick_Parameter_Zahl("REPEATTIME=", HTML_String)==1) NtripSettings.GGAfreq =5;
    if (Pick_Parameter_Zahl("REPEATTIME=", HTML_String)==2) NtripSettings.GGAfreq =10;
    
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==0) NtripSettings.baudOut = 9600;
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==1) NtripSettings.baudOut = 14400;
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==2) NtripSettings.baudOut = 19200;
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==3) NtripSettings.baudOut = 38400;
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==4) NtripSettings.baudOut = 57600;
    if (Pick_Parameter_Zahl("BAUDRATESET=", HTML_String)==5) NtripSettings.baudOut = 115200;   
    Serial.flush(); // wait for last transmitted data to be sent 
    Serial1.flush(); // wait for last transmitted data to be sent 
    Serial1.begin (NtripSettings.baudOut,SERIAL_8N1,RX1,TX1); //set new Baudrate
    DBG("\nRTCM/NMEA Baudrate: ");
    DBG(NtripSettings.baudOut, 1);
    EEprom_write_all();
   }
  if ( action == ACTION_SET_RESTART) {
    if (EEPROM.read(2)==0){
       EEPROM.write(2,1);
       EEPROM.commit();
       delay(2000);
       ESP.restart();
     } 
   }

  if ( action == ACTION_SET_GGA) {
    myIndex = Find_End("GGA_MY=", HTML_String);
    if (myIndex >= 0) {
       for (int i=0;i<100;i++) NtripSettings.GGAsentence[i]=0x00;
       Pick_Text(NtripSettings.GGAsentence, &HTML_String[myIndex], 100);
       exhibit ("NMEA: ", NtripSettings.GGAsentence);
      }
    EEprom_write_all();
  }

    if ( action == ACTION_SET_NMEAOUT) {
       NtripSettings.send_UDP_AOG = Pick_Parameter_Zahl("SENDNMEA_TYPE=", HTML_String);
       byte old = NtripSettings.enableNtrip;
       NtripSettings.enableNtrip = Pick_Parameter_Zahl("ENABLENTRIP=", HTML_String);
       if (NtripSettings.enableNtrip == 1 && old==0 ) restart == 0; // 
       EEprom_write_all();    
     } 

   /*if ( action == ACTION_SET_AHRS) {

    NtripSettings.AHRSbyte = 0;
    char tmp_string[20];
    for (int i = 0; i < 2; i++) {
      strcpy( tmp_string, "AHRS_TAG");
      strcati( tmp_string, i);
      strcat( tmp_string, "=");
      if (Pick_Parameter_Zahl(tmp_string, HTML_String) == 1)NtripSettings.AHRSbyte |= 1 << i;
     }

    EEprom_write_all();

   }*/
}  
//---------------------------------------------------------------------
// HTML Seite 01 aufbauen
//---------------------------------------------------------------------
void make_HTML01() {

  strcpy( HTML_String, "<!DOCTYPE html>");
  strcat( HTML_String, "<html>");
  strcat( HTML_String, "<head>");
  strcat( HTML_String, "<title>AG NTRIP Client Config Page</title>");
  strcat( HTML_String, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0;\" />\r\n");
  //strcat( HTML_String, "<meta http-equiv=\"refresh\" content=\"10\">");
  strcat( HTML_String, "<style>divbox {background-color: lightgrey;width: 200px;border: 5px solid red;padding:10px;margin: 10px;}</style>");
  strcat( HTML_String, "</head>");
  strcat( HTML_String, "<body bgcolor=\"#ff9900\">");
  strcat( HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
  strcat( HTML_String, "<h1>AG NTRIP-Client ESP</h1>");

  //-----------------------------------------------------------------------------------------
  // WiFi Client Access Data
  strcat( HTML_String, "(Rev. 1.2 by WEder)<br>");
  strcat( HTML_String, "<hr><h2>WiFi Network Client Access Data</h2>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "This NTRIP Client requires access to an Internet enabled Network!!<br><br>");
  strcat( HTML_String, "If access fails, an accesspoint will be created<br>");
  strcat( HTML_String, "(NTRIP_Client_ESP_Net PW:passport)<br><br>");
  strcat( HTML_String, "<table>");
  set_colgroup(150, 270, 150, 0, 0);

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Address:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"SSID_MY\" maxlength=\"22\" Value =\"");
  strcat( HTML_String, NtripSettings.ssid);
  strcat( HTML_String, "\"></td>");
  
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_SSID);
  strcat( HTML_String, "\">Submit</button></td>");
  strcat( HTML_String, "</tr>");

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Password:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"Password_MY\" maxlength=\"22\" Value =\"");
  strcat( HTML_String, NtripSettings.password);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");
  strcat( HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
  strcat( HTML_String, "<tr><td colspan=\"2\"><b>Restart NTRIP client for changes to take effect</b></td>");
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_RESTART);
  strcat( HTML_String, "\">Restart</button></td>");
  strcat( HTML_String, "</tr>");
  
  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");

//-------------------------------------------------------------  
// NTRIP Caster
  strcat( HTML_String, "<h2>NTRIP Caster Settings</h2>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "<table>");
  set_colgroup(150, 270, 150, 0, 0);
  
  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Network Name:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"CASTER\" maxlength=\"40\" Value =\"");
  strcat( HTML_String, NtripSettings.host);
  strcat( HTML_String, "\"></td>");
  
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_NTRIPCAST);
  strcat( HTML_String, "\">Submit</button></td>");
  strcat( HTML_String, "</tr>");

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Port:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"CASTERPORT\" maxlength=\"4\" Value =\"");
  strcati( HTML_String, NtripSettings.port);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");
  
  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Mountpoint:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"MOUNTPOINT\" maxlength=\"40\" Value =\"");
  strcat( HTML_String, NtripSettings.mountpoint);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Username:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"CASTERUSER\" maxlength=\"40\" Value =\"");
  strcat( HTML_String, NtripSettings.ntripUser);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");
  
  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Password:</b></td>");
  strcat( HTML_String, "<td>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:200px\" name=\"CASTERPWD\" maxlength=\"40\" Value =\"");
  strcat( HTML_String, NtripSettings.ntripPassword);
  strcat( HTML_String, "\"></td>");
  strcat( HTML_String, "</tr>");
  strcat( HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
  strcat( HTML_String, "<tr><td colspan=\"2\"><b>Restart NTRIP client for changes to take effect</b></td>");
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_RESTART);
  strcat( HTML_String, "\">Restart</button></td>");
  strcat( HTML_String, "</tr>");
  
  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");
//-------------------------------------------------------------  
// GGA processing
  strcat( HTML_String, "<h2>Send my Position</h2>");
  strcat( HTML_String, "(Required if your Caster provides VRS (Virtual Reference Station)<br>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "<table>");
  set_colgroup(150, 270, 150, 0, 0);

  strcat( HTML_String, "<br>");
  for (int i = 0; i < 3; i++) {
    strcat( HTML_String, "<tr>");
    if (i == 0)  strcat( HTML_String, "<td><b>Select Mode</b></td>");
    else strcat( HTML_String, "<td> </td>");
    strcat( HTML_String, "<td><input type = \"radio\" name=\"POSITION_TYPE\" id=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value=\"");
    strcati( HTML_String, i);
    strcat( HTML_String, "\"");
    if (NtripSettings.sendGGAsentence == i)strcat( HTML_String, " CHECKED");
    strcat( HTML_String, "><label for=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, position_type[i]);
    strcat( HTML_String, "</label></td>");
    if (i == 0){
      strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
      strcati(HTML_String, ACTION_SET_SENDPOS);
      strcat( HTML_String, "\">Submit</button></td>");
      strcat( HTML_String, "</tr>");
     }
  }
  strcat( HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
  
  
  for (int i = 0; i < 3; i++) {
    strcat( HTML_String, "<tr>");
    if (i == 0) {
      //strcat( HTML_String, "<td colspan=\"6\">&nbsp;</td><br>");
      strcat( HTML_String, "<td><b>Repeat time</b></td>");
     }
    else strcat( HTML_String, "<td>&nbsp;</td>");
    strcat( HTML_String, "<td><input type = \"radio\" name=\"REPEATTIME\" id=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value=\"");
    strcati( HTML_String, i);
    strcat( HTML_String, "\"");
    if (NtripSettings.GGAfreq == 1 && i==0)strcat( HTML_String, " CHECKED");
    if (NtripSettings.GGAfreq == 5 && i==1)strcat( HTML_String, " CHECKED");
    if (NtripSettings.GGAfreq == 10 && i==2)strcat( HTML_String, " CHECKED");
    strcat( HTML_String, "><label for=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, repeatPos[i]);
    strcat( HTML_String, "</label></td>");
  }
  strcat( HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
  
  for (int i = 0; i < 6; i++) {
    strcat( HTML_String, "<tr>");
    if (i == 0) strcat( HTML_String, "<td><b>Baudrate</b></td>");
    else strcat( HTML_String, "<td> </td>");
    strcat( HTML_String, "<td><input type = \"radio\" name=\"BAUDRATESET\" id=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value=\"");
    strcati( HTML_String, i);
    strcat( HTML_String, "\"");
    if ((NtripSettings.baudOut == 9600)   && i==0)strcat( HTML_String, " CHECKED");
    if ((NtripSettings.baudOut == 14400)  && i==1)strcat( HTML_String, " CHECKED");
    if ((NtripSettings.baudOut == 19200)  && i==2)strcat( HTML_String, " CHECKED");
    if ((NtripSettings.baudOut == 38400)  && i==3)strcat( HTML_String, " CHECKED");
    if ((NtripSettings.baudOut == 57600)  && i==4)strcat( HTML_String, " CHECKED");
    if ((NtripSettings.baudOut == 115200) && i==5)strcat( HTML_String, " CHECKED");
    strcat( HTML_String, "><label for=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, baud_output[i]);
    strcat( HTML_String, "</label></td>");
  }
  
  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");

//-------------------------------------------------------------  
// NMEA Sentence
  strcat( HTML_String, "<form>");
  set_colgroup(150, 270, 150, 0, 0);

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<h2>Fixed GGA or RMC Sentence of your Location:</h2>");
  strcat( HTML_String, "You can create your own from  <a href=\"https://www.nmeagen.org/\" target=\"_blank\">www.nmeagen.org</a><br><br>");
  strcat( HTML_String, "<input type=\"text\" style= \"width:650px\" name=\"GGA_MY\" maxlength=\"100\" Value =\"");
  strcat( HTML_String, NtripSettings.GGAsentence);
  strcat( HTML_String, "\"><br><br>");
  
  strcat( HTML_String, "<button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_GGA);
  strcat( HTML_String, "\">Submit</button>");
  strcat( HTML_String, "</tr>");

  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");
//-------------------------------------------------------------  
// NMEA transmission
  strcat( HTML_String, "<h2>Component Config</h2>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "<table>");
  set_colgroup(150, 270, 150, 0, 0);

  for (int i = 0; i < 2; i++) {
    strcat( HTML_String, "<tr>");
    if (i == 0)  strcat( HTML_String, "<td><b>NTRIP Client</b></td>");
    else strcat( HTML_String, "<td> </td>");
    strcat( HTML_String, "<td><input type = \"radio\" name=\"ENABLENTRIP\" id=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value=\"");
    strcati( HTML_String, i);
    strcat( HTML_String, "\"");
    if (NtripSettings.enableNtrip == i)strcat( HTML_String, " CHECKED");
    strcat( HTML_String, "><label for=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, ntripOn_type[i]);
    strcat( HTML_String, "</label></td>");
    if (i == 0){
      strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
      strcati(HTML_String, ACTION_SET_NMEAOUT);
      strcat( HTML_String, "\">Submit</button></td>");
      strcat( HTML_String, "</tr>");
     } 
  }
  strcat( HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

  for (int i = 0; i < 2; i++) {
    strcat( HTML_String, "<tr>");
    if (i == 0)  strcat( HTML_String, "<td><b>Transmission Mode</b></td>");
    else strcat( HTML_String, "<td> </td>");
    strcat( HTML_String, "<td><input type = \"radio\" name=\"SENDNMEA_TYPE\" id=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value=\"");
    strcati( HTML_String, i);
    strcat( HTML_String, "\"");
    if (NtripSettings.send_UDP_AOG == i)strcat( HTML_String, " CHECKED");
    strcat( HTML_String, "><label for=\"JZ");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, sendNmea[i]);
    strcat( HTML_String, "</label></td>");
    
  }
  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");
/* 
  //-------------------------------------------------------------  
  // Checkboxes AHRS
  strcat( HTML_String, "<h2>Config AHRS Funktions</h2>");
  strcat( HTML_String, "<form>");
  strcat( HTML_String, "<table>");
  set_colgroup(150, 270, 150, 0, 0);

  strcat( HTML_String, "<tr>");
  strcat( HTML_String, "<td><b>Select installed</b></td>");
  strcat( HTML_String, "<td>");
  for (int i = 0; i < 2; i++) {
    if (i == 1)strcat( HTML_String, "<br>");
    strcat( HTML_String, "<input type=\"checkbox\" name=\"AHRS_TAG");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" id = \"Part");
    strcati( HTML_String, i);
    strcat( HTML_String, "\" value = \"1\" ");
    if (NtripSettings.AHRSbyte & 1 << i) strcat( HTML_String, "checked ");
    strcat( HTML_String, "> ");
    strcat( HTML_String, "<label for =\"Part");
    strcati( HTML_String, i);
    strcat( HTML_String, "\">");
    strcat( HTML_String, AHRS_tab[i]);
    strcat( HTML_String, "</label>");
  }
  strcat( HTML_String, "</td>");
  strcat( HTML_String, "<td><button style= \"width:100px\" name=\"ACTION\" value=\"");
  strcati(HTML_String, ACTION_SET_AHRS);
  strcat( HTML_String, "\">Submit</button></td>");
  strcat( HTML_String, "</tr>"); 
 
  strcat( HTML_String, "</table>");
  strcat( HTML_String, "</form>");
  strcat( HTML_String, "<br><hr>");
*/
  
//-------------------------------------------------------------  
  strcat( HTML_String, "</font>");
  strcat( HTML_String, "</font>");
  strcat( HTML_String, "</body>");
  strcat( HTML_String, "</html>");
}

//--------------------------------------------------------------------------
void send_not_found() {

  DBG("\nSend Not Found\n");

  client_page.print("HTTP/1.1 404 Not Found\r\n\r\n");
  delay(20);
  //client_page.stop();
}

//--------------------------------------------------------------------------
void send_HTML() {
  char my_char;
  int  my_len = strlen(HTML_String);
  int  my_ptr = 0;
  int  my_send = 0;

  //--------------------------------------------------------------------------
  // in Portionen senden
  while ((my_len - my_send) > 0) {
    my_send = my_ptr + MAX_PACKAGE_SIZE;
    if (my_send > my_len) {
      client_page.print(&HTML_String[my_ptr]);
      delay(22);

      //Serial.println(&HTML_String[my_ptr]);

      my_send = my_len;
    } else {
      my_char = HTML_String[my_send];
      // Auf Anfang eines Tags positionieren
      while ( my_char != '<') my_char = HTML_String[--my_send];
      HTML_String[my_send] = 0;
      client_page.print(&HTML_String[my_ptr]);
      delay(22);
      
      //Serial.println(&HTML_String[my_ptr]);

      HTML_String[my_send] =  my_char;
      my_ptr = my_send;
    }
  }
  //client_page.stop();
}

//----------------------------------------------------------------------------------------------
void set_colgroup(int w1, int w2, int w3, int w4, int w5) {
  strcat( HTML_String, "<colgroup>");
  set_colgroup1(w1);
  set_colgroup1(w2);
  set_colgroup1(w3);
  set_colgroup1(w4);
  set_colgroup1(w5);
  strcat( HTML_String, "</colgroup>");

}
//------------------------------------------------------------------------------------------
void set_colgroup1(int ww) {
  if (ww == 0) return;
  strcat( HTML_String, "<col width=\"");
  strcati( HTML_String, ww);
  strcat( HTML_String, "\">");
}


//---------------------------------------------------------------------
void strcatf(char* tx, float f) {
  char tmp[8];

  dtostrf(f, 6, 2, tmp);
  strcat (tx, tmp);
}
//---------------------------------------------------------------------
//void strcatl(char* tx, long l) {
  //char tmp[sizeof l];
  //memcpy(tmp, l, sizeof l);
  //strcat (tx, tmp);
//}

//---------------------------------------------------------------------
void strcati(char* tx, int i) {
  char tmp[8];

  itoa(i, tmp, 10);
  strcat (tx, tmp);
}

//---------------------------------------------------------------------
void strcati2(char* tx, int i) {
  char tmp[8];

  itoa(i, tmp, 10);
  if (strlen(tmp) < 2) strcat (tx, "0");
  strcat (tx, tmp);
}

//---------------------------------------------------------------------
int Pick_Parameter_Zahl(const char * par, char * str) {
  int myIdx = Find_End(par, str);

  if (myIdx >= 0) return  Pick_Dec(str, myIdx);
  else return -1;
}
//---------------------------------------------------------------------
int Find_End(const char * such, const char * str) {
  int tmp = Find_Start(such, str);
  if (tmp >= 0)tmp += strlen(such);
  return tmp;
}

//---------------------------------------------------------------------
int Find_Start(const char * such, const char * str) {
  int tmp = -1;
  int ww = strlen(str) - strlen(such);
  int ll = strlen(such);

  for (int i = 0; i <= ww && tmp == -1; i++) {
    if (strncmp(such, &str[i], ll) == 0) tmp = i;
  }
  return tmp;
}
//---------------------------------------------------------------------
int Pick_Dec(const char * tx, int idx ) {
  int tmp = 0;

  for (int p = idx; p < idx + 5 && (tx[p] >= '0' && tx[p] <= '9') ; p++) {
    tmp = 10 * tmp + tx[p] - '0';
  }
  return tmp;
}
//----------------------------------------------------------------------------
int Pick_N_Zahl(const char * tx, char separator, byte n) {

  int ll = strlen(tx);
  int tmp = -1;
  byte anz = 1;
  byte i = 0;
  while (i < ll && anz < n) {
    if (tx[i] == separator)anz++;
    i++;
  }
  if (i < ll) return Pick_Dec(tx, i);
  else return -1;
}

//---------------------------------------------------------------------
int Pick_Hex(const char * tx, int idx ) {
  int tmp = 0;

  for (int p = idx; p < idx + 5 && ( (tx[p] >= '0' && tx[p] <= '9') || (tx[p] >= 'A' && tx[p] <= 'F')) ; p++) {
    if (tx[p] <= '9')tmp = 16 * tmp + tx[p] - '0';
    else tmp = 16 * tmp + tx[p] - 55;
  }

  return tmp;
}

//---------------------------------------------------------------------
void Pick_Text(char * tx_ziel, char  * tx_quelle, int max_ziel) {

  int p_ziel = 0;
  int p_quelle = 0;
  int len_quelle = strlen(tx_quelle);

  while (p_ziel < max_ziel && p_quelle < len_quelle && tx_quelle[p_quelle] && tx_quelle[p_quelle] != ' ' && tx_quelle[p_quelle] !=  '&') {
    if (tx_quelle[p_quelle] == '%') {
      tx_ziel[p_ziel] = (HexChar_to_NumChar( tx_quelle[p_quelle + 1]) << 4) + HexChar_to_NumChar(tx_quelle[p_quelle + 2]);
      p_quelle += 2;
    } else if (tx_quelle[p_quelle] == '+') {
      tx_ziel[p_ziel] = ' ';
    }
    else {
      tx_ziel[p_ziel] = tx_quelle[p_quelle];
    }
    p_ziel++;
    p_quelle++;
  }

  tx_ziel[p_ziel] = 0;
}
//---------------------------------------------------------------------
char HexChar_to_NumChar( char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 55;
  return 0;
}
//---------------------------------------------------------------------
void exhibit(const char * tx, int v) {
  DBG(tx);
  DBG(v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, unsigned int v) {
  DBG(tx);
  DBG((int)v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, unsigned long v) {
  DBG(tx);
  DBG((long)v, 1);
}
//---------------------------------------------------------------------
void exhibit(const char * tx, const char * v) {
  DBG(tx);
  DBG(v, 1);
}
