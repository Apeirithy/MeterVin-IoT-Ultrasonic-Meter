#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

//Declaration for ultrasonic
const int trigPin = 14;
const int echoPin = 13;

//Declaration for buttons
const int functionBtn = 27;
const int selBtn = 26;

//button state will be high if not pressed, and will be low if pressed
#define pressed LOW
#define notPressed HIGH

//LCD object creation
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Variables for LCD (All UI function use this)
char lcdBuffer[20];

//Variables for a little delay after user release the button
int debounce = 100;

//ultrasonic sensor variables
float distance_cm, distance_m;
float fixedValue_cm, fixedValue_m;
unsigned long duration;
unsigned long measurementsCooldown = 1000;

//wifi UI variable
unsigned long startTime_WiFi;
unsigned long reconnectionCooldown = 15000;
int reconnectionAttempt;
int WiFiOption;

//measurements UI variable
int measureOption;
unsigned long startTime_Measure_0;
unsigned long startTime_Measure_1;

//sendTo UI variable
char messageBuffer[200];
int sendToOption;

//WiFi Credentials
const char ssid[] = "YOUR_WIFI_SSID_HERE";
const char password[] = "YOUR_WIFI_PASSWORD_HERE";

String phoneNumber = "YOUR_PHONE_NUMBER_HERE";
String apiKey = "YOUR_API_KEY_HERE";

//functions
void ultrasonicMeasure(); //functions to do the measurements
void Measure_UI(); //function for measuremens user interface
void SendTo_UI(); //functions for send to phone user interface
void WiFi_UI(); //functions for wifi setup that displayed after the ESP32 is turned on
void sendMessage(String phoneNumber, String apiKey, String message);//funcions to send the messages

void setup() {
  startTime_Measure_0=startTime_Measure_1=startTime_WiFi=millis();//set millis when code executed as start time, used for millis uninterruptable delay

  //Button pin setup
  pinMode(functionBtn, INPUT_PULLUP);
  pinMode(selBtn, INPUT_PULLUP);

  //Ultrasonic pin setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //initialize the lcd
  lcd.init();
  //turning on the backlight
  lcd.backlight();

  //display product information
  lcd.setCursor(0,0);
  lcd.print("------MeterVin------");
  lcd.setCursor(0,1);
  lcd.print(" Ultrasonic  Sensor ");
  lcd.setCursor(0,2);
  lcd.print(" Made by:");
  lcd.setCursor(0,3);
  lcd.print(" Marvin & Nicholas ");
  delay(3000);

  //display wifi information
  WiFi_UI();
}

//-------------------------------------------------------------------------------------

void loop() {  
  Measure_UI(); //dislay measurements (default with product size)
  SendTo_UI(); //send to phone
}

//-------------------------------------------------------------------------------------

void ultrasonicMeasure(){
  delay(100); //to make sure that previous pulse disappear before continue reading
  digitalWrite(trigPin, LOW);//clear the status of trig pin, making sure it is off
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);//set the trig pin high
  delayMicroseconds(10);//wait 10 microseconds
  digitalWrite(trigPin, LOW);//turn off the pin again
  duration = pulseIn(echoPin, HIGH);//read the echo pin, pulse in function will return the sound wave travel time in microseconds
  //calculation
  distance_cm = duration * 0.0343 / 2;
  distance_m = distance_cm / 100;
}

//-------------------------------------------------------------------------------------

