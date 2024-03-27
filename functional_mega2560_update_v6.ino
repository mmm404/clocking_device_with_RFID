//RFID access control Device code

//Here's the code that goes to the Pro Mini:
/*The Access Control project was developed by A team
 *Future improvements include Embedded C and C++ as well as OTA
 *Program runs on Arduino Mega 2560 Pro Mini
 */

//Loading all the necessary libraries

#include <SPI.h>  //for SPI communication
#include <MFRC522.h> //for RFID reader
#include <MCUFRIEND_kbv.h> //for TFT 3.5" LCD with driver:ILI9486
//#include <UTFTGLUE.h> //use GLUE class and constructor
#include <Keypad.h> //for the 4x4 keypad

#include <StreamUtils.h>

//UTFTGLUE tft(0,A2,A1,A3,A4,A0); //all dummy args, *lib was not used due to no change in font size
MCUFRIEND_kbv tft;
#include <ArduinoJson.h>
#include <EEPROM.h>

#define MAX_UID_LENGTH 35    // Maximum length for RFID UID
#define MAX_ENTRIES 10      // Maximum number of entries to store
#define EEPROM_SIZE 4096   // Size of EEPROM in bytes
#define MAX_JSON_LENGTH 50
bool eepromCleared = false;
bool proceed;
#define RST_PIN  48  //reset pin on the RFID
#define SS_PIN   53  //slave select pin on the RFID

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

//Colours used for display on the TFT LCD
#define BLACK 0x0000
#define WHITE 0xFFFF
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define YELLOW 0xFFE0
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F

uint16_t c_color;
uint16_t g_identifier;

// Constants for row and column sizes
bool u_ind;
const byte ROWS = 4;
const byte COLS = 4;
String ind_UID = "";
String ind_UID1 = "";
String masterPin = "";
String masterPin1 = "";
String keypadPin = "9002";
String newPin = ""; //for password reset
String D_ata = "";
String rfidUid;
int red=21,green=23, blue=25; //RGB pins
int solenoidlock = 32;
int contactsensor = A14;
int buzzer = 20;
int cursorpos = 20;
char customKey;
int pos; //menus for display
char choice; //Menu displayed after Mastertag is scanned
String action;
String scannedTag = "" ; //To store value of the scanned tag
String masterTag = "39E1D456"; //A-UID for the Master Tag
String newEntry = "";
bool entryPar;
// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'*', '0', '#', 'D'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'1', '2', '3', 'A'},
};


byte rowPins[ROWS] = {A9,A8,A6,A7};  //connect to the row pinouts of the keypad
byte colPins[COLS] = {A11,A10,A12,A13}; //connect to the column pinouts of the keypad

// Create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);   //Initialize serial communications with the PC
  Serial.setTimeout(5000);
  Serial1.begin(115200); //Initialize serial communications with the other uc, ESP32
  //set the pins status
  pinMode(buzzer, OUTPUT);
  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(blue,OUTPUT);
  pinMode(contactsensor, INPUT);
  pinMode(solenoidlock, OUTPUT);
  //Common cathode RGB
  digitalWrite(green,LOW);
  digitalWrite(red,LOW);
  digitalWrite(blue,LOW);
  digitalWrite(solenoidlock,HIGH);
 
 while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  //mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  //Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  g_identifier = tft.readID(); //reads the driver of the TFT LCD
  Serial.print("ID = 0x");
  Serial.println(g_identifier, HEX);
  randomSeed(analogRead(0));
 
// Setup the LCD  
  tft.begin(g_identifier);
  tft.setRotation(3);
  tft.setTextSize(2);
//tft.setFont(BigFont);

  welcome(); //menus displayed on start-up
  delay(2000);
  idling(); //menu displayed while waiting for studentID (smart cards) to be scanned
}

void loop() {
    readtag();
    Keypadvalue();
    signalcheck();
}

unsigned long character(){

  tft.fillScreen(BLACK); // Black background on tft
  tft.setRotation(3); //to make text upright in landscape position
  tft.setTextColor(WHITE);
  tft.setCursor(0, 0); //sets cursor at x,y position
}

//Access granted menu display
unsigned long access () {
  //character();
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(DARKGREEN);
   //tft.setTextSize(5);
   tft.println("ACCESS GRANTED");//displays the string
}
//Access  denied menu display
unsigned long denied() {
  //character();
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(DARKGREEN);
   tft.println("ACCESS DENIED");//displays the string
}

void readtag()

