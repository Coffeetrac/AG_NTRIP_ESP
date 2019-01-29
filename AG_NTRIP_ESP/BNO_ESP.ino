//#include "BNO_ESP.h"
//===================================================================================================================
//====== Set of useful function to access acceleration. gyroscope, magnetometer, and temperature data
//===================================================================================================================

void readEulData(int16_t * destination)
{
  uint8_t rawData[6];  // x/y/z gyro register data stored here
  readBytes(BNO055_ADDRESS, BNO055_EUL_HEADING_LSB, 6, &rawData[0]);  // Read the six raw data registers sequentially into data array
  destination[0] = ((int16_t)rawData[1] << 8) | rawData[0] ;       // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = ((int16_t)rawData[3] << 8) | rawData[2] ;  
  destination[2] = ((int16_t)rawData[5] << 8) | rawData[4] ;
}


bool initBNO055() {
    byte c = readByte(BNO055_ADDRESS, BNO055_CHIP_ID);  // Read WHO_AM_I register for BNO055
    if (c != 0xA0) 
     {
       Serial.print("!! IMU Init fails! should be 0xA0, is:");
       Serial.println(c, HEX);
       return false; 
     }
   // Select BNO055 config mode
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
   delay(25);
   // Select page 1 to configure sensors
   writeByte(BNO055_ADDRESS, BNO055_PAGE_ID, 0x01);
   // Configure ACC
   writeByte(BNO055_ADDRESS, BNO055_ACC_CONFIG, APwrMode << 5 | Abw << 2 | Ascale );
   // Configure GYR
   writeByte(BNO055_ADDRESS, BNO055_GYRO_CONFIG_0, Gbw << 3 | Gscale );
   writeByte(BNO055_ADDRESS, BNO055_GYRO_CONFIG_1, GPwrMode);
   // Configure MAG
   writeByte(BNO055_ADDRESS, BNO055_MAG_CONFIG, MPwrMode << 5 | MOpMode << 3 | Modr );
   
   // Select page 0 to read sensors
   writeByte(BNO055_ADDRESS, BNO055_PAGE_ID, 0x00);

   // Select BNO055 gyro temperature source 
   writeByte(BNO055_ADDRESS, BNO055_TEMP_SOURCE, 0x01 );

   // Select BNO055 sensor units (temperature in degrees C, rate in dps, accel in mg)
   writeByte(BNO055_ADDRESS, BNO055_UNIT_SEL, 0x01 );
   
   // Select BNO055 system power mode
   writeByte(BNO055_ADDRESS, BNO055_PWR_MODE, PWRMode );
 
   // Select BNO055 system operation mode
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, OPRMode );
   Serial.println("BNO055 IMU Init done!");
   delay(25);
   return true;
}

