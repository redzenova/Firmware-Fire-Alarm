//========================================================
//====|      This Firmware version beta 0.1 for      |====
//====| Cloud-based Fire Alarm and Air Quality Check |====
//====|        Programming by Raweeroj Thongdee      |====
//========================================================
//======================| Setting |=======================
  #define Device_Mode    1    // 0-CloudBase_Mode
                              // 1-StanAlone_Mode
  
//========================================================
//======================| Library |=======================
  #include "DHT.h"                  // for dht11/22 sensor
  #include <PubSubClient.h>         // for MQTT 
  #include <Adafruit_NeoPixel.h>    // for Neopixel
  #include <WiFi.h>                 // for ESP32 APClient
  //#include <IOXhop_FirebaseESP32.h> // for ESP32 to Firebase
//========================================================
//=====================| PIN MAP |========================
  #define Flame_IR_01       39      // VN pin
  #define Flame_IR_02       34      // D34 pin
  #define Flame_Somke_03    A0      // VP pin
  #define Air_Temp_01       32      // D32 pin
  #define Air_CO_02         A7      // D35 pin
  #define Air_UV_03         A5      // D33 pin
  #define Air_Light_04_SCL  22      // D22 pin
  #define Air_Light_04_SDA  21      // D21 pin
  #define Air_Dust_05_Di    14      // D14 pin
  #define Air_Dust_05_An    A17     // D27 pin
  #define Alarm_Buzzer_01   12      // D12 pin
  #define Alarm_Neo_02      13      // D13 pin
//========================================================
//=====================| Constant |=======================
  #define Air_Temp_Type   DHT22
  #define NUM_Neopixel       40
  #define def_CO             4.0
  #define def_Smoke          4.0
//========================================================
//=====================| Conection |======================
  #define FIREBASE_HOST   "cloud-b2f31.firebaseio.com"
  #define FIREBASE_AUTH   "QxbHtnr5t6jHELLJ6tJinlhwhkuNjPefD0ybU26y"
  #define MQTT_Server     "node.raweeroj.xyz"
  #define MQTT_port       1883
  #define MQTT_user       "nodemqtt"
  #define MQTT_pass       "nodemqtt2017"
  #define WIFI_SSID       "PCCPL-WiFi" //Project-ID
  #define WIFI_PASSWORD   "" //terminal99
  char    clientId[]   =   "CL_01" ;
//========================================================
//=======================| TOPIC |========================
  char TEMP[]        =   "/temp"  ;
  char HUMI[]        =   "/humi"  ;
  char FIRE_1[]      =   "/ir1"   ;
  char FIRE_2[]      =   "/ir2"   ;
  char BUZZ[]        =   "/buzz"  ;
  char Flame_Somke[] =   "/smoke" ;
  char AIR_CO[]      =   "/airco" ;
//========================================================
//======================| Varible |=======================
  float Val_Air_CO,
        TP_Air_CO,
        Val_Flame_Somke,
        TP_Flame_Somke,
        FIR_01,
        FIR_02,
        Temp = 0,
        humi = 0 ;
  char com[100],
       temp[100];
  String name;      
//========================================================
//===================| Pre_Processing |===================
  DHT dht(Air_Temp_01, Air_Temp_Type);
  
  WiFiClient espClient;
  PubSubClient client(espClient);
  
  Adafruit_NeoPixel Neo = Adafruit_NeoPixel(
                            NUM_Neopixel, 
                            Alarm_Neo_02, 
                            NEO_GRB + NEO_KHZ800
                            );