{
  bool proceed = false;
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  digitalWrite(blue,HIGH); //Blue RGB LED lights to show that card has been scanned
 
 // Dump debug info about the card; PICC_HaltA() is automatically called
 // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

   //For loop for the array of values
   scannedTag = "";
    for(int i; i < mfrc522.uid.size; i++)
    {
      //Serial.print(mfrc522.uid.uidByte[i], HEX);
      scannedTag +=  String(mfrc522.uid.uidByte[i],HEX); // Adds the 4 bytes in a single String variable
      scannedTag.toUpperCase();
    }
    if(scannedTag.length() == 13)
    {
      scannedTag = "0"+scannedTag;
    }
     

    Serial.println(scannedTag);
    //checkClockAction();
    mfrc522.PICC_HaltA(); // Stop reading
    digitalWrite(blue,LOW);
    action = "";
    clock_menu();
   
}

void verify(){
    String httpRequestData = "{\"cardId\": \"" +scannedTag+ "\",\"action\": \"" +action+ "\"}"; //stores the cardId value as a Json file
    Serial.println(httpRequestData);
    sendtoESP(httpRequestData);
  
    if(scannedTag == masterTag)
    {
      //digitalWrite(blue,LOW);
      masterTagMenu();
    }
    else if(confirmStoredEntries(scannedTag) == true) 
    {
      open_sequence();
    }

    else{
       idling();
       loop();
    }
}

unsigned long clock_menu() {

  pos = 7;
  character();
  tft.fillScreen(DARKGREEN);
  //tft.fillRoundRect(5,5,310,300,10,ORANGE);
  //tft.drawRoundRect(5,5,310,300,10,WHITE);
  tft.fillRoundRect(40,30,240,180,10,YELLOW);
  tft.drawRoundRect(40,30,240,180,10,BLACK);
  
   
  tft.setTextColor(DARKGREEN);
  tft.setCursor(50, 40);
  tft.println("Select Clock Action:");
  for (uint16_t i=0; i<165; i++){
    tft.drawFastHLine(50+i, 60, 60, DARKGREEN);
  }
  
  tft.setTextColor(DARKGREEN);
  tft.setCursor(50, 80);
  tft.println("1. CLOCKING IN");
  tft.setCursor(50, 120);
  tft.println("2. CLOCKING OUT");
  tft.setCursor(50, 160);
  tft.println("3. BACK");
 
}

void toLowerCase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

void sendtoESP(String data) {
    Serial1.println(data);      
    delay(1000);
    digitalWrite(blue, LOW);
}

String enrolltag(bool isEnrollMode = true) {
  Serial.println("Scanning Tag...");
  
  while (true) {
    if (!mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      continue;
    }
    
    digitalWrite(blue, HIGH); //Blue RGB LED lights to show that card has been scanned
    scannedTag = "";
    for (int i = 0; i < mfrc522.uid.size; i++) {
      scannedTag += String(mfrc522.uid.uidByte[i], HEX); // Adds the 4 bytes in a single String variable
      scannedTag.toUpperCase();
    }
    
    if (scannedTag.length() == 13) {
      scannedTag = "0" + scannedTag;
    }
    digitalWrite(blue, LOW);
    Serial.print("Scanned a tag : ");
    Serial.println(scannedTag);

    if (isEnrollMode) {
      return scannedTag;
    } else {
      ind_UID1 = scannedTag;
      scanEEPROM(ind_UID1);
      ind_UID1 = "";
      //delUID;
    }
  }
}

unsigned long welcome(){
  pos = 0;
   //character();
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(80,90,140,40,10,YELLOW); //draws a filled Rectangle at x,y,width,height,radius,colour
   //tft.drawRoundRect(93,128,303,53,10,DARKCYAN);//draws a rounded Rectangle at x,y,width,height,radius,colour
   tft.setCursor(100,100); //sets cursor at x,y position
   //tft.setTextSize(5);//text size 1-5
   tft.setTextColor(DARKGREEN);
   tft.println("LOADING ");
   c_color = YELLOW;
   displayLoadingAnimation();
}