void accelgyroCalBNO055(float * dest1, float * dest2) 
{
  uint8_t data[6]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii = 0, sample_count = 0;
  int32_t gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
 
  Serial.println("Accel/Gyro Calibration: Put device on a level surface and keep motionless! Wait......");
  delay(4000);
  
  // Select page 0 to read sensors
   writeByte(BNO055_ADDRESS, BNO055_PAGE_ID, 0x00);
   // Select BNO055 system operation mode as AMG for calibration
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
   delay(25);
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, AMG);
   
 // In NDF fusion mode, accel full scale is at +/- 4g, ODR is 62.5 Hz, set it the same here
   writeByte(BNO055_ADDRESS, BNO055_ACC_CONFIG, APwrMode << 5 | Abw << 2 | AFS_4G );
   sample_count = 256;
   for(ii = 0; ii < sample_count; ii++) {
    int16_t accel_temp[3] = {0, 0, 0};
    readBytes(BNO055_ADDRESS, BNO055_ACC_DATA_X_LSB, 6, &data[0]);  // Read the six raw data registers into data array
    accel_temp[0] = (int16_t) (((int16_t)data[1] << 8) | data[0]) ; // Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t) (((int16_t)data[3] << 8) | data[2]) ;
    accel_temp[2] = (int16_t) (((int16_t)data[5] << 8) | data[4]) ;
    accel_bias[0]  += (int32_t) accel_temp[0];
    accel_bias[1]  += (int32_t) accel_temp[1];
    accel_bias[2]  += (int32_t) accel_temp[2];
    delay(20);  // at 62.5 Hz ODR, new accel data is available every 16 ms
   }
    accel_bias[0]  /= (int32_t) sample_count;  // get average accel bias in mg
    accel_bias[1]  /= (int32_t) sample_count;
    accel_bias[2]  /= (int32_t) sample_count;
    
  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) 1000;}  // Remove gravity from the z-axis accelerometer bias calculation
  else {accel_bias[2] += (int32_t) 1000;}

    dest1[0] = (float) accel_bias[0];  // save accel biases in mg for use in main program
    dest1[1] = (float) accel_bias[1];  // accel data is 1 LSB/mg
    dest1[2] = (float) accel_bias[2];          

 // In NDF fusion mode, gyro full scale is at +/- 2000 dps, ODR is 32 Hz
   writeByte(BNO055_ADDRESS, BNO055_GYRO_CONFIG_0, Gbw << 3 | GFS_2000DPS );
   writeByte(BNO055_ADDRESS, BNO055_GYRO_CONFIG_1, GPwrMode);
   for(ii = 0; ii < sample_count; ii++) {
    int16_t gyro_temp[3] = {0, 0, 0};
    readBytes(BNO055_ADDRESS, BNO055_GYR_DATA_X_LSB, 6, &data[0]);  // Read the six raw data registers into data array
    gyro_temp[0] = (int16_t) (((int16_t)data[1] << 8) | data[0]) ;  // Form signed 16-bit integer for each sample in FIFO
    gyro_temp[1] = (int16_t) (((int16_t)data[3] << 8) | data[2]) ;
    gyro_temp[2] = (int16_t) (((int16_t)data[5] << 8) | data[4]) ;
    gyro_bias[0]  += (int32_t) gyro_temp[0];
    gyro_bias[1]  += (int32_t) gyro_temp[1];
    gyro_bias[2]  += (int32_t) gyro_temp[2];
    delay(35);  // at 32 Hz ODR, new gyro data available every 31 ms
   }
    gyro_bias[0]  /= (int32_t) sample_count;  // get average gyro bias in counts
    gyro_bias[1]  /= (int32_t) sample_count;
    gyro_bias[2]  /= (int32_t) sample_count;
 
    dest2[0] = (float) gyro_bias[0]/16.;  // save gyro biases in dps for use in main program
    dest2[1] = (float) gyro_bias[1]/16.;  // gyro data is 16 LSB/dps
    dest2[2] = (float) gyro_bias[2]/16.;          

  // Return to config mode to write accelerometer biases in offset register
  // This offset register is only used while in fusion mode when accelerometer full-scale is +/- 4g
  writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
  delay(25);
  
  //write biases to accelerometer offset registers ad 16 LSB/dps
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_X_LSB, (int16_t)accel_bias[0] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_X_MSB, ((int16_t)accel_bias[0] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_Y_LSB, (int16_t)accel_bias[1] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_Y_MSB, ((int16_t)accel_bias[1] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_Z_LSB, (int16_t)accel_bias[2] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_ACC_OFFSET_Z_MSB, ((int16_t)accel_bias[2] >> 8) & 0xFF);
  
   //write biases to gyro offset registers
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_X_LSB, (int16_t)gyro_bias[0] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_X_MSB, ((int16_t)gyro_bias[0] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_Y_LSB, (int16_t)gyro_bias[1] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_Y_MSB, ((int16_t)gyro_bias[1] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_Z_LSB, (int16_t)gyro_bias[2] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_GYR_OFFSET_Z_MSB, ((int16_t)gyro_bias[2] >> 8) & 0xFF);
  
  // Select BNO055 system operation mode
  writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, OPRMode );

   Serial.println("Accel/Gyro Calibration done!");
}

