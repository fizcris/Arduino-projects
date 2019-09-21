
#include "DHT.h"
#include "EspMQTTClient.h"

#define DHTPIN 4
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

int RELAY_PIN_1 = 27;
int RELAY_PIN_2 = 26;
int RELAY_PIN_3 = 25; 
int RELAY_PIN_4 = 33;
int RELAY_PIN_1_PREV_STATE;
int RELAY_PIN_2_PREV_STATE;
int RELAY_PIN_3_PREV_STATE;
int RELAY_PIN_4_PREV_STATE;
int RELAY_PIN_GEN_PREV_STATE = 0;

const int timeThreshold = 150;
const int intPin = 18; //Cambiar nombre a SWITCH
long startTime = 0;

float h;
float t;
long lastMsg = 0;
char msgT[20];
char msgH[20];
char msgR1[6];
char msgR2[6];
char msgR3[6];
char msgR4[6];

bool flagPublishRelayStates = false;

EspMQTTClient client
(
  "vodafone12FE_B",
  "CasaAlonso",
  "192.168.0.164",       // MQTT Broker server ip
  "MQTTUsername",        // Can be omitted if not needed
  "MQTTPassword",        // Can be omitted if not needed
  "ESP-garden-lights",   // Client name that uniquely identify your device
  1883                   // The MQTT port, default to 1883. this line can be omitted
);

void setup()
{
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  pinMode(intPin, INPUT_PULLUP);

  RELAY_PIN_1_PREV_STATE = digitalRead(RELAY_PIN_1);
  RELAY_PIN_2_PREV_STATE = digitalRead(RELAY_PIN_2);
  RELAY_PIN_3_PREV_STATE = digitalRead(RELAY_PIN_3);
  RELAY_PIN_4_PREV_STATE = digitalRead(RELAY_PIN_4);
  
  Serial.begin(9600);
  dht.begin();
  
  attachInterrupt(digitalPinToInterrupt(intPin), debounceCount, CHANGE);
  
  client.enableDebuggingMessages();
}

void onConnectionEstablished()
{
   // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("home/garden-lights/1", [](const String & payload) {
    Serial.println(payload);
    if(String(RELAY_PIN_1_PREV_STATE) != payload){
      SwitchRelay1(); }
  });

    client.subscribe("home/garden-lights/2", [](const String & payload) {
      if(String(RELAY_PIN_2_PREV_STATE) != payload){
      SwitchRelay2(); }
  });

    client.subscribe("home/garden-lights/3", [](const String & payload) {
      if(String(RELAY_PIN_3_PREV_STATE) != payload){
      SwitchRelay3(); }
  });

    client.subscribe("home/garden-lights/4", [](const String & payload) {
      if(String(RELAY_PIN_4_PREV_STATE) != payload){
      SwitchRelay4(); }
  });
}

void SwitchRelay1(){
  digitalWrite(RELAY_PIN_1, !RELAY_PIN_1_PREV_STATE); 
  }

void SwitchRelay2(){
  digitalWrite(RELAY_PIN_2, !RELAY_PIN_2_PREV_STATE); 
  }

void SwitchRelay3(){
  digitalWrite(RELAY_PIN_3, !RELAY_PIN_3_PREV_STATE); 
  }

void SwitchRelay4(){
  digitalWrite(RELAY_PIN_4, !RELAY_PIN_4_PREV_STATE); 
  }


void loop() 
{
  delay(2000);
 
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //Serial.print(F("Humidity: "));
  //Serial.print(h);
  //Serial.print(F("%  Temperature: "));
  //Serial.print(t);
  //Serial.print(F("°C "));

  client.loop();

  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    /* read DHT11/DHT22 sensor and convert to string */
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (!isnan(t)) {
      snprintf (msgT, 20, "%lf", t);
      snprintf (msgH, 20, "%lf", h);
      /* publish the message */
      client.publish("home/garden-lights/temperature", msgT);
      client.publish("home/garden-lights/humidity", msgH);
    }
  }

  if (flagPublishRelayStates)
  {
    String IsTrue = "true";
    String IsFalse = "false";

    if (RELAY_PIN_1_PREV_STATE == false)
    {
      IsFalse.toCharArray(msgR1, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR1, 6);  
    }

    if (RELAY_PIN_2_PREV_STATE == false)
    {
      IsFalse.toCharArray(msgR2, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR2, 6); 
    }

    if (RELAY_PIN_3_PREV_STATE == false)
    {
      IsFalse.toCharArray(msgR3, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR3, 6); 
    }

    if (RELAY_PIN_4_PREV_STATE == false)
    {
      IsFalse.toCharArray(msgR4, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR4, 6); 
    }
  
    client.publish("home/garden-lights/1", msgR1);
    client.publish("home/garden-lights/2", msgR2);
    client.publish("home/garden-lights/3", msgR3);
    client.publish("home/garden-lights/4", msgR4); 

    flagPublishRelayStates = false;   
    }
}

void debounceCount()
{
  if (millis() - startTime > timeThreshold)
  {
    startTime = millis();
  }

  digitalWrite(RELAY_PIN_1, !RELAY_PIN_GEN_PREV_STATE); 
  digitalWrite(RELAY_PIN_2, !RELAY_PIN_GEN_PREV_STATE); 
  digitalWrite(RELAY_PIN_3, !RELAY_PIN_GEN_PREV_STATE); 
  digitalWrite(RELAY_PIN_4, !RELAY_PIN_GEN_PREV_STATE); 

  RELAY_PIN_GEN_PREV_STATE = !RELAY_PIN_GEN_PREV_STATE;

  flagPublishRelayStates = true;
}
