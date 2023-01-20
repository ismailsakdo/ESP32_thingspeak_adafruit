#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <SHT21.h> 
#include "config.h"
#define LED_PIN 5


char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;
SHT21 sht; 

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

float temp; 	// variable to store temperature
float humidity; // variable to store hemidity

//setup 'digital' feed
AdafruitIO_Feed *digital = io.feed("digital");

void setup() {
  Wire.begin();
  Serial.begin(115200);  //Initialize serial
  pinMode(LED_PIN, OUTPUT);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  digital->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  digital->get();
}

void loop() {
  Thingspeak();
  io.run();
}

void Thingspeak(){

  temp = sht.getTemperature();  // get temp from SHT 
  humidity = sht.getHumidity(); // get temp from SHT

  Serial.print("Temp: ");			// print readings
  Serial.print(temp);
  Serial.print("\t Humidity: ");
  Serial.println(humidity);

  delay(85);	// min delay for 14bit temp reading is 85ms

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  // set the fields with the values
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humidity);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(15000); // Wait 20 seconds to update the channel again
}

void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");

  if(data->toPinLevel() != HIGH) // change this either '==' or '!='
    Serial.println("LOW");
  else
    Serial.println("HIGH");


  digitalWrite(LED_PIN, data->toPinLevel());
}