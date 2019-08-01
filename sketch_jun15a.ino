#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 12 // what digital pin we're connected to NodeMCU (D6)
#define INPUT_PIN4 4 //NodeMCU (D2)
#define INPUT_PIN5 5 //NodeMCU (D1)
#define INPUT_PIN16 16 //NodeMCU (D0)
#define INPUT_PIN14 14 //NodeMCU (D5)
#define INPUT_PIN1 1 //NodeMCU (TX)
#define OUTPUT_PIN0 0 //NodeMCU (D3)

bool inp = 0;
int led = 2;

// Uncomment whatever type you're using!
#define DHTTYPE DHT22 // DHT 11

//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors. This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.


DHT dht(DHTPIN, DHTTYPE);

//#define wifi_ssid "Lenovo"
//#define wifi_password "12345678"
#define wifi_ssid "Servus"
#define wifi_password "Ahnyy4hau7dm"

#define mqtt_server "192.168.0.31"
#define mqtt_user ""
#define mqtt_password ""

#define humidity_topic "sensor/humidity"
#define temperature_celsius_topic "sensor/temperature_celsius"
#define temperature_fahrenheit_topic "sensor/temperature_fahrenheit"

#define PIN_D0 "DigitalInput/D0"
#define PIN_D1 "DigitalInput/D1"
#define PIN_D2 "DigitalInput/D2"
#define PIN_D5 "DigitalInput/D5"
#define PIN_TX "DigitalInput/TX"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  pinMode(OUTPUT_PIN0, OUTPUT);
  digitalWrite(OUTPUT_PIN0, LOW);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(OUTPUT_PIN0, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
    
          // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;  
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);


    // Attempt to connect
    // If you do not want to use a username and password, change next line to
  if (client.connect((char*) clientName.c_str())) {
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      digitalWrite(OUTPUT_PIN0, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      digitalWrite(OUTPUT_PIN0, LOW);

    }
  }
}


void loop() {
  
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      // Wait a few seconds between measurements.
      delay(2000);
      
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);
     
      pinMode(INPUT_PIN4, INPUT); // Configure pin INPUT_PIN4 as input
      pinMode(INPUT_PIN5, INPUT); // Configure pin INPUT_PIN5 as input
      pinMode(INPUT_PIN16, INPUT_PULLDOWN_16); // Configure pin INPUT_PIN16 as input
      pinMode(INPUT_PIN14, INPUT); // Configure pin INPUT_PIN0 as input
      pinMode(INPUT_PIN1, INPUT); // Configure pin INPUT_PINTX as input
      
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
      }
      
      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.print(" *C ");
      Serial.print(hif);
      Serial.println(" *F");


      Serial.print("Temperature in Celsius:");
      Serial.println(String(t).c_str());
      client.publish(temperature_celsius_topic, String(t).c_str(), true);

      Serial.print("Temperature in Fahrenheit:");
      Serial.println(String(f).c_str());
      client.publish(temperature_fahrenheit_topic, String(f).c_str(), true);


      Serial.print("Humidity:");
      Serial.println(String(h).c_str());
      client.publish(humidity_topic, String(h).c_str(), true);
  
  inp=digitalRead(INPUT_PIN16);
  Serial.print("Read digital input D0: ");
  Serial.println(inp);
  client.publish(PIN_D0, String(inp).c_str(), true);

  inp=digitalRead(INPUT_PIN5);
  Serial.print("Read digital input D1: ");
  Serial.println(inp);
  client.publish(PIN_D1, String(inp).c_str(), true);

  inp=digitalRead(INPUT_PIN4);
  Serial.print("Read digital input D2: ");
  Serial.println(inp);
  client.publish(PIN_D2, String(inp).c_str(), true);

  inp=digitalRead(INPUT_PIN14);
  Serial.print("Read digital input D5: ");
  Serial.println(inp);
  client.publish(PIN_D5, String(inp).c_str(), true);

   inp=digitalRead(INPUT_PIN1);
  Serial.print("Read digital input RX: ");
  Serial.println(inp);
  client.publish(PIN_TX, String(inp).c_str(), true);
  
}
