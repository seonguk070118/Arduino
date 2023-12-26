#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 0

#define WIFI_SSID "bssm_free"
#define WIFI_PASSWORD "bssm_free"

#define API_KEY "AIzaSyBMgUKoZtZGuzwv0a2e09J4-e-ygDsbKdQ"

#define DATABASE_URL "hackaton-da1e4-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key; 

byte nuidPICC[4];

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
    
    for (byte i=0;i<=3;i++) {
      key.keyByte[i]=0xFF;
    }
    printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop()
{
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) return;
    for (byte i = 0; i<=3; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    printHex(rfid.uid.uidByte, rfid.uid.size);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
    delay(1000);
}

void printHex(byte *buffer, byte bufferSize) {
  FirebaseJson json;
  Firebase.RTDB.setInt(&fbdo, "CARD/1", buffer[0]);
  Firebase.RTDB.setInt(&fbdo, "CARD/2", buffer[1]);
  Firebase.RTDB.setInt(&fbdo, "CARD/3", buffer[2]);
  Firebase.RTDB.setInt(&fbdo, "CARD/4", buffer[3]);
}
