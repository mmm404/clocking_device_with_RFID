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

//UTFTGLUE tft(0,A2,A1,A3,A4,A0); //all dummy args, *lib was not used due to no change in font size
MCUFRIEND_kbv tft;
#include <ArduinoJson.h>
#include <EEPROM.h>

#define MAX_UID_LENGTH 20    // Maximum length for RFID UID
#define MAX_ENTRIES 10      // Maximum number of entries to store
#define EEPROM_SIZE 4096   // Size of EEPROM in bytes

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

uint16_t g_identifier;

// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 4;

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
   tft.fillScreen(DARKCYAN);
   tft.fillRoundRect(10,70,300,80,5,DARKGREEN);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(WHITE);
   //tft.setTextSize(5);
   tft.println("ACCESS GRANTED");//displays the string
}
//Access  denied menu display
unsigned long denied() {
  //character();
   tft.fillScreen(DARKCYAN);
   tft.fillRoundRect(10,70,300,80,5,MAROON);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(WHITE);
   tft.println("ACCESS DENIED");//displays the string
}

void readtag()

{
  bool proceed = false;
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
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
     
   // String httpRequestData = "?userId="+scannedTag;

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
  character();
  tft.fillScreen(BLACK);
  pos = 7;
  tft.setCursor(60, 10);
  tft.println("Select Action :");
  for (uint16_t i=0; i<140; i++){
    tft.drawFastHLine(60+i, 30, 30, WHITE);
  }
  tft.setCursor(60, 50);
  tft.println("1. CLOCK IN");
  tft.setCursor(60, 100);
  tft.println("2. CLOCK OUT");
  tft.setCursor(60, 150);
  tft.println("3.  BACK");
  //Keypadvalue();
}

void sendtoESP(String data) {
    Serial1.println(data);      
    delay(1000);
    digitalWrite(blue, LOW);
}

String enrolltag()
{
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
    return scannedTag;
  }
}

unsigned long welcome(){
  pos = 0;
   //character();
   tft.fillScreen(BLACK);
   tft.fillRoundRect(60,85,200,40,10,WHITE); //draws a filled Rectangle at x,y,width,height,radius,colour
   //tft.drawRoundRect(93,128,303,53,10,DARKCYAN);//draws a rounded Rectangle at x,y,width,height,radius,colour
   tft.setCursor(100,100); //sets cursor at x,y position
   //tft.setTextSize(5);//text size 1-5
   tft.setTextColor(BLACK);
   tft.println("  LOADING ");
   displayLoadingAnimation();
   tft.fillScreen(BLACK);  
   tft.fillRoundRect(5,5,310,230,10,DARKCYAN);
   tft.drawRoundRect(5,5,310,230,10,WHITE);
   tft.setCursor(110,110);
   //tft.setTextSize(3);//text size 1-5
   // showmsgXY(170, 250, 2, &FreeSans9pt7b, "Loading...");
   tft.setTextColor(WHITE);
   tft.println("WELCOME!");//displays the string
}
//Menu for entering pin for the master tag
unsigned long masterTagMenu(){
  pos = 1;
  tft.fillScreen(BLACK);
  tft.setRotation(3);
  tft.setCursor(90,80);
  tft.setTextColor(WHITE);
  tft.println("Enter the pin");
  for (uint16_t i=0; i<150; i++){
    tft.drawFastHLine(90+i,140,10,WHITE);
  }
}
//Master tag Menu
unsigned long menu() {
  pos = 2;
  character();
  tft.fillScreen(BLACK);
  //tft.fillRoundRect(0,0,380,50,10,BLACK);
 // tft.drawRoundRect(0,0,380,50,10,WHITE);
  tft.setCursor(60, 10);
  tft.println("Select Option:");
  for (uint16_t i=0; i<140; i++){
    tft.drawFastHLine(60+i, 30, 30, WHITE);
  }
  tft.setCursor(60, 50);
  tft.println("1.Single Access");
  tft.setCursor(60, 100);
  tft.println("2.Multiple Access");
  tft.setCursor(60, 150);
  tft.println("3.Back");
  tft.setCursor(40, 200);
  tft.println("4. Enroll/Flush members");
  
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

// Function to find available memory block
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

bool confirmStoredEntries(String newEntry) {
  Serial.println("Checking stored Entries:");
  entryPar = false;
  for (int i = 0; i < MAX_ENTRIES; i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      // Read UID from EEPROM
      String rfidUid = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      
      // Print UID
      Serial.print("RFID UID: ");
      Serial.print(rfidUid);
      if(rfidUid == newEntry && rfidUid.length() > 2){
        entryPar = true;
      }
    } else {
      return false; // No more stored entries
    }
    return entryPar;
  }
}

