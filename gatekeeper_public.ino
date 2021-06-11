//Code by @teltorob 

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <WiFiClientSecure.h>
#include "AsyncTelegram.h"
AsyncTelegram myBot;

const char* ssid     = "Home";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "hydrogengas";     // The password of the Wi-Fi network
const char token[] = "1712489194:AAF2qCxOPkxvdeZjOn7CAxsvxe0918P0_NI";

#define SS_PIN 2  //D2
#define RST_PIN 0 //D1

#include <SPI.h>
#include <MFRC522.h>

const int motorPin1= D0;
const int motorPin2=D1;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

byte check = 0;

boolean isOpen=false;

struct IdCard
{
    char const *UID;
    char const *name;
    int  const chatId;
};

const int cards= 2;//The number of users you want to add

//Struct to define users along with their UID and Chat ID
//STL map implementation proves to be costly 
IdCard const identity[cards] =
{
    { "UID1", "NAME1",TELEGRAM CHAT ID 1 },
    { "UID2", "NAME2",TELEGRAM CHAT ID 2 },
};//You can add users separated by commas(,)

void setup() {
  
Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  WiFi.setAutoConnect(true);   
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
  delay(2000);

 // Initiate SPUI and MFRC522
 
   SPI.begin();      // Initiate  SPI bus
   delay(100);
   mfrc522.PCD_Init();   // Initiate MFRC522

 // Setup motors
   pinMode(motorPin1, OUTPUT);
   pinMode(motorPin2, OUTPUT);

 // To ensure certificate validation, WiFiClientSecure needs time upadated
    myBot.setInsecure(true);
    myBot.setClock("CET-1CEST,M3.5.0,M10.5.0/3"); //Set time zone (Used stock time zone)
  
    myBot.setUpdateTime(2000);
    myBot.setTelegramToken(token);
    
    // Check if all things are ok
    Serial.print("\nTest Telegram connection... ");
    myBot.begin() ? Serial.println("OK") : Serial.println("NOK");
    
    Serial.print("Bot name: @");  
    Serial.println(myBot.userName);
    
}

 
void loop() {
      static uint32_t ledTime = millis();
  if (millis() - ledTime > 150) {
    ledTime = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }


 // a variable to store telegram message data
 TBMessage msg;
  if (myBot.getNewMessage(msg)){
    String message= msg.text;
    Serial.println(msg.sender.id);
    for(byte i=0; i< cards;i++)
    {
      
      if ((identity[i].chatId)== msg.sender.id)
      {

        if (((message)=="/open") && (!isOpen)) check=1;
        else if (((message)=="/close") && (isOpen)) check=1;
        else 
        myBot.sendMessage(msg, "Enter valid option");
        delay(10000); //To avoid for simultaneous operation
        motor();
        break;
      }
    }
    }
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  
  //Show UID on serial monitor
  Serial.println("reached here");
  delay(800);
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;

  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();

  String uid=content.substring(1); //Stores the UID of detected card

  for(byte i=0; i< cards;i++)
  {
    if (String(identity[i].UID)== uid)
    {
      greet(i);
      notify(i);
      check=1;
      break;
    }
  }

motor();
}

// Greets the user on Serial Monitor
void greet(int index)
{
  Serial.print("Welcome home ");
  Serial.println(identity[index].name);
}

//Notifies the user on telegram
void notify(int index)
{
  String visitor= String(identity[index].name);
  String  notification = visitor + " entered home.";
  int32_t userId = identity[index].chatId;  
  myBot.sendToUser(userId, notification); 
  Serial.print("Notification sent to: ");
  Serial.println(visitor);
}

//Operates motor
void motor()
{

 if(check==1)
 {
   if (isOpen)
   {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);// Clockwise Rotation
    delay(5000);  
    digitalWrite(motorPin1, LOW);
    isOpen = false;   
   }

   else
   {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);// Anti-Clockwise Rotation
    delay(5000);  
    digitalWrite(motorPin2, LOW);
    isOpen = true;
   }
   }
   

}