void Measure_UI(){
  lcd.clear();

  measureOption=0;
  while(true){
    //confirm measurements using function button
    if(digitalRead(functionBtn)==pressed)
    {
      while(digitalRead(functionBtn)==pressed){}
      delay(debounce);
      while(digitalRead(functionBtn)==pressed){}
      break; //get out of while(true) loop
    } 
    //remove or add product size using select button
    else if (digitalRead(selBtn)==pressed)
    {
      while(digitalRead(selBtn)==pressed){}
      delay(debounce);
      measureOption++;
      if(measureOption>1){
        measureOption=0;
      }
    }

    switch(measureOption){
      case 0:
      {
        //display measurement WITH product size
        unsigned long currentTime_Measure_0 = millis();

        if (currentTime_Measure_0 - startTime_Measure_0 >= measurementsCooldown)
        {
          startTime_Measure_0=currentTime_Measure_0;
          ultrasonicMeasure();
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Distance measured: ");
          lcd.setCursor(0,1);
          lcd.print(distance_cm+12.2 + String(" cm"));
          lcd.setCursor(0,2);
          lcd.print(distance_m+0.122 + String(" m"));
          lcd.setCursor(0,3);
          lcd.print("(+ Product Size)");
          //result stored in temporary variable for next step after confirmation
          fixedValue_cm=distance_cm+12.2;
          fixedValue_m=distance_m+0.122;
        }
      }
      break;

      case 1:
      {
        //display measurement WITHOUT product size
        unsigned long currentTime_Measure_1 = millis();

        if (currentTime_Measure_1 - startTime_Measure_1 >= measurementsCooldown)
        {
          startTime_Measure_1=currentTime_Measure_1;
          ultrasonicMeasure();
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Distance measured: ");
          lcd.setCursor(0,1);
          lcd.print(distance_cm + String(" cm"));
          lcd.setCursor(0,2);
          lcd.print(distance_m + String(" m"));
          //result stored in temporary variable for next step after confirmation
          fixedValue_cm=distance_cm;
          fixedValue_m=distance_m;
        }
      }
      break;
    }
  }
  lcd.clear();//clear the screen as the last code executed in measure_UI function
}

//-------------------------------------------------------------------------------------
//send to phone
void SendTo_UI(){
  lcd.clear();

  sendToOption=0;
  while(true){
    //confirm user selection either yes or no using function button
    if(digitalRead(functionBtn)==pressed)
    {
      while(digitalRead(functionBtn)==pressed){}
      delay(debounce);
      break;//get out of while(true) loop
    } 
    //change between yes and no option in LCD using select button
    else if (digitalRead(selBtn)==pressed)
    {
      while(digitalRead(selBtn)==pressed){}
      delay(debounce);
      sendToOption++;
      if(sendToOption>1){
        sendToOption=0;
      }
    }
    //display the confimed result according to whether it include product size or not
    if(measureOption==0){
      lcd.setCursor(0,0);
      lcd.print("Measurements (+Size)");
      lcd.setCursor(0,1);
      sprintf(lcdBuffer, "%.2f cm | %.2f m ", fixedValue_cm, fixedValue_m);
      lcd.print(lcdBuffer);
    }
    //display the confimed result according to whether it include product size or not
    else if(measureOption==1){
      lcd.setCursor(0,0);
      lcd.print("Measurements        ");
      lcd.setCursor(0,1);
      sprintf(lcdBuffer, "%.2f cm | %.2f m ", fixedValue_cm, fixedValue_m);
      lcd.print(lcdBuffer);
    }
    //display the text send to phone? to ask user input
    lcd.setCursor(0,2);
    lcd.print("Send to phone?      ");

    //display to the user, current selection
    switch(sendToOption){
      case 0:
      lcd.setCursor(0,3);
      lcd.print("  ->Yes   |    No   ");
      break;
      case 1:
      lcd.setCursor(0,3);
      lcd.print("    Yes   |  ->No   ");
      break;
    }
  }

  //doing the action according to user select yes or no
  switch (sendToOption){
    // if user select yes
    case 0:
    lcd.setCursor(0,3);
    lcd.print(">Sending To Phone...");

    //send text messages according to whethter user included the product size or not
    if (measureOption==0)
    {
      sprintf(messageBuffer, "Hey! This is MeterVin. This is your measurements result\n%.2f cm | %.2f m\n\n(+Product size added)", fixedValue_cm, fixedValue_m);
      sendMessage(phoneNumber, apiKey, messageBuffer);
    }
    else if (measureOption==1)
    {
      sprintf(messageBuffer, "Hey! This is MeterVin. This is your measurements result\n%.2f cm | %.2f m\n\n(Product size not included)", fixedValue_cm, fixedValue_m);
      sendMessage(phoneNumber, apiKey, messageBuffer);
    }
    break;
  
    case 1:
    //do nothing because user selected no
    break;
  }

  delay(1000);
  //display thank you message
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Thank you for using!");
  lcd.setCursor(0,2);
  lcd.print("----- MeterVin -----");
  delay(2000);
  lcd.clear();
}