//Menu for entering pin for the master tag
unsigned long masterTagMenu(){
  pos = 1;
  tft.fillScreen(DARKGREEN);
  tft.setRotation(3);
  tft.setCursor(90,80);
  tft.setTextColor(DARKGREEN);
   
  tft.fillRoundRect(40,30,240,180,10,YELLOW);
  tft.drawRoundRect(40,30,240,180,10,BLACK);
  tft.println("Enter the pin");
  for (uint16_t i=0; i<150; i++){
    tft.drawFastHLine(90+i,140,10,DARKGREEN);
  }
}
//Master tag Menu
unsigned long menu() {
  pos = 2;
  character();
  tft.fillScreen(DARKGREEN);
  tft.setTextColor(DARKGREEN);
  //tft.fillRoundRect(0,0,380,50,10,BLACK);
 // tft.drawRoundRect(0,0,380,50,10,WHITE);
  tft.fillRoundRect(10,10,300,220,10,YELLOW);
  tft.drawRoundRect(10,10,300,220,10,BLACK);
  tft.setCursor(60, 15);
  tft.println("Select Option:");
  for (uint16_t i=0; i<140; i++){
    tft.drawFastHLine(60+i, 30, 30, DARKGREEN);
  }
  tft.setCursor(30, 50);
  tft.println("1. Single Access");
  tft.setCursor(30, 100);
  tft.println("2. Multiple Access");
  tft.setCursor(30, 150);
  tft.println("3. Enroll/Delete");
  tft.setCursor(30, 200);
  tft.println("4. Back");
  
}

unsigned long resetPassword(){
  pos = 3;
  character();
  tft.setCursor(60,80);
  tft.println("Enter New Password");
 for (uint16_t i=0; i<160; i++){
    tft.drawFastHLine(80+i, 140, 10,WHITE);
  }
//compare the two and accept if correct *remember to store this new password
}
unsigned long confirmPassword(){
  pos = 5;
  character();
  tft.setCursor(40, 80);
  tft.println("Confirm New Password");
  for (uint16_t i=0; i<160; i++){
    tft.drawFastHLine(80+i, 140, 10, WHITE);
  }
}

bool isMemoryBlockEmpty(int addr, int maxLength) {
  for (int i = 0; i < maxLength; i++) {
    if (EEPROM.read(addr + i) != '\0') {
      return false; 
    }
  }
  return true; 
}


int findAvailableMemoryBlock() {
  for (int addr = 0; addr < EEPROM_SIZE; addr += MAX_UID_LENGTH) {
    if (isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      return addr; 
    }
  }
  return -1; 
}

void clearEEPROM() {
  for (int addr = 0; addr < EEPROM_SIZE; addr++) {
    EEPROM.write(addr, '\0');
  }
}



bool matchUID(String inputString, String value) {
  DynamicJsonDocument jsonDoc(1024);

  int startPos = 0;
  int endPos = 0;

  while ((startPos = inputString.indexOf('{', endPos)) != -1) {
    endPos = inputString.indexOf('}', startPos);
    if (endPos == -1) {
      break;
    }
    String segment = inputString.substring(startPos, endPos + 1); 
    DynamicJsonDocument segmentDoc(128);
    deserializeJson(segmentDoc, segment);
    String uid = segmentDoc["UID"].as<String>();
    if (value.equals(uid)) {
      return true;
    }
  }
  return false; 
}


bool confirmStoredEntries(String newEntry) {
  //Serial.println("Checking stored Entries:");
  //printStoredEntries();
  bool entryPar = false;
  String rfidUid = "";
  String n_rfidUid = "";
  for (int i = 0; i < (getIndex()); i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      rfidUid = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      n_rfidUid += rfidUid; 
    }
  }
  Serial.println(n_rfidUid);
  return matchUID(n_rfidUid,newEntry);  
}

unsigned long getIndex() {
  int index = 0;  
  for (int i = 0; i < MAX_ENTRIES; i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      index++;
    }
  }

  Serial.println(index);
  return index;
}


void printStoredEntries() {
  Serial.println("Stored Entries:");
  for (int i = 0; i < (getIndex()); i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      String rfidUid = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      Serial.print(rfidUid);
    } else {
      break; 
    }
  }
}


void writeStringToEEPROM(int addr, String data, int maxLength) {
  
  EepromStream eepromStream(addr, maxLength);
  StaticJsonDocument<MAX_JSON_LENGTH> doc;
  doc["INDEX"] = getIndex();
  doc["UID"] = data;
 // doc["ADDR"] = addr;
  Serial.println(doc.as<String>());
  serializeJson(doc, eepromStream);
  for (unsigned int i = data.length(); i < maxLength; i++) {
    eepromStream.write('\0');
  }
}


