#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include <ArduinoJson.h>
#include "arduino_secrets.h"
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

#include "dust.h"

int dust_sensor=A1;

dust dust1(6, 7);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }
  mqttClient.poll();
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    char payload[512];
    getDeviceStatus(payload);
    sendMessage(payload);
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("$aws/things/DustDevice/shadow/update/delta");
}

void getDeviceStatus(char* payload) {
  //디바이스의 상태를 저장.
  float dustDensityug;
  dust1.getDvalue(dust_sensor);
  dustDensityug=dust1.getden();
  const char* gps = "37.52152328034671, 126.88951251249985"; //내 위치 정보
  const char* Mstate = (dust1.getMotor()==MOTOR_ON)? "ON" : "OFF";
  const char* state = (dust1.getState()==dustHIGH)? "BAD" : "GOOD";
  const char* autoM = (dust1.getAuto()==Auto_On)? "ON" : "OFF";
  sprintf(payload,"{\"state\":{\"reported\":{\"GPS\":\"%s\",\"Dustdensity\":\"%0.2f\",\"Auto\":\"%s\",\"Motor\":\"%s\",\"State\":\"%s\"}}}", gps, dustDensityug, autoM, Mstate, state);
}

void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/DustDevice/shadow/update";
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  mqttClient.beginMessage(TOPIC_NAME);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // store the message received to the buffer
  char buffer[512] ;
  int count=0;
  while (mqttClient.available()) {
     buffer[count++] = (char)mqttClient.read();
  }
  buffer[count]='\0'; // 버퍼의 마지막에 null 캐릭터 삽입
  Serial.println(buffer);
  Serial.println();
  
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, buffer);
  JsonObject root = doc.as<JsonObject>();
  JsonObject state = root["state"];
  const char* Mstate1 = state["Motor"];
  const char* Mstate2 = state["Auto"];
  Serial.println(Mstate1);
  Serial.println(Mstate2);
  
  char payload[512];
  
  if (strcmp(Mstate1,"ON")==0) {
    dust1.actionon();
    sprintf(payload,"{\"state\":{\"reported\":{\"Motor\":\"%s\"}}}","ON");
    sendMessage(payload);
  } else if (strcmp(Mstate1,"OFF")==0) {
    dust1.actionoff();
    sprintf(payload,"{\"state\":{\"reported\":{\"Motor\":\"%s\"}}}","OFF");
    sendMessage(payload);
  } else if (strcmp(Mstate2,"ON")==0) {
    dust1.actionAutoOn();
    sprintf(payload,"{\"state\":{\"reported\":{\"Auto\":\"%s\"}}}","ON");
    sendMessage(payload);
  } else if (strcmp(Mstate2,"OFF")==0) {
    dust1.actionAutoOff();
    sprintf(payload,"{\"state\":{\"reported\":{\"Auto\":\"%s\"}}}","OFF");
    sendMessage(payload);
  }
  
}