//========================================================
//===================| User_Function |====================
  char *cat(char input[]){
      char *cc;
      strcpy(temp,clientId);
      strcpy(com,strcat(clientId,input));
      cc=com;
      strcpy(clientId,temp);
      return cc;
    }
    
  void receivedCallback(char* topic, byte* payload, unsigned int length) {
          Serial.print("Message received: ");
          Serial.println(topic);
          Serial.print("payload: ");
              for (int i = 0; i < length; i++){
                  Serial.print((char)payload[i]);
              }
       }

   void mqttconnect() {
          while (!client.connected()) {
            Serial.print("MQTT connecting ...");
            if (client.connect(clientId, MQTT_user, MQTT_pass)) {
              Serial.println("connected");
            } 
            else {
              Serial.print("failed, status code =");
              Serial.print(client.state());
              Serial.println("try again in 5 seconds");
              delay(5000);
            }
          }
        }

   void theaterChase(uint32_t c, uint8_t wait) {
        for (int j=0; j<15; j++) {  //do 10 cycles of chasing
            for (int q=0; q < 3; q++) {
                for (uint16_t i=0; i < Neo.numPixels(); i=i+3) {
                    Neo.setPixelColor(i+q, c);    //turn every third pixel on
                }
                   Neo.show();
                   delay(wait);
                for (uint16_t i=0; i < Neo.numPixels(); i=i+3) {
                   Neo.setPixelColor(i+q, 0);     //turn every third pixel off
                }
           }
       }
      }

      void rainbow(uint8_t wait) {
        uint16_t i, j;
          for(j=0; j<256; j++) {
             for(i=0; i<Neo.numPixels(); i++) {
                  Neo.setPixelColor(i, Wheel((i+j) & 255));
            }
              Neo.show();
              delay(wait);
          }
        }

    uint32_t Wheel(byte WheelPos) {
             WheelPos = 255 - WheelPos;
                if(WheelPos < 85) {
                    return Neo.Color(255 - WheelPos * 3, 0, WheelPos * 3);
                }
                if(WheelPos < 170) {
                    WheelPos -= 85;
                    return Neo.Color(0, WheelPos * 3, 255 - WheelPos * 3);
                }
                    WheelPos -= 170;
                    return Neo.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
            }
            
   void colorWipe(uint32_t c, uint8_t wait) {
        for(uint16_t i=0; i < Neo.numPixels(); i++) {
            Neo.setPixelColor(i, c);
            Neo.show();
            delay(wait);
  }
}

//========================================================
//========================| Setup |=======================
void setup() {
   Serial.begin(115200);
   pinMode (Flame_IR_01,INPUT ) ;
   pinMode (Flame_IR_02,INPUT ) ;
   pinMode (Alarm_Buzzer_01,OUTPUT ) ;
   if(Device_Mode == 0){
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(MQTT_Server, MQTT_port);
    client.setCallback(receivedCallback);
   } 
   dht.begin();
   Neo.begin();
   Neo.show();
 // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
//========================================================
//==================| Loop_Programming |==================
void loop() {
  if(Device_Mode == 0){
    if(!client.connected()){
      mqttconnect();
    }
    client.loop();
  }
  //==== Start Detect Fire ====
  TP_Air_CO      = analogRead(Air_CO_02);
  TP_Flame_Somke = analogRead(Flame_Somke_03);
  Val_Air_CO      = TP_Air_CO/4096.0*5.0 ;
  Val_Flame_Somke = TP_Flame_Somke/4096.0*5.0 ;
  FIR_01          = digitalRead(Flame_IR_01) ;
  FIR_02          = digitalRead(Flame_IR_02) ;
                  
  if (FIR_01 == LOW || FIR_02 == LOW || Val_Air_CO >= def_CO || Val_Flame_Somke >= def_Smoke){
      digitalWrite(Alarm_Buzzer_01,HIGH);
      theaterChase(Neo.Color(127, 0, 0), 25); // Red
      client.publish(cat(FIRE_1), "1");
      client.publish(cat(FIRE_2), "1");
      client.publish(cat(BUZZ), "1");
      client.publish(cat(Flame_Somke), String(Val_Flame_Somke).c_str(), true);
      client.publish("CL_01/airco", String(Val_Air_CO).c_str(), true); 
      
    }
    else if (FIR_01 == HIGH || FIR_02 == HIGH || Val_Air_CO < 2.7 || Val_Flame_Somke < 2.7){
      digitalWrite(Alarm_Buzzer_01,LOW);
      colorWipe(Neo.Color(0, 255, 0),1);
      //rainbow(100);
      client.publish(cat(FIRE_1), "0");
      client.publish(cat(FIRE_2), "0");
      client.publish(cat(BUZZ), "0");
      client.publish(cat(Flame_Somke), String(Val_Flame_Somke).c_str(), true);
    }
    else{
      client.publish(cat(FIRE_1), "00");
      client.publish(cat(FIRE_2), "00");
      client.publish(cat(Flame_Somke), "00");

    }
    //===== End Detect Fire =====
    //==== Start Air-Qurity Check ====
        //====|> Tempurature and Humidity <|===========================
            humi = dht.readHumidity();
            Temp = dht.readTemperature();
            if (!isnan(Temp)&&!isnan(humi)) {
              
              //Firebase.pushFloat("/temperature", 20.25);
              //Firebase.pushFloat("/humidity", 30);
              //Firebase.pushFloat("/list-temp", 20.25);

              client.publish(cat(TEMP), String(Temp).c_str(), true);
              client.publish(cat(HUMI), String(humi).c_str(), true);
            }
        //============================================================
        //====|> CO Sensor <|=========================================
              client.publish("CL_01/airco",String(Val_Air_CO).c_str(), true);
        //============================================================
    //===== End Air-Qurity Check =====
    delay(1000);
}
//=======================================================
