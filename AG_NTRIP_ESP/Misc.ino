//--------------------------------------------------------------
//  EEPROM Data Handling
//--------------------------------------------------------------
#define EEPROM_SIZE 512
#define EE_ident1 0xED  // Marker Byte 0 + 1
#define EE_ident2 0xED


//--------------------------------------------------------------
//  Restore EEprom Data
//--------------------------------------------------------------
void restoreEEprom(){
  byte get_state  = digitalRead(restoreDefault_PIN);
  if (debugmode) get_state = true;
  if (get_state ) DBG("State: restoring default values !\n");
  else DBG("State: read default values from EEPROM\n");
  
  if (EEprom_empty_check()==1 || get_state) { //first start?
    EEprom_write_all();     //write default data
   }
  if (EEprom_empty_check()==2) { //data available
    EEprom_read_all();
   }
  //EEprom_show_memory();  //
  EE_done =1;   
}

//--------------------------------------------------------------
byte EEprom_empty_check(){
    
  if (!EEPROM.begin(EEPROM_SIZE))  
    {
     DBG("failed to initialise EEPROM\n"); delay(1000);
     return false;
    }
  if (EEPROM.read(0)!= EE_ident1 || EEPROM.read(1)!= EE_ident2)
     return true;  // is empty
  
  if (EEPROM.read(0)== EE_ident1 && EEPROM.read(1)== EE_ident2)
     return 2;     // data available
     
 }
//--------------------------------------------------------------
void EEprom_write_all(){  // called if EEPROM empty
  EEPROM.write(0, EE_ident1);
  EEPROM.write(1, EE_ident2);
  EEPROM.write(2, 0); // reset Restart blocker
  EEPROM.put(3, NtripSettings);
  EEPROM.commit();
}
//--------------------------------------------------------------
void EEprom_read_all(){
  
  EEPROM.get(3, NtripSettings);
  
}
//--------------------------------------------------------------
void EEprom_show_memory(){
byte c2=0, data_;
  DBG(EEPROM_SIZE, 1);
  DBG(" bytes read from Flash . Values are:\n");
  for (int i = 0; i < EEPROM_SIZE; i++)
  { 
    data_=byte(EEPROM.read(i));
    if (data_ < 0x10) Serial.print("0");
    DBG(data_,HEX); 
    if (c2==15) {
       DBG(" ");
      }
    else if (c2>=31) {
           DBG("",1); //NL
           c2=-1;
          }
    else DBG(" ");
    c2++;
  }
}







   
