
#include "DHT.h"
#include "EspMQTTClient.h"

#define DHTPIN 4
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

int RELAY_PIN_1 = 27;
int RELAY_PIN_2 = 26;
int RELAY_PIN_3 = 25; 
int RELAY_PIN_4 = 33;
const int intPin = 18; //Cambiar nombre a SWITCH

int RELAY_PIN_1_STATE = 0;
int RELAY_PIN_2_STATE = 0;
int RELAY_PIN_3_STATE = 0;
int RELAY_PIN_4_STATE = 0;
int INPUT_STATE;
int int_payload;

int UPDATE_INTERVAL = 200;

const int timeThreshold = 150;

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

  digitalWrite(RELAY_PIN_1, !RELAY_PIN_1_STATE); 
  digitalWrite(RELAY_PIN_2, !RELAY_PIN_2_STATE); 
  digitalWrite(RELAY_PIN_3, !RELAY_PIN_3_STATE); 
  digitalWrite(RELAY_PIN_4, !RELAY_PIN_4_STATE); 
  
  Serial.begin(9600);
  dht.begin();
  
  attachInterrupt(digitalPinToInterrupt(intPin), debounceCount, CHANGE);
  client.enableDebuggingMessages();
}

void onConnectionEstablished()
{
   // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("home/garden-lights/1", [](const String & payload) {

    if (payload == "true") int_payload=1;
    else int_payload = 0;

    RELAY_PIN_1_STATE = !digitalRead(RELAY_PIN_1);
    if(RELAY_PIN_1_STATE != int_payload){
      SwitchRelay1(); }
  });

    client.subscribe("home/garden-lights/2", [](const String & payload) {

       if (payload == "true") int_payload=1;
       else int_payload = 0;

       RELAY_PIN_2_STATE = !digitalRead(RELAY_PIN_2);

      if(RELAY_PIN_2_STATE != int_payload){
      SwitchRelay2(); }
  });


    client.subscribe("home/garden-lights/3", [](const String & payload) {

       if (payload == "true") int_payload=1;
       else int_payload = 0;
       RELAY_PIN_3_STATE = !digitalRead(RELAY_PIN_3);
       if(RELAY_PIN_3_STATE != int_payload){
      SwitchRelay3(); }
  });


    client.subscribe("home/garden-lights/4", [](const String & payload) {

       if (payload == "true") int_payload=1;
       else int_payload = 0;
      RELAY_PIN_4_STATE = !digitalRead(RELAY_PIN_4);

      if(RELAY_PIN_4_STATE != int_payload){
      SwitchRelay4(); }
  });
}

void SwitchRelay1(){
  RELAY_PIN_1_STATE = !RELAY_PIN_1_STATE;
  digitalWrite(RELAY_PIN_1, !RELAY_PIN_1_STATE); 

  }

void SwitchRelay2(){
  RELAY_PIN_2_STATE = !RELAY_PIN_2_STATE;
  digitalWrite(RELAY_PIN_2, !RELAY_PIN_2_STATE); 
  
  }

void SwitchRelay3(){
  RELAY_PIN_3_STATE = !RELAY_PIN_3_STATE; 
  digitalWrite(RELAY_PIN_3, !RELAY_PIN_3_STATE);
  
  }

void SwitchRelay4(){
  RELAY_PIN_4_STATE = !RELAY_PIN_4_STATE;
  digitalWrite(RELAY_PIN_4, !RELAY_PIN_4_STATE); 
  
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
  if (now - lastMsg > UPDATE_INTERVAL) {
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

    if (RELAY_PIN_1_STATE == false)
    {
      IsFalse.toCharArray(msgR1, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR1, 6);  
    }

    if (RELAY_PIN_2_STATE == false)
    {
      IsFalse.toCharArray(msgR2, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR2, 6); 
    }

    if (RELAY_PIN_3_STATE == false)
    {
      IsFalse.toCharArray(msgR3, 6);
    }
    else
    {
      IsTrue.toCharArray(msgR3, 6); 
    }

    if (RELAY_PIN_4_STATE == false)
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

    Serial.print("Relay state 1:");
    Serial.print(INPUT_STATE);
}

void debounceCount()
{
  if (millis() - startTime > timeThreshold)
  {
    startTime = millis();
    return;
  }

  INPUT_STATE = digitalRead(intPin);

  if (INPUT_STATE) doRising();  
    else doFalling();
  }


void  doRising(){
    digitalWrite(RELAY_PIN_1, HIGH); 
    digitalWrite(RELAY_PIN_2, HIGH); 
    digitalWrite(RELAY_PIN_3, HIGH); 
    digitalWrite(RELAY_PIN_4, HIGH); 


    RELAY_PIN_1_STATE= false;
    RELAY_PIN_2_STATE= false;
    RELAY_PIN_3_STATE= false;
    RELAY_PIN_4_STATE= false;

    INPUT_STATE = 0;

    flagPublishRelayStates = true;
    return;
    };
    
  void doFalling(){
    digitalWrite(RELAY_PIN_1, LOW); 
    digitalWrite(RELAY_PIN_2, LOW); 
    digitalWrite(RELAY_PIN_3, LOW); 
    digitalWrite(RELAY_PIN_4, LOW); 

    RELAY_PIN_1_STATE= true;
    RELAY_PIN_2_STATE= true;
    RELAY_PIN_3_STATE= true;
    RELAY_PIN_4_STATE= true;

    INPUT_STATE = 1;

    flagPublishRelayStates = true;
    return;
    };
