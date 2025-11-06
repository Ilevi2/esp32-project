#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// === Konfigurasi WiFi ===
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// === Konfigurasi MQTT Broker ===
const char* mqtt_server = "test.mosquitto.org";
const char* topicPub = "esp32/dht22/data";
const char* topicSub = "esp32/led/control";

// === Inisialisasi objek ===
WiFiClient espClient;
PubSubClient client(espClient);

// === Konfigurasi DHT22 ===
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === Pin LED ===
#define LED_PIN 5

// === Koneksi WiFi ===
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// === Callback jika ada pesan MQTT masuk ===
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Kontrol LED dari MQTT command
  if (message == "ON") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED is ON");
  } else if (message == "OFF") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED is OFF");
  }
}

// === Reconnect ke MQTT broker jika terputus ===
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topicSub);
      Serial.println("Subscribed to control topic!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// === Setup awal ===
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  dht.begin();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// === Loop utama ===
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca data sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    char payload[50];
    snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", t, h);
    client.publish(topicPub, payload);
    Serial.print("Published data: ");
    Serial.println(payload);
  }

  delay(5000); // publish setiap 5 detik
}
