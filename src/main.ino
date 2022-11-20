#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "config.h"

#define LED 2
#define MAX_MSG_LEN 32
#define MSG_TIMEOUT_MILLIS 100

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(1200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ICOM");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_SERVER, 1883);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT...");
    String clientId = "ICOM-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.publish("ICOM", "status=online");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

long lastSerial = 0;
int serialCount = 0;
String serialBuffer = "";

void loop()
{
  if (!client.connected())
  {
    digitalWrite(LED, LOW);
    reconnect();
    digitalWrite(LED, HIGH);
  }

  digitalWrite(LED, millis() % 1000 < 200 ? LOW : HIGH);
  client.loop();

  if (Serial.available() && serialCount < MAX_MSG_LEN)
  {
    lastSerial = millis();
    char hexBuf[3];
    sprintf(hexBuf, "%02X", Serial.read());
    serialBuffer += hexBuf;
    serialCount++;
  }

  if (millis() - lastSerial >= MSG_TIMEOUT_MILLIS && serialCount > 0)
  {
    client.publish("ICOM", ("msg=" + serialBuffer).c_str());
    serialCount = 0;
    serialBuffer = "";
  }
}