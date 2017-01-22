#include <SoftwareSerial.h> 
#include <SparkFunESP8266WiFi.h>

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined(ARDUINO_ARCH_SAMD)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6

char user_input;
int x;
int y;
int state;


//////////////////////////////
// WiFi Network Definitions //
//////////////////////////////

const char mySSID[] = "AirPennNet-Guest";
ESP8266Server server = ESP8266Server(80);

#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//////////////////
// HTTP Strings //
//////////////////
const char destServer[] = "example.com";
const String htmlHeader = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Connection: close\r\n\r\n"
                          "<!DOCTYPE HTML>\r\n"
                          "<html>\r\n";

const String httpRequest = "GET / HTTP/1.1\n"
                           "Host: example.com\n"
                           "Connection: close\n\n";

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void setup(void){

  delay(500);

  Serial.begin(9600);
  
  serialTrigger(F("Press any key to begin."));
  
  // initializeESP8266() verifies communication with the WiFi
  // shield, and sets it up.
  initializeESP8266();

  
  serialTrigger(F("Press any key to begin."));
  // connectESP8266() connects to the defined WiFi network.
  connectESP8266();

  // displayConnectInfo prints the Shield's local IP
  // and the network it's connected to.
  displayConnectInfo();

  //serialTrigger(F("Press any key to connect client."));
  //clientDemo();
  
  serialTrigger(F("Press any key to test server."));
  serverSetup();

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);

  resetEDPins();

//  serialTrigger(F("Press any key to set Bluetooth."));
//  if ( !ble.begin(VERBOSE_MODE)){
//    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
//  }
//  ble.echo(false);
//  ble.info();
  
}

void loop(void){
//  int unlockcount = 0;
//  if( ble.waitForOK() && unlockcount == 0)
//  {
//    StepForwardDefault();
//    unlockcount = 1;
//    
//  }else if((!ble.waitForOK()) && unlockcount == 1)
//  {
//    ReverseStepDefault();
//    unlockcount = 0;
//  }
//  resetEDPins();
  digitalWrite(EN, LOW);
  serverDemo();
  resetEDPins();
}

void initializeESP8266()
{
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  // true
  int test = esp8266.begin();
  if (test != true)
  {
    Serial.println(F("Error talking to ESP8266."));
    errorLoop(test);
  }
  Serial.println(F("ESP8266 Shield Present"));
}

void connectESP8266()
{
  // The ESP8266 can be set to one of three modes:
  //  1 - ESP8266_MODE_STA - Station only
  //  2 - ESP8266_MODE_AP - Access point only
  //  3 - ESP8266_MODE_STAAP - Station/AP combo
  // Use esp8266.getMode() to check which mode it's in:
  int retVal = esp8266.getMode();
  if (retVal != ESP8266_MODE_STA)
  { // If it's not in station mode.
    // Use esp8266.setMode([mode]) to set it to a specified
    // mode.
    retVal = esp8266.setMode(ESP8266_MODE_STA);
    if (retVal < 0)
    {
      Serial.println(F("Error setting mode."));
      errorLoop(retVal);
    }
  }
  Serial.println(F("Mode set to station"));

  // esp8266.status() indicates the ESP8266's WiFi connect
  // status.
  // A return value of 1 indicates the device is already
  // connected. 0 indicates disconnected. (Negative values
  // equate to communication errors.)
  retVal = esp8266.status();
  if (retVal <= 0)
  {
    Serial.print(F("Connecting to "));
    Serial.println(mySSID);
    // esp8266.connect([ssid], [psk]) connects the ESP8266
    // to a network.
    // On success the connect function returns a value >0
    // On fail, the function will either return:
    //  -1: TIMEOUT - The library has a set 30s timeout
    //  -3: FAIL - Couldn't connect to network.
    retVal = esp8266.connect(mySSID);
    if (retVal < 0)
    {
      Serial.println(F("Error connecting"));
      errorLoop(retVal);
    }
  }
}

void displayConnectInfo()
{
  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  char mac[17];
  // esp8266.getAP() can be used to check which AP the
  // ESP8266 is connected to. It returns an error code.
  // The connected AP is returned by reference as a parameter.
  int retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print(F("Connected to: "));
    Serial.println(connectedSSID);
    esp8266.localMAC(mac);
    int x = 0;
    for(x = 0;x<17;x++){
      Serial.print(mac[x]);
    }
    Serial.print('\n');
  }

  // esp8266.localIP returns an IPAddress variable with the
  // ESP8266's current local IP address.
  IPAddress myIP = esp8266.localIP();
  Serial.print(F("My IP: ")); Serial.println(myIP);
}
//
//void clientDemo()
//{
//  // To use the ESP8266 as a TCP client, use the 
//  // ESP8266Client class. First, create an object:
//  ESP8266Client client;
//
//  // ESP8266Client connect([server], [port]) is used to 
//  // connect to a server (const char * or IPAddress) on
//  // a specified port.
//  // Returns: 1 on success, 2 on already connected,
//  // negative on fail (-1=TIMEOUT, -3=FAIL).
//  int retVal = client.connect(destServer, 80);
//  if (retVal <= 0)
//  {
//    Serial.println(F("Failed to connect to server."));
//    return;
//  }
//
//  // print and write can be used to send data to a connected
//  // client connection.
//  client.print(httpRequest);
//
//  // available() will return the number of characters
//  // currently in the receive buffer.
//  while (client.available())
//    Serial.write(client.read()); // read() gets the FIFO char
//  
//  // connected() is a boolean return value - 1 if the 
//  // connection is active, 0 if it's closed.
//  if (client.connected())
//    client.stop(); // stop() closes a TCP connection.
//}

void serverSetup()
{
  // begin initializes a ESP8266Server object. It will
  // start a server on the port specified in the object's
  // constructor (in global area)
  server.begin();
  Serial.print(F("Server started! Go to "));
  Serial.println(esp8266.localIP());
  Serial.println();
}

void serialTrigger(String message)
{
  Serial.println();
  Serial.println(message);
  Serial.println();
  while (!Serial.available())
    ;
  while (Serial.available())
    Serial.read();
}

void serverDemo()
{
  // available() is an ESP8266Server function which will
  // return an ESP8266Client object for printing and reading.
  // available() has one parameter -- a timeout value. This
  // is the number of milliseconds the function waits,
  // checking for a connection.
  ESP8266Client client = server.available(500);
  
  if (client) 
  {
    Serial.println(F("Client Connected!"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply

        
        if (c == '@'){      // lock statement
          
          Serial.println(F("Sending HTML page"));
          // send a standard http response header:
          client.print(htmlHeader);
          String htmlBody;
          htmlBody = "I hate this";
          client.print(htmlBody);
          Serial.println("Lock request received.");
          ReverseStepDefault(); 
          break;
        }
        if (c == '!'){      // unlock statement
          
          Serial.println(F("Sending HTML page"));
          // send a standard http response header:
          client.print(htmlHeader);
          String htmlBody;
          htmlBody = "I hate this";
          client.print(htmlBody);
          Serial.println("Unlock request received.");
          StepForwardDefault();
          break;
        }
       
        if (c == '\n') 
        {
          currentLineIsBlank = true;
          
        }
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
   
    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }
  
}

void StepForwardDefault()
{
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  for(x = 0; x<1400; x++){
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}
void ReverseStepDefault()
{
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  for(x = 0; x<1400; x++){
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

void errorLoop(int error)
{
  Serial.print(F("Error: ")); Serial.println(error);
  Serial.println(F("Looping forever."));
  for (;;)
    ;
}

