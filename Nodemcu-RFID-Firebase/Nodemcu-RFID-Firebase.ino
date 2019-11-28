#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define SS_PIN D4
#define RST_PIN D2
#define FIREBASE_HOST "HOST FIREBASE" 
#define FIREBASE_AUTH "TOKEN"
#define WIFI_SSID "HYP_Spot"
#define WIFI_PASSWORD "rumahcipageran"

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
void setup() {
   Serial.begin(9600);
   SPI.begin();       // Init SPI bus
   mfrc522.PCD_Init(); // Init MFRC522
   Serial.println("RFID reading UID");

   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   Serial.print("connecting");
   while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  //connect to Firebase database
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

String Cardid() {
  if ( mfrc522.PICC_IsNewCardPresent())
    {
        if ( mfrc522.PICC_ReadCardSerial())
        {
           Serial.print("Tag UID:");
           for (byte i = 0; i < mfrc522.uid.size; i++) {
                  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                  Serial.print(mfrc522.uid.uidByte[i], HEX);
            }
         
            Serial.println();
            mfrc522.PICC_HaltA();
        }
    }
}

void kirim(){
  Firebase.setString("RFID_MHS/1174043/RFID", Cardid()); // send to Firebase
  if (Firebase.failed()) { //if error/failed 
    Serial.print("kirim ke firebase failed:"); // print to serial 
    Serial.println(Firebase.error());  
    return;
  }
  delay(100);
}

void loop() {
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    kirim();
}
