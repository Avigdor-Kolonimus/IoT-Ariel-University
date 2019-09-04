/******************************************************************************************************************
* Authors: Shir, Hodaya and Alexey 
* Version: 3.0
 ******************************************************************************************************************/
// libraries
#include <Wire.h> // Only needed for Arduino 1.6.5 and earlier
#include <PubSubClient.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "DHT.h"
#include "WiFi.h"

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

// Initialize MQTT
/* this is the IP of PC/raspberry where you installed MQTT Server 
on Wins use "ipconfig" 
on Linux use "ifconfig" to get its IP address */
const char* mqttServer = "IP_MQTT_SERVER";
const int mqttPort = 1883;
const char* mqttUser = "YOURUSER";
const char* mqttPassword = "YOURPASSWORD";

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Client";
    /* connect now */
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600); 
  /***************************************** Part of WiFi *********************************************************/
  /* Set ESP32 to WiFi Station mode */
  WiFi.mode(WIFI_AP_STA);
  /* start SmartConfig */
  WiFi.beginSmartConfig();

  /* Wait for SmartConfig packet from mobile */
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("SmartConfig done.");

  /* Wait for WiFi to connect to AP */
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /***************************************** Part of MQTT *********************************************************/
  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqttServer, mqttPort);
  /* this receivedCallback function will be invoked 
  when client received subscribed topic */
  client.setCallback(receivedCallback);
  /*start DHT sensor */
  dht.begin();
  delay(5000);
}

void loop() {
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming 
  subscribed topic-process-invoke receivedCallback */
  //client.loop();
  
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
  const char * msg = send_message.c_str();
  Serial.println(send_message);
  client.publish("OurEsp", msg);
  
  // Wait between measurements
  delay(5000);
}
