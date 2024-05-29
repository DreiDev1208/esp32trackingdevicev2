#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include "ca_cert.h"

#define MQTT_BROKER     "d11986ae.ala.us-east-1.emqxsl.com"
#define MQTT_PORT       8883
#define MQTT_ID         "IotDevice"
#define MQTT_USERNAME   "IotDevice001"
#define MQTT_PASS       "andreimar123"
#define TOPIC           "gps/location"

#define WIFI_SSID       "#######"
#define WIFI_PASSWORD   "02172023"

TinyGPSPlus gps;
WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.println("MQTT message received");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

boolean mqttConnect() {
  Serial.println("Attempting MQTT connection...");
  wifiClient.setCACert(root_ca);
  boolean status = mqtt.connect(MQTT_ID, MQTT_USERNAME, MQTT_PASS);

  if (status == false) {
    Serial.println("MQTT connection failed");
    Serial.print("MQTT connection status: ");
    Serial.println(mqtt.state());
    return false;
  }

  Serial.println("MQTT connected successfully");
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 18, 19);
  delay(10);

  Serial.println("Attempting GPRS CONNECTION...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("GPRS connecting...");
  }

  Serial.println("GPRS connected successfully");
  Serial.println(WiFi.localIP());

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (!mqtt.connected()) {
    mqttConnect();
  }

  while (Serial2.available()) {
    char c = Serial2.read();
    gps.encode(c);
  }

  if (gps.location.isValid()) {
    Serial.println("GPS location is valid, publishing...");
    publishGPSLocation();
  }

  mqtt.loop();
}

void publishGPSLocation() {
  Serial.println("Publishing GPS location...");
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  String latitudeStr = String(gps.location.lat(), 6);
  String longitudeStr = String(gps.location.lng(), 6);

  if (mqtt.connected()) {
    if (mqtt.publish(TOPIC, (latitudeStr + "," + longitudeStr).c_str())) {
      Serial.println("GPS location published successfully");
    } else {
      Serial.println("Failed to publish GPS location");
    }
  } else {
    Serial.println("MQTT not connected, failed to publish GPS location");
  }
}