String readStringFromEEPROM(int addr, int maxLength) {
  
  String data = "";
  EepromStream eepromStream(addr, maxLength);
  StaticJsonDocument<256> doc;
  deserializeJson(doc, eepromStream);

  for (int i = 0; i < maxLength; i++) {
    char character = EEPROM.read(addr + i);
    if (character != '\0') {
      data += character;
    } else {
      break; 
    }
  }
  return data;
}


void displayLoadingAnimation() {
  int count = 0;
  int centerX = 150; // X coordinate of the center of the circle
  int centerY = 120; // Y coordinate of the center of the circle
  int radius = 90;   // Radius of the circle
  int angle = 0;     // Current angle of rotation

  // Animation loop
  while (count < 365 ) {
    count++;
    int x = centerX + radius * cos(angle * PI / 180);
    int y = centerY + radius * sin(angle * PI / 180);
    tft.fillCircle(x, y, 4, BLACK); // Draw a white dot
    angle += 1; // Adjust speed of rotation as needed
    int prevX = centerX + radius * cos((angle - 10) * PI / 180);
    int prevY = centerY + radius * sin((angle - 10) * PI / 180);
    tft.fillCircle(prevX, prevY, 6, c_color); // Erase previous dot
    delay(25); // Adjust as needed
    if(count==200){
      c_color = WHITE;
      tft.fillRoundRect(80,90,140,40,10,YELLOW); //draws a filled Rectangle at x,y,width,height,radius,colour
      tft.setCursor(100,100); //sets cursor at x,y position
      tft.setTextColor(DARKGREEN);
      tft.println("WELCOME! ");
      delay(1500);
    }
  }
}


String fix_rfid_uid(String uid) {
  uid.replace(" ", "");
  return uid;
}

void correct_stored_entries(String entries[], int size) {
  for (int i = 0; i < size; i++) {
    entries[i] = fix_rfid_uid(entries[i]);
  }
}

int storeUid(String rfidUid) {
  Serial.println("Attempting to store Uid ...");
  int availableAddr = findAvailableMemoryBlock();
  Serial.print("Available Memory Block : ");
  Serial.println(availableAddr);
  int ind = getIndex();
  if (availableAddr != -1) {
    if (rfidUid.length() <= MAX_UID_LENGTH) {
      StaticJsonDocument<MAX_JSON_LENGTH> doc;
      doc["Index"] = ind;
      doc["UID"] = rfidUid;
      doc["ADDR"] = availableAddr; 
      String jsonString;
      serializeJson(doc, jsonString); 
      correct_stored_entries(&jsonString, 1);

      EepromStream eepromStream(availableAddr, MAX_JSON_LENGTH);
      serializeJson(doc, eepromStream);
      eepromStream.flush(); 
      return ind;

    } else {
      Serial.println("RFID UID length exceeds maximum.");
      return -1;
    }
  } else {
    Serial.println("No available memory block found.");
    return -1;
  }
}

void enroll(){
  int feed;
  String tagname = "";
  tft.fillScreen(DARKGREEN);  
  tft.fillRoundRect(5,70,310,80,5,YELLOW);
  tft.drawRoundRect(5,70,310,80,5,BLACK);
  tft.setCursor(10,100); //sets cursor at x,y position
  tft.setTextColor(DARKGREEN);
  tft.println("Tap Student ID to enroll");//displays the string
  tagname = enrolltag();
  Serial.println("Found a tag to enroll: ");
  Serial.println(tagname);
  if(tagname.length()>0){
    feed = storeUid(tagname);
  }
  if (feed != -1){
    enroll_success(feed);
    printStoredEntries();
    delay(1500);
  }
  else{
    Serial.println("Error with storing UID in EEPROM");
  }
  enrollNew();
}


unsigned long enroll_success(int memberId) {
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(DARKGREEN);
   tft.println(" ENROLLED AS ");
   tft.setCursor(240,100); 
   tft.setTextColor(MAROON);
   tft.print(memberId);
}

void flush() {
  flushingAnimation();
  clearEEPROM();
  printStoredEntries();
  flush_success();
  enrollNew();
}

void flushingAnimation(){
  tft.setCursor(80, 210);
  tft.setTextColor(MAROON); 
  tft.setTextSize(2); 
  tft.println("FLUSHING"); 
  int dotX1 = 190; 
  int dotY = 220; 
  for (int j = 0; j < 5; j++) { 
    for (int i = 0; i < 3; i++) {
      tft.fillCircle(dotX1 + i * 10, dotY, 4, MAROON);
      delay(200);
      tft.fillCircle(dotX1, dotY, 4, WHITE);
      tft.fillCircle(dotX1 + 10, dotY, 4, WHITE);
      tft.fillCircle(dotX1 + 20, dotY, 4, WHITE);
      delay(100);
    }
  }
}

