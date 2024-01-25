//Here's the code that is uploaded to ESP32: (using endpoints from the Software Development Center)

/* This code runs on ESP32 to GET the student from the endpoint, ( after registering student: cardId with the REGno) after scanning the student Id */
//Libraries to allow access to WiFi,Use Http requests and Json files
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//Declare the pins used for Serial communication on ESP32
#define txd1 17
#define rxd1 16

//credentials for WiFi network
const char* ssid = "dekut";
const char* password = "dekut@ict2020?";
//const char* ssid = "MEDI";
//const char* password = "nonmerc!";

const char* serverName = "http://41.89.227.23:8000/api/student/entrycheck/"; //Server Endpoint to fetch resources from
const char* sendRegNo = "http://41.89.227.23:8000//api/student/addstudentaccess"; //

String httpRequestData;
const char* cardIdValue;
String response;
String RegNo;
int PostCode;
int httpResponseCode;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200); //To allow use of Serial monitor
  Serial2.begin(115200, SERIAL_8N1, rxd1, txd1); //Use of multiple Serial, for communication between the two microcontrollers.
  WiFi.disconnect();  //disconnects wifi from previous network
  WiFi.begin(ssid, password); //To connect ESP32 to the WiFi Network
  WiFi.mode(WIFI_STA); //sets the Wifi module to station mode - devices are connecting to it
  Serial.println("Connecting to wifi");

  //Buffer, waiting for wifi to connect
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n Connected to a wifi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); //Fetches then shows the IP address of the wifi network
}

void loop()
{
  if (Serial2.available()) //Ensures data can be sent from the other microcontroller
  {
    httpRequestData = Serial2.readString(); //Reads the data (cardId) from the Pro Mini uc
    delay(1000);
    StaticJsonDocument<192> docobj; //Creates a json object in memory
    DeserializationError error = deserializeJson(docobj, httpRequestData); //fetches the data from the memory, if successful
    cardIdValue = docobj["cardId"]; //stores the fetched data in the var as a String - array of char
    Serial.print("Test test : ");
    Serial.println(cardIdValue);
    checkstudent();
  }
}

void checkstudent() {

  Serial.println("Get Auth"); //to know that this function is running

  //Check WiFi connection status - pre-check to confirm we are still connected to the WiFi network, before trying to do the request
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http; //declaring an object of the class, HTTPClient to allow use of the methods needed to perform the HTTP request.
    //HTTP request with a content type: application/json, of the body of our request, so the server knows how to interpret it.
    char getStudent[128]; //A string is an array of characters
    strcpy(getStudent, serverName); //make the var local from the global url
    strcat(getStudent, cardIdValue);  // Append the cardId to the url endpoint;will always refresh the concat since it's a local var
    http.begin(getStudent);  //Specify destination to send HTTP request - server endpoint
    httpResponseCode = http.GET(); //Send GET request to server
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) //If request was successful
    {
      String response = http.getString(); //Get the response to the request - data in the endpoint
      Serial.println(httpResponseCode);
      Serial.println(response);

      // const char* input;
      //size_t inputLength; //(optional)

      //Use ArduinoJsonAssistant to help deserealize (fetch data) json file (response), stores in the memory(access)
      StaticJsonDocument<192> access;

      DeserializationError error = deserializeJson(access, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      const char* FullName = access["FullName"]; // "MATHENGE JOSEPH MAINA"
      const char* Reg_no = access["Reg_no"]; // "E022-01-0281/2018"
      const char* Status = access["Status"]; // "Current"
      bool Allow_entry = access["Allow_entry"]; // true
      Serial.println(FullName);
      Serial.println(Reg_no);
      Serial.println(Status);
      Serial.println(Allow_entry);

       RegNo = String(Reg_no);

        HTTPClient http;
        http.begin(sendRegNo);
        http.addHeader("Content-Type", "application/json");
        String Student = "{\"Reg_no\": \"" +RegNo+ "\"}";
        PostCode = http.POST(Student);
        Serial.println(sendRegNo);
        Serial.println(Student);
        Serial.println(PostCode);
       
        if(PostCode>0){
        String payload = http.getString();  //Get the response to the request
        //Serial.println(PostCode);   //Print return code
        Serial.println(payload);           //Print request answer
        }
      else{
        Serial.print("Error on sending POST: ");
        Serial.println(PostCode);
        }


      if (Allow_entry == 1)
      {
        Serial2.println("1");
        Serial.println("ACCESS GRANTED");
        //PostCode = http.POST(Student);

      }
      /*else if (httpResponseCode == 500)
        //User not registered
      {
        Serial2.println("2");
        Serial.println("ACCESS DENIED");
      }*/

      else
      {
        Serial2.println("2");
        Serial.println("ACCESS DENIED");
      }
     

      http.end();  //Free resources

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
    }
    else //If internal error occurs when trying to send the request to the server, print the error code to help debugging.
    {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }

  }
  else
  {
    Serial.println("Error in WiFi connection");
  }
}


