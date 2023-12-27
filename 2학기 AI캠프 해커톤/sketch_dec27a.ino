#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <SPI.h>
#include <MFRC522.h>
#include "DHT.h"

#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define WIFI_SSID "bssm_free"
#define WIFI_PASSWORD "bssm_free"

#define API_KEY "AIzaSyBnPJf9MC1VD5wuoRHPcMc95DzvPR9Impg"

#define DATABASE_URL "bssmfirst-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key; 

int cds = 34;
int pir = 32;
int pin = 4;
long duration;
long starttime;
long sampletime_ms=30000;
long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float ugm3 = 0;

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    Firebase.signUp(&config, &auth, "", "");
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    dht.begin();
    pinMode(pir,INPUT);

    pinMode(pin,INPUT);
    starttime = millis();
    Serial.println("Start");
}

void loop()
{
  
  FirebaseJson json;
  int cds1 = analogRead(cds);
  int t = dht.readTemperature();
  int val = digitalRead(pir);
  Serial.println(cds1);
  Serial.printf("Temperature : ");
  Serial.println(t);
  Serial.println(val);
  Firebase.RTDB.setInt(&fbdo, "CDS",cds1);
  Firebase.RTDB.setInt(&fbdo, "Temperature",t);
  Firebase.RTDB.setInt(&fbdo, "PIR",val);

  duration = pulseIn(pin,LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if((millis()-starttime)>sampletime_ms){
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    ugm3 = concentration*100/13000;
    Serial.print(ugm3);
    Serial.println("ug/m3");
    lowpulseoccupancy = 0;
    starttime = millis();
    Firebase.RTDB.setInt(&fbdo, "fine dust",ugm3);
  }
  delay(1000);
}