void confirmFlush(){
  pos = 9;
  tft.fillScreen(DARKGREEN);
  tft.fillRoundRect(20,20,280,180,10,YELLOW );
  tft.drawRoundRect(20,20,280,180,10,BLACK);
  tft.setCursor(10,100); 
  tft.setTextColor(DARKGREEN);
  tft.setCursor(60, 50);
  tft.println(" CONFIRM FLUSH? ");
  tft.setCursor(100, 100);
  tft.println("1. Yes");
  tft.setCursor(100, 150);
  tft.println("2. No");

}
void delOption(){
  pos = 10;
  tft.fillScreen(DARKGREEN);
  tft.fillRoundRect(20,20,280,180,10,YELLOW );
  tft.drawRoundRect(20,20,280,180,10,BLACK);
  tft.setCursor(10,100); 
  tft.setTextColor(DARKGREEN);
  tft.setCursor(55, 50);
  tft.println(" DELETION OPTION ");
  tft.setCursor(45, 100);
  tft.println("1. Card Deletion");
  tft.setCursor(45, 150);
  tft.println("2. Index Deletion");

}

void delUID() {
  //String delTag = "";
  String ind_UID = "";

//  tft.setCursor(30, 140); //sets cursor at x,y position
//  tft.setTextColor(DARKGREEN);
//  tft.println("Tap card to delete");

  tft.fillScreen(DARKGREEN);
  tft.fillRoundRect(10, 70, 300, 80, 5, YELLOW);
  tft.drawRoundRect(10, 70, 300, 80, 5, BLACK);
  tft.setCursor(30, 100); 
  tft.setTextColor(DARKGREEN);
  tft.println("Tap Card to Delete");
  //enrolltag(false);
  ind_UID = enrolltag();
  if (ind_UID.length() > 5){
    scanEEPROM(ind_UID);
  } 
}


void delIndex() {
  String delTag = "";
  pos = 8;

  tft.fillScreen(DARKGREEN);
  tft.fillRoundRect(10, 50, 300, 150, 5, YELLOW);
  tft.drawRoundRect(10, 50, 300, 150, 5, BLACK);
  tft.setCursor(60, 80); 
  tft.setTextColor(DARKGREEN);
  tft.println(" Key-In to Delete");
  tft.setCursor(80, 130); 
  tft.setTextColor(DARKGREEN);
  tft.println(" 'D' TO STOP ");

}
  
int addrUID(String inputString, String value) {
  DynamicJsonDocument jsonDoc(1024);
  int startPos = 0;
  int endPos = 0;

  while ((startPos = inputString.indexOf('{', endPos)) != -1) {
    endPos = inputString.indexOf('}', startPos);
    if (endPos == -1) {
      break;
    }
    String segment = inputString.substring(startPos, endPos + 1); 
    DynamicJsonDocument segmentDoc(128);
    deserializeJson(segmentDoc, segment);
    String uid = segmentDoc["UID"].as<String>();
    if (value.equals(uid)) {
      int uidInd = segmentDoc["ADDR"].as<int>();
      return uidInd;
    }
  }
  return -1; 
}

int replaceAddr(String inputString) {
  DynamicJsonDocument jsonDoc(1024);
  int startPos = 0;
  int endPos = 0;

  while ((startPos = inputString.indexOf('{', endPos)) != -1) {
    endPos = inputString.indexOf('}', startPos);
    if (endPos == -1) {
      break;
    }
    String segment = inputString.substring(startPos, endPos + 1); 
    DynamicJsonDocument segmentDoc(128);
    deserializeJson(segmentDoc, segment);
    String uid = segmentDoc["UID"].as<String>();
    if (value.equals(uid)) {
      int uidInd = segmentDoc["ADDR"].as<int>();
      return uidInd;
    }
  }
  return -1; 
}


int addrIndex(String inputString, String value) {
  DynamicJsonDocument jsonDoc(1024);
  int startPos = 0;
  int endPos = 0;

  while ((startPos = inputString.indexOf('{', endPos)) != -1) {
    endPos = inputString.indexOf('}', startPos);
    if (endPos == -1) {
      break;
    }
    String segment = inputString.substring(startPos, endPos + 1); 
    DynamicJsonDocument segmentDoc(128);
    deserializeJson(segmentDoc, segment);
    String uid = segmentDoc["Index"].as<String>();
    if (value.equals(uid)) {
      int uidInd = segmentDoc["ADDR"].as<int>();
      return uidInd;
    }
  }
  return -1; 
}



