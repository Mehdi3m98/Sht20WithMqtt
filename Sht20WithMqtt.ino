#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DFRobot_SHT20.h>

#define lowTemp 10
#define HighTemp 40

const char* mqttServer = "";
const int mqttPort = 1883;
const char* ssid = "";
const char* password = "";
const char* Seri = "sht_20_001";
//const char* mqttUser = "TEMP1";
//const char* mqttPassword = "11228";
const char* commandTopic = "TEMP1/COM";
unsigned long LTime = 0;

WiFiClient wifiClient;
PubSubClient client(mqttServer, mqttPort, wifiClient);
DFRobot_SHT20 sht20;
float temp;
float humd;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  client.setCallback(callback);
  reconnect();
  sht20.initSHT20();
  sht20.checkSHT20();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  humd = sht20.readHumidity();
  temp = sht20.readTemperature();
  MqttTemp1();
}

void MqttTemp1() {
  unsigned long CTime = millis();
  if (CTime - LTime >= 10000) {
    if (sht20.readTemperature() > HighTemp )
    {
      client.publish("TEMP1/Alarm", "HIGH Temp");
      client.publish("TEMP1/TEMP", String(temp).c_str());
      client.publish("TEMP1/HUMD", String(humd).c_str());
    }
    if (sht20.readTemperature() < lowTemp)
    {
      client.publish("TEMP1/Alarm", "LOW Temp");
      client.publish("TEMP1/TEMP", String(temp).c_str());
      client.publish("TEMP1/HUMD", String(humd).c_str());
    }
    if (sht20.readTemperature() > lowTemp && sht20.readTemperature() < HighTemp)
    {
      client.publish("TEMP1/Alarm", "NORMAL TEMP");
      client.publish("TEMP1/TEMP", String(temp).c_str());
      client.publish("TEMP1/HUMD", String(humd).c_str());
    }
    LTime = CTime;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message received");
  String command = "";
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.println("Command: " + command);

  if(!strcmp(command.c_str(),"see")){
    String data ="TEMP is:" + String(temp) + "*" + "//" + "HUMD is:" + String(humd) + "%";
    client.publish(commandTopic, data.c_str());
    }
  if(!strcmp(command.c_str(),"serial")){
   // String Ser ="Serial is:" + String(Seri);
   // client.publish(commandTopic, Ser.c_str());
    client.publish(commandTopic, String(Seri).c_str());
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    //if (client.connect("ESP8266", mqttUser, mqttPassword))
    if (client.connect("ESP8266")) {
      Serial.println("Connected to MQTT");
      client.subscribe(commandTopic);
      client.subscribe("TEMP1/Alarm");
      client.subscribe("TEMP1/TEMP");
      client.subscribe("TEMP1/HUMD");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");   
    }
  }
 }
