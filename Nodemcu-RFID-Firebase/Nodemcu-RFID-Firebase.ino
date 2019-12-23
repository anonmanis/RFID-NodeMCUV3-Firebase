#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <FirebaseObject.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SS_PIN D4
#define RST_PIN D2
#define FIREBASE_HOST "presensiperkuliahan.firebaseio.com" 
#define FIREBASE_AUTH "GkbEpuP0EAElc49X2Saw2ojM08w9eDc3SEAkdMzI"
#define WIFI_SSID "anonmanis's Phone"
#define WIFI_PASSWORD "Irvan12345"

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;
boolean test;
//const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200,60000);

//method setup koneksi
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
  test = true;
  
  //connect to Firebase database
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

//method ambil data UID dari kartu atau tag keychain
int getid() {  
  if(!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  
  Serial.print("THE UID OF THE SCANNED CARD IS : ");
  
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str; //menyimpan data UID yang sudah diubah ke string pada variabel StrUID
  }
  Serial.println(StrUID);
  mfrc522.PICC_HaltA();
  return 1;
}

//method ubah array byte jadi string
void array_to_string(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

//method mengirim data UID ke firebase
void kirim(){
  getid();
 
  Firebase.setString("RFID", StrUID); // send to Firebase
  if (Firebase.failed()) { //if error/failed 
    Serial.print("kirim ke firebase failed:"); // print to serial 
    Serial.println(Firebase.error());  
    return;
  }
  delay(100);
}
//method ambil data json firebase
void terima() {
  getid();

  timeClient.update();
  int day = timeClient.getDay();
  //Serial.println(day);
  String myString = String(day);
  //Serial.println(myString);
  
  FirebaseObject object = Firebase.get("/RFIDMhs/" + StrUID );
  String Kode = object.getString("/NPM/");

  if (Kode != NULL){
    Serial.print("NPM : ");
    Serial.println(Kode);
    FirebaseObject object = Firebase.get("/Jadwal/" + Kode + "/" + myString );
    String pertemuan = object.getString();
    Serial.println(pertemuan);
    if (pertemuan != NULL){
      if (test == true ){
      String Hadir = "H";
      Firebase.setString("/PresensiMhs/" + Kode + "/Matakuliah/" + pertemuan + "/Pertemuan/" + myString + "/StatusPresensi/", Hadir );
      test = false;
      }
    } else {
      Serial.println("Jadwal Tidak ada");
    }
  } else {
    Serial.println("Data tidak ada");
    test = true;
  }
  
}

//method loop
void loop() {
    delay(100);
    terima();
    //kirim();
}