void scanEEPROM(String entity) {
  String EEPROM_data = "";
  String EEPROM_segment = "";

  Serial.print("Attempting delete : ");
  Serial.println(entity);

  for (int i = 0; i < getIndex(); i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      EEPROM_segment = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      EEPROM_data += EEPROM_segment;
    }
  }

  int uidAddr = addrUID(EEPROM_data,entity); 
  
  Serial.print("uidAddr : ");
  Serial.println(uidAddr);

  if (uidAddr != -1) {
    EEPROM.write(uidAddr, 0xFF); 
    detailsDeleted();

    delay(1000);
  } else {
    detailsNotFound();
    delay(1000);
    
  }
  enrollNew();
}


void scanIndex(String U_Ind) {
  String Ind_data = "";
  String Ind_segment = "";

  Serial.print("Attempting delete : ");
  Serial.println(U_Ind);

  for (int i = 0; i < getIndex(); i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      Ind_segment = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      Ind_data += Ind_segment;
    }
  }
  
  int uidAddr = addrIndex(Ind_data,U_Ind); 
  
  Serial.print("uidAddr : ");
  Serial.println(uidAddr);

  if (uidAddr != -1) {
    EEPROM.write(uidAddr, 0xFF); 
    detailsDeleted();
    delay(1000);
  } else {
    detailsNotFound();
    delay(1000);
    
  }
  enrollNew();
}

unsigned long detailsDeleted() {
  tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(50,100); 
   tft.setTextColor(DARKGREEN);
   tft.println("DELETED SUCCESSFULLY");
   delay(1500);
   enrollNew();
}

//detailnotfound
unsigned long detailsNotFound() {
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); 
   tft.setTextColor(MAROON);
   tft.println("WRONG DETAILS");
}


unsigned long flush_success() {
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(10,70,300,80,5,YELLOW);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); 
   tft.setTextColor(DARKGREEN);
   //tft.setTextSize(5);
   tft.println("FLUSH SUCCESS");
   delay (2000);
}

void enrollNew(){
  pos = 6;
  tft.fillScreen(DARKGREEN);
  tft.setTextColor(DARKGREEN);
  //tft.fillRoundRect(0,0,380,50,10,BLACK);
 // tft.drawRoundRect(0,0,380,50,10,WHITE);
  tft.fillRoundRect(10,10,300,220,10,YELLOW);
  tft.drawRoundRect(10,10,300,220,10,BLACK);
  tft.setCursor(60, 15);
  tft.println("Select Option:");
  for (uint16_t i=0; i<140; i++){
    tft.drawFastHLine(60+i, 30, 30, DARKGREEN);
  }
  tft.setCursor(30, 50);
  tft.println("1. Enroll Member");
  tft.setCursor(30, 100);
  tft.println("2. Delete All Members");
  tft.setCursor(30, 150);
  tft.println("3. Delete Member");
  tft.setCursor(30, 200);
  tft.println("4. Back");
  ind_UID = "";
  ind_UID1 = "";
}

unsigned long stopMultiple(){
 pos = 4;
   
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(5,70,310,80,5,YELLOW );
   tft.drawRoundRect(5,70,310,80,5,BLACK);
   tft.setCursor(10,100); 
   tft.setTextColor(DARKGREEN);
    //tft.setTextSize(3);
   tft.println("   PRESS 1 TO PROCEED  ");
}

unsigned long idling()
{
   proceed = false; 
   scannedTag = ""; 
   //character();
   tft.fillScreen(DARKGREEN);
   tft.fillRoundRect(5,70,310,80,5,YELLOW );
   tft.drawRoundRect(5,70,310,80,5,BLACK);
   tft.setCursor(10,100); 
   tft.setTextColor(DARKGREEN);
    //tft.setTextSize(3);
   tft.println("Tap Student ID on sticker");
   //loop();
   //wifiNotConnected();
   pos = 0;
 
  }


