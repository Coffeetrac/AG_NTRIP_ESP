//--------------------------------------------------------------
//  EEPROM Data Handling
//--------------------------------------------------------------
#define EEPROM_SIZE 512
#define EE_ident1 0xED  // Marker Byte 0 + 1
#define EE_ident2 0xEA


//--------------------------------------------------------------
byte EEprom_empty_check(){
    
  if (!EEPROM.begin(EEPROM_SIZE))  
    {
     Serial.println("failed to initialise EEPROM"); delay(1000);
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
  Serial.print(EEPROM_SIZE);
  Serial.println(" bytes read from Flash . Values are:");
  for (int i = 0; i < EEPROM_SIZE; i++)
  { 
    data_=byte(EEPROM.read(i));
    if (data_ < 0x10) Serial.print("0");
    Serial.print(data_,HEX); 
    if (c2==15) {
       Serial.print(" ");
      }
    else if (c2>=31) {
           Serial.println();
           c2=-1;
          }
    else Serial.print(" ");
    c2++;
  }
}





   
