----------------------------------------------------------------------
                NTRIP Client for ESP32
----------------------------------------------------------------------

Easy example NTRIP client written with Arduino IDE for 
a espressif ESP32.   Copyright (C) 2018- by Wilhelm Eder

Now enhanced with Bluetooth NMEA Output.
Optional IMU (BNO055) module,and optional Inclinometer (MMA8452) module.

The IMU data is transmitted to AOG via Wifi like the NMEA Data. 
AOG is now enhanced to prefer enabled IMU/Inclino data from Roofcontroller 
instead of the data from Autosteer module. 

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, 
or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
or read http://www.gnu.org/licenses/gpl.txt

Files in > AG_NTRIP_ESP
-----------------------------
AG_NTRIP_ESP_BNO_MMA_BT.ino:    Ntrip client boot up source code
Core1code:                      Code for Core1, mainly the NTrip client
Core2code:                      Code, serving the Setup Webpage and reading serial data from GPS
Misc.ino:                       Write and read the nonvolatile EEPROM (ESP uses Flash instead)
Network_AOG.h:                  Definitions used by Network_AOG.ino 
Network_AOG.ino:                The Webpage server
BNO_ESP.h                       BNO055 IMU Library
BNO_ESP.ino                             "
MMA8452_AOG.h                   MMA 8452 Library
MMA8452_AOG.ino                         "
README.txt:                     this Documentation


Ntrip
-----
The ntripclient is an HTTP client based on 'Networked Transport
of RTCM via Internet Protocol' (Ntrip). This is an application-level 
protocol streaming Global Navigation Satellite System (GNSS) data over 
the Internet. 

Ntrip is designed for disseminating differential correction data 
(e.g in the RTCM-104 format) or other kinds of GNSS streaming data to
stationary or mobile users over the Internet, allowing simultaneous PC,
Laptop, PDA, or receiver connections to a broadcasting host. Ntrip 
supports Wireless Internet access through Mobile IP Networks like GSM, 
GPRS, EDGE, or UMTS.

Ntrip is implemented in three system software components: NtripClients, 
NtripServers and NtripCasters. The NtripCaster is the actual HTTP 
server program whereas NtripClient and NtripServer are acting as HTTP 
clients.

ntripclient
-----------
This ESP32 Ntrip client program is written under GNU General Public 
License in C programming language. The program reads data from an Ntrip 
Broadcaster and writes on standard output to a COM-port. 
PLEASE NOTE THAT THIS PROGRAM VERSION DOES NOT HANDLE 
POTENTIALLY OCCURRING INTERRUPTIONS OF COMMUNICATION
OR NETWORK CONGESTION SITUATIONS. Its distribution may stimulate
those intending to write their own client program.


Use of the Client:

After first start, the Client will create a Hotspot where you can enter the 
Access Data to your WiFi, which is required to gain Internet access.
This could be a GSM Router, Mobile Hotspot or Tablet Hotspot.

The Access to the ESP Hotspot:

Network Name:  NTRIP_Client_ESP_Net
Password:             passport

Configpage:          192.168.1.79

after restart, the Client logs into your WiFi and you can access it also via 
the Adress:   192.168.1.79 **

** If your Network use other Network than 192.168.1.xxx you have to modify the 
Clients IP Adress at AG_NTRIP_ESP_BNO_MMA_BT.ino  before uploading 
the Sketch to the ESP!

The Client uses the Serial Port 0 output (mostly the USB connector to PC) for 
detailed Debug Information from the Client. Baudrate is 115200.
Here you get the Status of the Client and also a printout of the 
Casters Sourcetable, where all the Mountpoints of your Caster are listed.

Also Serial 1 of the ESP is used:
a)  to output the correcction Data stream (RTCM) to your GPS receiver
b) optionaly read your location and transmit it to your Caster (required by VRS)
c) reads GGA, RMC, VTG - NMEA and transmit it via UDP to AOG.


Caster Registration
------------
Some of the data streams (mountpoints) from an NtripCaster may be
available for test, demonstration, and evaluation purposes and
accessible without authentication/authorization. For accessing other
data streams (mountpoints) the user needs a user-ID and a
user password. Authorization can be provided for a single stream,
for a group of streams (network) or for all available streams.


Disclaimer
----------
Note that this ntripclient program is for experimental use
only. I disclaim any liability nor responsibility to any person or entity 
with respect to any loss or damage caused, or alleged 
to be caused, directly or indirectly by the use and application of the 
Ntrip technology.

Further Information
-------------------
http://igs.bkg.bund.de/index_ntrip.htm
euref-ip@bkg.bund.de