void wifiConnected(){
  int centerX = 270; 
  int centerY = 45;
  int startAngle = 45;
  int endAngle = 135;
  int largestRadius = 35;
  int mediumRadius = 20;
  int smallestRadius = 5;
  int thickness = 3; 
  int dotRadius = 4; 

  for (int currentRadius = largestRadius; currentRadius >= smallestRadius - thickness; currentRadius--) {
    for (int angle = startAngle; angle <= endAngle; angle++) {
      float radians = angle * PI / 180;
      int x = centerX + currentRadius * cos(radians);
      int y = centerY - currentRadius * sin(radians);

      
      for (int i = 0; i < thickness; i++) {
        tft.drawPixel(x + i, y, YELLOW); 
      }
    }
  }

  int dotX = centerX ; 
  int dotY = centerY + 10;
  tft.fillCircle(dotX, dotY, dotRadius, YELLOW); 

  return 0; 
}

void wifiNotConnected(){
  int centerX = 270; 
  int centerY = 45; 
  int startAngle = 45;
  int endAngle = 135;

  int largestRadius = 35;
  int mediumRadius = 20;
  int smallestRadius = 5;
  int thickness = 3; 
  int dotRadius = 4; 

  for (int currentRadius = largestRadius; currentRadius >= smallestRadius - thickness; currentRadius--) {
    for (int angle = startAngle; angle <= endAngle; angle++) {
      float radians = angle * PI / 180;
      int x = centerX + currentRadius * cos(radians);
      int y = centerY - currentRadius * sin(radians);

      for (int i = 0; i < thickness; i++) {
        tft.drawPixel(x + i, y, YELLOW); // Adjust x for offset within thickness
      }
    }
  }
  int slashThickness = 6; 
  int slashStartX = centerX - largestRadius / 2; 
  int slashStartY = centerY;
  int slashEndX = centerX + largestRadius / 2;
  int slashEndY = centerY - largestRadius; 
  tft.drawLine(slashStartX, slashStartY, slashEndX, slashEndY, MAROON); 
for (int i = 0; i < slashThickness; i++) {
  tft.drawLine(slashStartX + i, slashStartY, slashEndX + i, slashEndY, MAROON); 
}

  int dotX = centerX ; 
  int dotY = centerY + 10; 
  tft.fillCircle(dotX, dotY, dotRadius, YELLOW); 

  return 0; 
}



void signalcheck()
{
 while(Serial1.available() >0) 
 {
  int serial_val = Serial1.parseInt();
  //int serial_val = Serial1.read();
  Serial.println(serial_val);
  if(serial_val == 1)
  {
    open_sequence();    
    Serial.print("YES = ");
    Serial.println(serial_val);
  }
  else if(serial_val == 2){
    denied_sequence();
    Serial.print("NO = ");
    Serial.println(serial_val);
    }
  else
  {
    idling();
    }
 }
}

void invalidChoice(){
  tft.fillScreen(DARKGREEN);
  tft.fillRoundRect(10,70,300,80,5,YELLOW);
  tft.drawRoundRect(10,70,300,80,5,BLACK);
  tft.setCursor(80,100);  
  tft.setTextColor(MAROON);
  tft.println("INVALID CHOICE!");
  delay(2000);
        
}

void open_sequence()
{
  digitalWrite(green,HIGH);
  digitalWrite(buzzer,HIGH);
 
  delay(1200);

  digitalWrite(solenoidlock,LOW);
  digitalWrite(green,LOW);
  digitalWrite(buzzer,LOW);
  int solenoid =digitalRead(solenoidlock); 
  Serial.print(solenoid);
  Serial.println("solenoidlockval");
  access();
 
   while(solenoid == 0)
  {
    int contact = digitalRead(contactsensor);
    if(contact == 0)
    {
    digitalWrite(solenoidlock,HIGH);
    solenoid = 1;
    }
    else {
     digitalWrite(solenoidlock,LOW);
    }
  }
  idling();
}

void denied_sequence()
{
  denied();
  digitalWrite(red,HIGH);
  for(int i=0; i<=5; i++){
   digitalWrite(buzzer,HIGH);
   delay(100);
   digitalWrite(buzzer,LOW);
   delay(100);
    }
  digitalWrite(red,LOW);
  idling();
}

