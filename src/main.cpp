#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

uint8_t servoMotorpin = D2;
uint8_t selenoide = D0;
uint8_t magnetic = D1;

#define WIFI_SSID "H2H_GROUP"
#define WIFI_PASS "DejH.2025"
#define MQTT_SERVER "mqtt.h2h.name"
#define MQTT_USER "mqtt-h2box-001"
#define MQTT_PASSWORD "-XoektfR3XGE3WHo9NSzPW-AHnY3GFyyPp9GEL0p4Xg"
#define MQTT_TOPIC "messages/mqtt-h2box-001"
#define MQTT_SUBTOPIC "commands/mqtt-h2box-001"


char message[301];

WiFiClient espClient;
PubSubClient mqtt(espClient);
WiFiServer server(80);

void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("message arrived:");
  if((char)payload[1]=='o'){
    analogWrite(servoMotorpin,175);//open box
    digitalWrite(selenoide, LOW);
    Serial.println("Box unlock");
    mqtt.publish(MQTT_TOPIC,"Open");
  }else if((char)payload[1]=='c'){
    analogWrite(servoMotorpin,140);//cloe box
    digitalWrite(selenoide, HIGH);
    Serial.println("Box lock");
    mqtt.publish(MQTT_TOPIC,"Close");
  }
};
void pollSerial();

char chipId[8];

void setup() {
 Serial.begin(9600);
 Serial.println("Control Box");
 pinMode(magnetic,INPUT_PULLUP);
 pinMode(selenoide,OUTPUT);
 digitalWrite(selenoide, LOW);
 analogWriteFreq(50);//Set PWM freq 50Hz or 20ms
 analogWrite(servoMotorpin,140);//close box
 Serial.println();
 sprintf(chipId, "%08X", ESP.getChipId());
 Serial.print("CHIP ID:");
 Serial.println(chipId);
 Serial.print("Connecting to ");
 Serial.println(WIFI_SSID);
 WiFi.begin(WIFI_SSID,WIFI_PASS);
 while (WiFi.status() != WL_CONNECTED)
 {
   delay(500);
   Serial.print(".");
}
mqtt.setServer(MQTT_SERVER,1883);
mqtt.setCallback(callback);

}

int count = 100;

void reconnect_MQTT() {
  // Loop until we're reconnected

  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(chipId, MQTT_USER, MQTT_PASSWORD)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe(MQTT_SUBTOPIC); //
      mqtt.publish(MQTT_TOPIC, "Box online");
    }else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void reconnected_WiFi(){
  Serial.print("Connecting to... ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  delay(300);
}

void loop() {
  if(!mqtt.connected()){
    analogWrite(servoMotorpin,140);//close box
    digitalWrite(selenoide, LOW);
    reconnect_MQTT();
  }
  if(WiFi.status() == WL_CONNECT_FAILED){
    analogWrite(servoMotorpin,140);//cloe box
    reconnected_WiFi();
  }
  if(digitalRead(magnetic)==0){
    delay(500);
    Serial.println("Box open");
    mqtt.publish(MQTT_TOPIC, "Box open");
  }
  mqtt.loop();
  delay(200);


}
