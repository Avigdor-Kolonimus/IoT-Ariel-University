/******************************************************************************************************************
* Authors: Shir, Hodaya and Alexey 
* Version: 1.0
 ******************************************************************************************************************/
// libraries
#include <Wire.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "DHT.h"

// Initialize the light intensity sensor
#define InPin 12
int value_light ;

// Initialize DHT sensor
// as the current DHT reading algorithm adjusts itself to work on faster procs.
#define DHTPIN 14         // what digital pin we're connected to
#define DHTTYPE DHT11     // DHT 11// Uncomment whatever type you're using!
DHT dht(DHTPIN, DHTTYPE);

// Initialize the soil moisture sensor
int sensor_pin = 0;       // sensor input at Analog pin A0
int value_moisture;

void setup() {
  Serial.begin(9600);
  /*start DHT sensor */
  dht.begin();
  delay(5000);
}

void loop() {
  // **************************************************************************************** Light Sensor ****************************************************************
  value_light = analogRead( InPin );

  // ***************************************************************************************** DHT Sensor *****************************************************************
  // DHT Sensor (Temp.& Humidity)
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default) and as Fahrenheit (isFahrenheit = true)
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(5000);
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  // ************************************************************************************* Soil Moisture Sensor ***********************************************************
  /*
   * When taking the analog output from the soil moisture sensor, the sensor gives us a value from 0 to 4095. 
   * The moisture is measured in percentage, so we will map these values from 0 to 100 and then we will show these values on the serial monitor.
   */
  value_moisture = analogRead(sensor_pin);
  value_moisture = map( value_moisture, 4095, 0, 0, 100) + 39;

  // ************************************************************************************* MQTT ***************************************************************************
  //                         Soil Moisture                 Light                     Humidity           Temperature          Heat index
  String send_message = String(value_moisture) + " , " +String(value_light) + " , " + String(h) + " , " + String(t) + ", " + String(hic);
  Serial.println(send_message);
  
  // Wait between measurements
  delay(5000);
}