void Keypadvalue()
{
  customKey = customKeypad.getKey();
  if (customKey)
  {
    if(pos == 0)
    {
      loop();
    }
    else if(pos == 1)
    {
      if (masterPin1.length() < 4)
      {
        cursorpos += 40;
        masterPin = String(customKey);
        masterPin1 +=  masterPin;
        tft.setCursor(40+cursorpos,120);
        tft.print("*");
        //Serial.println(masterPin1.length());
//        Serial.println(masterPin1);
          Serial.println(customKey);
      }
      if (masterPin1.length() == 4 )
      {
        if (masterPin1 == keypadPin)
        {
          access ();
          delay (800);
          menu();
        }
        else
        {
          denied_sequence();
          masterTagMenu();
        }
        cursorpos = 20;
      }
      if (masterPin1.length() > 3 )
      {
        masterPin1 = "";
      }
    }
    else if (pos == 2) {
      choice = (customKey);
      //Serial.println(choice);
      if(choice == '1'){
          open_sequence();
        }
        else if(choice == '2'){
          digitalWrite(green,HIGH);
          digitalWrite(buzzer,HIGH);
          delay(1200);
          digitalWrite(solenoidlock,LOW);
          digitalWrite(green,LOW);
          digitalWrite(buzzer,LOW);
          stopMultiple();
        }
        else if(choice == '3'){
          enrollNew();
        }
        else if(choice == '4'){
          idling();
        }
        else
        {
          invalidChoice();
          menu();
        }
     }
    else if(pos == 3){
      if (masterPin1.length() < 4)
      {
        cursorpos += 40;
   //       Serial.println(masterPin1.length());
        masterPin = String(customKey);
        masterPin1 +=  masterPin;
  //        Serial.println(masterPin1);
  //        Serial.println(customKey);
        tft.setCursor(40+cursorpos,120);
        tft.print("*");
 
        }
        if (masterPin1.length() == 4 )
        {
          //confirmPassword();
          cursorpos = 20;
         }
             
      if (masterPin1.length() > 3 )
      {
        masterPin1 = "";
      }
   }
       else if(pos == 4){
          choice = (customKey);
          if(choice == '1')
          {
          digitalWrite(solenoidlock,HIGH);
          idling();
          }
        }
        else if(pos == 5){
          if (masterPin1.length() < 4)
          {
          cursorpos += 40;
   //       Serial.println(masterPin1.length());
          masterPin = String(customKey);
          masterPin1 +=  masterPin;
  //        Serial.println(masterPin1);
  //        Serial.println(customKey);
          tft.setCursor(40+cursorpos,120);
          tft.print("*");
          }
        if (masterPin1.length() == 4 )
        {
          if ( keypadPin == masterPin1)
           {          
              //passwordReset();
              delay(3000);
              idling();
           }
          else
           {
            //notMatch();
            delay(3000);
            idling();
            }
          cursorpos = 20;
         }
             
      if (masterPin1.length() > 3 )
      {
        masterPin1 = "";
      }
   }
   else if (pos == 6) {
      choice = (customKey);
      if(choice == '1'){
          enroll();
        }
        else if(choice == '2'){
          confirmFlush();
          delay(1500);
        }
        else if(choice == '3'){
          //delUID();
          delOption();
        }
        else if(choice == '4'){
          menu();
        }
        else
        {
          invalidChoice(); 
          enrollNew();
        }
     }

     else if (pos == 10) {
      choice = (customKey);
      if(choice == '1'){
          delUID();
        }
        else if(choice == '2'){
          delIndex();
        }
        else
        {
          invalidChoice(); 
          enrollNew();
        }
     }
     else if (pos == 7) {
     // Serial.println("Keypad started");
      choice = (customKey);
      Serial.println(choice);
      if(choice == '1'){
          action = "IN";
          //bool proceed = true;
          verify();
        }
        else if(choice == '2'){
          action = "OUT";
          //bool proceed = true;
          verify();
        }
        else if(choice == '3'){
          //bool proceed = true;
          idling();
        }
        else
        {
          invalidChoice();
          bool proceed = true;
          idling();
          
          
        }
     }
    
    else if (pos == 8) {
      char quit = 'd';
      customKey = char(toLowerCase(customKey));
      Serial.println(customKey);
      
      if (customKey == quit) {
        Serial.println("--Done--");
        if (ind_UID1.length() > 0) {
          scanIndex(ind_UID1);
        } else {
          enrollNew(); 
        }
      }
      else if (customKey != quit) {
        ind_UID = String(customKey);
        ind_UID1 += ind_UID;
        tft.setCursor(125, 160); //sets cursor at x,y position
        tft.setTextColor(MAROON);
        tft.print(ind_UID1);
        if (ind_UID1.length() > 3) {
          scanIndex(ind_UID1);
        } 
      } 
      
    }

     else if (pos == 9) {
      choice = (customKey);
      if(choice == '1'){
          flush();
        }
      else if(choice == '2'){
        enrollNew();
      }
      else
      {
        enrollNew();
      }
    }
   
    else
    {
      return;
    }
  }
}