void magCalBNO055(float * dest1) 
{
  uint8_t data[6]; // data array to hold mag x, y, z, data
  uint16_t ii = 0, sample_count = 0;
  int32_t mag_bias[3] = {0, 0, 0};
  int16_t mag_max[3] = {(int16_t)0x8000, (int16_t)0x8000, (int16_t)0x8000}, mag_min[3] = {0x7FFF, 0x7FFF, 0x7FFF};
 
  Serial.println("Mag Calibration: Wave device in a figure eight until done!");
  delay(4000);
  
  // Select page 0 to read sensors
   writeByte(BNO055_ADDRESS, BNO055_PAGE_ID, 0x00);
   // Select BNO055 system operation mode as AMG for calibration
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
   delay(25);
   writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, AMG );

 // In NDF fusion mode, mag data is in 16 LSB/microTesla, ODR is 20 Hz in forced mode
   sample_count = 256;
   for(ii = 0; ii < sample_count; ii++) {
    int16_t mag_temp[3] = {0, 0, 0};
    readBytes(BNO055_ADDRESS, BNO055_MAG_DATA_X_LSB, 6, &data[0]);  // Read the six raw data registers into data array
    mag_temp[0] = (int16_t) (((int16_t)data[1] << 8) | data[0]) ;   // Form signed 16-bit integer for each sample in FIFO
    mag_temp[1] = (int16_t) (((int16_t)data[3] << 8) | data[2]) ;
    mag_temp[2] = (int16_t) (((int16_t)data[5] << 8) | data[4]) ;
    for (int jj = 0; jj < 3; jj++) {
      if (ii == 0) {
        mag_max[jj] = mag_temp[jj]; // Offsets may be large enough that mag_temp[i] may not be bipolar! 
        mag_min[jj] = mag_temp[jj]; // This prevents max or min being pinned to 0 if the values are unipolar...
      } else {
        if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
        if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
      }
    }
    delay(105);  // at 10 Hz ODR, new mag data is available every 100 ms
   }

    mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
    mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
    mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts
    
    dest1[0] = (float) mag_bias[0] / 1.6;  // save mag biases in mG for use in main program
    dest1[1] = (float) mag_bias[1] / 1.6;  // mag data is 1.6 LSB/mg
    dest1[2] = (float) mag_bias[2] / 1.6;          

  // Return to config mode to write mag biases in offset register
  // This offset register is only used while in fusion mode when magnetometer sensitivity is 16 LSB/microTesla
  writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
  delay(25);
  
  //write biases to magnetometer offset registers as 16 LSB/microTesla
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_X_LSB, (int16_t)mag_bias[0] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_X_MSB, ((int16_t)mag_bias[0] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_Y_LSB, (int16_t)mag_bias[1] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_Y_MSB, ((int16_t)mag_bias[1] >> 8) & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_Z_LSB, (int16_t)mag_bias[2] & 0xFF);
  writeByte(BNO055_ADDRESS, BNO055_MAG_OFFSET_Z_MSB, ((int16_t)mag_bias[2] >> 8) & 0xFF);
 
  writeByte(BNO055_ADDRESS, BNO055_OPR_MODE, OPRMode );

   Serial.println("Mag Calibration done!");
}

// I2C read/write functions for the BNO055 sensor
  void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire.beginTransmission(address);  // Initialize the Tx buffer
	Wire.write(subAddress);           // Put slave register address in Tx buffer
	Wire.write(data);                 // Put data in Tx buffer
	Wire.endTransmission();           // Send the Tx buffer
}

  uint8_t readByte(uint8_t address, uint8_t subAddress)
{ 
	uint8_t _data, retval; // `data` will store the register data	 
	Wire.beginTransmission(address);         // Initialize the Tx buffer
	Wire.write(subAddress);	                 // Put slave register address in Tx buffer
	Wire.endTransmission(false);             // Send the Tx buffer, but send a restart to keep connection alive
	retval = Wire.requestFrom(address, (uint8_t) 1);            // Read one byte from slave register address 
	_data = Wire.read();                      // Fill Rx buffer with result
	return _data;                             // Return data read from slave register
}

  void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest)
{ uint8_t retval; 
	Wire.beginTransmission(address);   // Initialize the Tx buffer
	Wire.write(subAddress);            // Put slave register address in Tx buffer
	Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
	uint8_t i = 0;
  retval = Wire.requestFrom(address, count);  // Read bytes from slave register address 
	while (Wire.available()) {
        dest[i++] = Wire.read(); }         // Put read results in the Rx buffer
}
