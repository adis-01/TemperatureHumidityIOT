#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "YOUR_WIFI_ID"
#define WIFI_PASSWORD "YOUR_WIFI_PASS"

// Insert Firebase project API Key
#define API_KEY "YOUR_API_KEY"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "YOUR_DATABASE_URL" 
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
#include <DHT.h>

DHT dht;
int senzor = D3;
int dioda = D1;
int slider = 0;
bool upaliUredjaj = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(senzor, INPUT);
  dht.setup(senzor);
  Serial.begin(115200);
  pinMode(dioda,OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

void loop() {
    if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)){

      if(Firebase.RTDB.getInt(&fbdo,"/mjerenja/sliderVrijednost")){
        slider = fbdo.intData();
        if(slider!=0){
          analogWrite(dioda,slider);
          delay(3000);
        }
        else{
          analogWrite(dioda,0);
          Firebase.RTDB.setInt(&fbdo,"mjerenja/sliderVrijednost",0);
        }
      }

      if(Firebase.RTDB.getBool(&fbdo, "/mjerenja/jelUpaljeno")){
          upaliUredjaj = fbdo.boolData();
          if(upaliUredjaj){
            digitalWrite(dioda,HIGH);
          }
          else{
            digitalWrite(dioda,LOW);
          }
      }

      float temperatura=dht.getTemperature();
      float vlaznost=dht.getHumidity();

      if (Firebase.RTDB.setInt(&fbdo, "mjerenja/vlaznost", vlaznost)){
          String porukaVl = "Poslata vrijednost vlaznosti " + String(vlaznost);
          Serial.println(porukaVl);
        }
      else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        } 

      if (Firebase.RTDB.setFloat(&fbdo, "mjerenja/temperatura", temperatura)){
          String porukaTemp = "Poslata vrijednost temperature " + String(temperatura);
          Serial.println(porukaTemp);
          Serial.println("************************");
        }
      else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
    }
    delay(2000);
}