//-------------------------------------------------------------------------------------
//display wifi information
void WiFi_UI(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--- Wi-Fi Status ---");
  lcd.setCursor(0,1);
  lcd.print("Connecting...");
  //set the wifi mode to station mode
  WiFi.mode(WIFI_STA);
  
  //begin wifi connection with provided ssid and password
  WiFi.begin(ssid, password);
  delay(2000);
  //check if the wifi already successfully connected with wifi.begin
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.setCursor(0,1);
    lcd.print("Connected!          ");
    lcd.setCursor(0,2);
    lcd.print(ssid);
    lcd.setCursor(0,3);
    lcd.print(WiFi.localIP());
    delay(3000);
  }
  //otherwise, this line below will executed
  else if (WiFi.status() != WL_CONNECTED)
  {
    //connect to wifi or skip
    WiFiOption=0;
    while (true){
      //confirm the user selection
      if(digitalRead(functionBtn)==pressed)
      {
        while(digitalRead(functionBtn)==pressed){}
        delay(debounce);
        break;
      } 
      else if (digitalRead(selBtn)==pressed)
      {
        //change the selection from reconnect to skip and vice versa
        while(digitalRead(selBtn)==pressed){}
        delay(debounce);
        WiFiOption++;
        if(WiFiOption>1){
          WiFiOption=0;
        }
      }

      //print status not connected to LCD
      lcd.setCursor(0,1);
      lcd.print("Not connected!      ");

      switch (WiFiOption)
      {
        case 0:
        //display the status of user selection, in this case, reconnect
        lcd.setCursor(0,2);
        lcd.print("-> Reconnect");
        lcd.setCursor(0,3);
        lcd.print("   Skip");
        break;
      
        case 1:
        //display the status of user selection, in this case, skip
        lcd.setCursor(0,2);
        lcd.print("   Reconnect");
        lcd.setCursor(0,3);
        lcd.print("-> Skip");
        break;
      }
    }

    //do the action according to confirmed selection
    switch (WiFiOption)
    {
      case 0:
      //trying to connect while not connected
      while (WiFi.status()!= WL_CONNECTED)
      {
        //retrying multiple attempt while not connected
        unsigned long currentTime_WiFi=millis();
        if (currentTime_WiFi-startTime_WiFi>=reconnectionCooldown || reconnectionAttempt==0)
        {
          startTime_WiFi=currentTime_WiFi;
          lcd.setCursor(0,1);
          lcd.print("Reconnecting...     ");
          lcd.setCursor(0,2);
          lcd.print("                    ");
          lcd.setCursor(0,3);
          lcd.print("Attempt: "+ String(reconnectionAttempt+1));
          WiFi.disconnect();
          WiFi.reconnect();
          reconnectionAttempt++;
        }
      }
      //connection succesful, therefore getting out of while loop
      //show wifi status
      if (WiFi.status()== WL_CONNECTED)
      {
        lcd.setCursor(0,1);
        lcd.print("Connected!          ");
        lcd.setCursor(0,2);
        lcd.print(ssid);
        lcd.setCursor(0,3);
        lcd.print(WiFi.localIP());
        delay(3000);
      }
      break;
    
      case 1:
      //do nothing if user select to skip
      break;
    }
  }
  lcd.clear();
}

void sendMessage(String phoneNumber, String apiKey, String message){
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&text=" + urlEncode(message) + "&apikey=" + apiKey;

  //object creation with name http
  HTTPClient http;
  //starts an HTTP session with a server. It's used in Arduino code to perform HTTP requests on ESP8266 and ESP32 boards. For example, to send an HTTP request to api.callmebot.com
  http.begin(url);

  //Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  //Send HTTP POST request, send data and save the return value (response code) to variable httpResponseCode
  int httpResponseCode = http.POST(url);
  //code 200 means OK (standard response status code for a successful HTTP request)
  if (httpResponseCode == 200){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print(">Sent");
  }
  //other code than 200 means other (assume that as a failed request)
  else{
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    sprintf(lcdBuffer,">Failed, code: %d", httpResponseCode);
    lcd.print(lcdBuffer);
  }

  //free up the resources
  http.end();
}