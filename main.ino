/*
  Datacake Arduino MKR WAN 1310 + MKR ENV Demo Firmware
  Made for The Things Summer Academy
*/

#include <MKRWAN.h>
#include <Arduino_MKRENV.h>
#include <CayenneLPP.h>

// Murata Module
LoRaModem modem(Serial1);

// LoRaWAN Configuration
// Get your devEUI from Murata Module
#define LORAREGION US915
String appEui = "0000000000000000";
String appKey = "926C29B2EEFF8890D9A853C618EFAACB";

// CayenneLPP Configuration
CayenneLPP lpp(51);

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);

  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(LORAREGION)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

  // Enable US915-928 channels
  // LoRaWAN® Regional Parameters and TTN specification: channels 8 to 15 plus 65 
  modem.sendMask("ff000001f000ffff00020000");
  Serial.print("Set Channel Mask --> ");
  Serial.println(modem.getChannelMask());
  modem.setADR(true);

  // Connect via LoRaWAN
  Serial.println("Connecting...");
  int connected = modem.joinOTAA(appEui, appKey);

  // Check Connectivity
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  // print an empty line
  Serial.println();

  // We are connected ... idle a few
  delay(1000);

  // init ENV Board
  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  } 

  // All Done ... idle a few
  delay(1000);
}

void loop() {

  // Read Sensors from ENV Board
  Serial.println("Reading sensor values...");

  float temperature = ENV.readTemperature(FAHRENHEIT);
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure(PSI);
  float illuminance = ENV.readIlluminance(LUX);
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex();  

// print each of the sensor values
  Serial.print("  Temperature = ");
  Serial.print(temperature);
  Serial.println(" °F");

  Serial.print("  Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("  Pressure    = ");
  Serial.print(pressure);
  Serial.println(" psi");

  Serial.print("  Illuminance = ");
  Serial.print(illuminance);
  Serial.println(" lx");

  Serial.print("  UVA         = ");
  Serial.print(uva);
  Serial.println(" ");

  Serial.print("  UVB         = ");
  Serial.print(uvb);
  Serial.println(" ");
  
  Serial.print("  UV Index    = ");
  Serial.print(uvIndex);
  Serial.println(" ");
  
  // Create LPP
  lpp.reset();
  lpp.addTemperature(0, temperature);
  lpp.addRelativeHumidity(0, humidity);  
  lpp.addBarometricPressure(0, pressure);
  lpp.addLuminosity(0, illuminance);
  lpp.addTemperature(1, uva);
  lpp.addTemperature(2, uvb);
  lpp.addTemperature(3, uvIndex);

  // Send LPP Packet over LoRaWAN
  Serial.println("Sending message...");
  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  int err = modem.endPacket(true);
  
  // Check for errors
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
    Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
    Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
  }

  // Idle 60 secs and start again
  delay(60000);

  // print an empty line
  Serial.println();
}