void printStoredEntries() {
  Serial.println("Stored Entries:");
  for (int i = 0; i < MAX_ENTRIES; i++) {
    int addr = i * MAX_UID_LENGTH;
    if (!isMemoryBlockEmpty(addr, MAX_UID_LENGTH)) {
      // Read UID from EEPROM
      String rfidUid = readStringFromEEPROM(addr, MAX_UID_LENGTH);
      
      // Print UID
      //Serial.print("RFID UID: ");
      Serial.print("/");
      Serial.print(rfidUid);
    } else {
      break; // No more stored entries
    }
  }
}

void writeStringToEEPROM(int addr, String data, int maxLength) {
  for (unsigned int i = 0; i < data.length() && i < maxLength; i++) {
    EEPROM.write(addr + i, data[i]);
  }
  // Fill the rest of the space with null characters
  for (int i = data.length(); i < maxLength; i++) {
    EEPROM.write(addr + i, '\0');
  }
}

String readStringFromEEPROM(int addr, int maxLength) {
  String data = "";
  for (int i = 0; i < maxLength; i++) {
    char character = EEPROM.read(addr + i);
    if (character != '\0') {
      data += character;
    } else {
      break; // Stop reading if null character is encountered
    }
  }
  return data;
}

void displayLoadingAnimation() {
   int count = 0;

  // Variables for the rotating circle animation
  int centerX = 160; // X coordinate of the center of the circle
  int centerY = 200; // Y coordinate of the center of the circle
  int radius = 10;   // Radius of the circle
  int angle = 0;     // Current angle of rotation

  // Animation loop
  while (count < 500) {
    count++;
    // Draw the rotating circle
    int x = centerX + radius * cos(angle * PI / 180);
    int y = centerY + radius * sin(angle * PI / 180);
    tft.fillCircle(x, y, 4, WHITE); // Draw a white dot

    // Increment angle for the next frame
    angle += 5; // Adjust speed of rotation as needed

    // Clear the previous circle
    int prevX = centerX + radius * cos((angle - 10) * PI / 180);
    int prevY = centerY + radius * sin((angle - 10) * PI / 180);
    tft.fillCircle(prevX, prevY, 4, BLACK); // Erase previous dot

    // Delay for animation smoothness
    delay(10); // Adjust as needed
  }
}


bool storeUid(String rfidUid) {
  int availableAddr = findAvailableMemoryBlock();
  if (availableAddr != -1) {
    // Check if UID length exceeds maximum
    if (rfidUid.length() <= MAX_UID_LENGTH) {
      // Write UID to EEPROM
      writeStringToEEPROM(availableAddr, rfidUid, MAX_UID_LENGTH);
      Serial.println("Entry Stored successfully"); // Entry stored successfully
      return true;
    } else {
      Serial.println("RFID UID length exceeds maximum.");
      return false;
    }
  } else {
    Serial.println("No available memory block found.");
    return false;
  } 
}

void enroll(){
  bool feed;
  String tagname = "";
  tft.fillScreen(DARKCYAN);
  tft.fillRoundRect(5,70,310,80,5,BLACK);
  tft.drawRoundRect(5,70,310,80,5,WHITE);
  tft.setCursor(10,100); //sets cursor at x,y position
  tft.setTextColor(WHITE);
  tft.println("Tap Student ID to enroll");//displays the string
  tagname = enrolltag();
  Serial.println(tagname);
  if(tagname.length()>0){
    feed = storeUid(tagname);
  }
  if (feed){
    enroll_success();
    printStoredEntries();
    delay(1500);
  }
  enrollNew();
}

unsigned long enroll_success() {
  //character();
   tft.fillScreen(DARKCYAN);
   tft.fillRoundRect(10,70,300,80,5,DARKGREEN);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(WHITE);
   //tft.setTextSize(5);
   tft.println("MEMBER ENROLLED");//displays the string
}

