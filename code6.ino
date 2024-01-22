#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
String UIDval; // Declare UIDval as a string

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Clear UIDval before reading a new card
  UIDval = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UIDval += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    UIDval += String(mfrc522.uid.uidByte[i], HEX);
  }
  

  Serial.println("Card UID: " + UIDval);
}

//finish on this section
uint_16_t storeVal(UIDval){
  if(UIDval != ''){
    UIDval2 = UIDval;
  }
  else{
    return 
  }
}