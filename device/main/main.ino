#include <SimpleDHT.h>                   // Data ---> D3 VCC ---> 3V3 GND ---> GND
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <BlynkSimpleEsp8266.h>
#include <vector>
#include <string>
// WiFi parameters
#define WLAN_SSID       "bao"
#define WLAN_PASS       "12345678"
// Adafruit IO
#define AIO_SERVER      "192.168.82.227"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "bao"
#define AIO_KEY         "12345678" 
WiFiClient client;


#define BLYNK_TEMPLATE_ID "TMPLp8lBDHir"
#define BLYNK_TEMPLATE_NAME "CamBienChuongGa"
#define BLYNK_AUTH_TOKEN "7FZEdb4jgC0HivDesKL9g0d7SkHRZS1Q"
char auth[] = "7FZEdb4jgC0HivDesKL9g0d7SkHRZS1Q";
int num_led;
int auto_tune=1;
std::vector<double> temp_log;
std::vector<double> hum_log;
double avg_temp;
double avg_hum;
double tmp_temp;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Log1 = Adafruit_MQTT_Publish(&mqtt, "log");

// Set up the led and DTH11 
int pinDHT11 = 12;
SimpleDHT11 dht11(pinDHT11);
byte hum = 0;  //Stores humidity value
byte temp = 0; //Stores temperature value

int LEDs[5] = {16, 5, 4, 0, 2};

void setup() {
  for(int i=0; i<5; i++) {
    pinMode(LEDs[i], OUTPUT);
  }

  Serial.begin(115200);
  Serial.println(F("Adafruit IO Example"));
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // connect to adafruit io
  connect();
  Blynk.begin(auth, WLAN_SSID, WLAN_PASS);
  Blynk.setProperty(V1, "isDisabled", true);
}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}

BLYNK_WRITE(V1) {
  if(auto_tune == 0) {
    num_led = param.asInt();
    for(int i=0; i<5; i++) {
      if (i < num_led) {    
        digitalWrite(LEDs[i], HIGH);
      }
      else {
        digitalWrite(LEDs[i], LOW);
      }
    }
    Blynk.virtualWrite(V2, num_led);
  }
}

BLYNK_WRITE(V0) {
  auto_tune = param.asInt();
  if (auto_tune == 1) {
    Blynk.setProperty(V1, "isDisabled", true);
    Blynk.setProperty(V2, "isDisabled", false);
  } else {
    Blynk.setProperty(V1, "isDisabled", false);
    Blynk.setProperty(V2, "isDisabled", true);
  }
}


void loop() {
  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }
  dht11.read(&temp, &hum, NULL);
  Serial.print(temp); Serial.print(" *C, "); 
  Serial.print(hum); Serial.println(" H");
  if(temp_log.size() < 10) {
    temp_log.push_back((int)temp);    
  }
  else {
    temp_log.erase(temp_log.begin());
    temp_log.push_back((int)temp);    
  }
  if(hum_log.size() < 10) {
    hum_log.push_back((int)hum);    
  }
  else {
    hum_log.erase(hum_log.begin());
    hum_log.push_back((int)hum);    
  }
  tmp_temp = 0;
  for(int i = 0; i < temp_log.size(); i++) {
    tmp_temp += temp_log[i];
  }
  avg_temp = tmp_temp / (temp_log.size());
  tmp_temp = 0;
  for(int i = 0; i < hum_log.size(); i++) {
    tmp_temp += hum_log[i];
  }
  avg_hum = tmp_temp / (hum_log.size());
  Blynk.virtualWrite(V3, avg_temp);
  Blynk.virtualWrite(V4, avg_hum);
  if (auto_tune == 1) {
    if (avg_temp < 5.0) {
      num_led = 5;    
    }
    else if (avg_temp <= 10) {
      num_led = 4;
    }
    else if (avg_temp <= 15) {
      num_led = 3;
    }
    else if (avg_temp <= 20) {
      num_led = 2;
    }
    else if (avg_temp <= 25) {
      num_led = 1;
    }
    else {
      num_led = 0;
    }
    for(int i=0; i<5; i++) {
      if (i < num_led) {    
        digitalWrite(LEDs[i], HIGH);
      }
      else {
        digitalWrite(LEDs[i], LOW);
      }
    }
    Blynk.virtualWrite(V1, num_led);
    Blynk.virtualWrite(V2, num_led);
  }
  Blynk.run();
  
  delay(5000);
  
  if (! Log1.publish((String(avg_temp) + " " + String(avg_hum)).c_str())) {                     //Publish to Adafruit
    Serial.println(F("Failed"));
  } 
  else {
    Serial.println(F("Sent!"));
  }

}