void flush() {
  displayLoadingAnimation();
  clearEEPROM();
  printStoredEntries();
  flush_success();
  enrollNew();
}
unsigned long flush_success() {
  //character();
   tft.fillScreen(DARKCYAN);
   tft.fillRoundRect(10,70,300,80,5,DARKGREEN);
   tft.drawRoundRect(10,70,300,80,5,BLACK);
   tft.setCursor(80,100); //sets cursor at x,y position
   tft.setTextColor(WHITE);
   //tft.setTextSize(5);
   tft.println("FLUSH SUCCESS");//displays the string
   delay (2000);
}
void enrollNew(){
  pos = 6;
  character();
  tft.fillScreen(BLACK);
  tft.setCursor(60, 10);
  tft.println("Select Option:");
  for (uint16_t i=0; i<140; i++){
    tft.drawFastHLine(60+i, 30, 30, WHITE);
  }
  tft.setCursor(60, 50);
  tft.println("1. Enroll Member");
  tft.setCursor(60, 100);
  tft.println("2. Flush Members");
  tft.setCursor(60, 150);
  tft.println("3. Go Back ");
}

unsigned long stopMultiple(){
 pos = 4;
  character();
  
  //tft.println("Stop Multiple Access");
  tft.fillRoundRect(5,5,310,230,10,DARKCYAN);
  tft.drawRoundRect(5,5,310,230,10,WHITE);
  //tft.setTextSize(3);//text size 1-5
  // showmsgXY(170, 250, 2, &FreeSans9pt7b, "Loading...");
  tft.setTextColor(WHITE);
  tft.setCursor(70,110);
  tft.println("PRESS 1 TO STOP");
}


unsigned long idling()
{
   proceed = false; 
   scannedTag = ""; 
   //character();
   tft.fillScreen(DARKCYAN);
   tft.fillRoundRect(5,70,310,80,5,BLACK);
   tft.drawRoundRect(5,70,310,80,5,WHITE);
   tft.setCursor(10,100); //sets cursor at x,y position
   tft.setTextColor(WHITE);
    //tft.setTextSize(3);
   tft.println("Tap Student ID on sticker");//displays the string
   //loop();
   pos = 0;
}

void signalcheck()
{
 while(Serial1.available() >0) //Ensures data can be sent from the other uc, ESP32
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
void open_sequence()
{
  digitalWrite(green,HIGH);
  digitalWrite(buzzer,HIGH);
 
  delay(1200);

  digitalWrite(solenoidlock,LOW);
  digitalWrite(green,LOW);
  digitalWrite(buzzer,LOW);
  int solenoid =digitalRead(solenoidlock); //variable that stores the state of PNP pin to output to the relay/solenoid.
  Serial.print(solenoid);
  Serial.println("solenoidlockval");
   //variable that stores the state of contact sensor input pin
  access();
 
   while(solenoid == 0)
  {
    int contact = digitalRead(contactsensor);
    //int contact = digitalRead(contactsensor); //variable that stores the state of contact sensor input pin
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
  // Get key value if pressed
  customKey = customKeypad.getKey();
  if (customKey)
  {
    if(pos == 0)
    {
      loop();
    }
    if(pos == 1)
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
        else if(choice == '4'){
          enrollNew();
        }
        else if(choice == '3'){
          idling();
        }
        else
        {
          character();
          tft.fillRoundRect(5,5,310,230,10,DARKCYAN);
          tft.drawRoundRect(5,5,310,230,10,WHITE);
          tft.setCursor(10,110);
          tft.setTextColor(WHITE);
          tft.println("INVALID CHOICE,TRY AGAIN!");//displays the string
          delay(2000);
          menu();
        }
     }
        //To reset the password
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
        //To Stop Multiple Access
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
          flush();
        }
        else if(choice == '3'){
          menu();
        }
        else
        {
          character();
          tft.fillRoundRect(5,5,310,230,10,DARKCYAN);
          tft.drawRoundRect(5,5,310,230,10,WHITE);
          tft.setCursor(10,110);
          tft.setTextColor(WHITE);
          tft.println("INVALID CHOICE,TRY AGAIN!");
          delay(2000);
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
          character();
          tft.fillRoundRect(5,5,310,230,10,DARKCYAN);
          tft.drawRoundRect(5,5,310,230,10,WHITE);
          tft.setCursor(10,110);
          tft.setTextColor(WHITE);
          tft.println("INVALID CHOICE,TRY AGAIN!");
          delay(2000);
          bool proceed = true;
          idling();
          
          
        }
     }
    else
    {
      return;
    }
      }
}

