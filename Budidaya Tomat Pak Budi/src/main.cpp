#include <Arduino.h>

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6j7BPqVJ8"
#define BLYNK_TEMPLATE_NAME "Weather Station"
#define BLYNK_AUTH_TOKEN "LhvG059ul4tBGOPUH3IZT3VUlR9h77M8"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp32.h>



#define TEMP_PIN 25                          
//#define RELAY_PIN 32
//#define SCHED_PIN 33
#define RAIN_PIN 26
//#define TEMP_PIN 35                          
#define RELAY_PIN 15
#define SCHED_PIN 33
#define DHTTYPE DHT22



// calculate debit water
const float literPerDetik = 0.0666666666666;  // Debit air dalam liter per detik
int activeTime;  // Waktu penyiraman dalam detik

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 25200, 60000);
BlynkTimer timer;
DHT dht (TEMP_PIN, DHTTYPE);

const char *ssid     = "nodeMCU8266";
const char *password = "12345678";

int rain_value =0;
float banyakAir=0;
float water_out=0;
int RELAYPIN=0;
int relayPin=0;
float temp=0.0;
int schedPin = 0;
int hour=0;
int minute=0;
int second=0;
String rain_status;
float banyakAir_new = 0.0;



//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//////////////////////////////////////////////////////////////////////////////////////////////////////
void time(); 
void temperature();
void rain();
void schedule();
void penyiraman();
void delayOneDay();
void sendData1();

//////////////////////////////////////////////////////////////////////////////////////////////////////


void setup(){
  Serial.begin(115200);
  pinMode(TEMP_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  dht.begin();
  Serial.println("DHT startup!");
  timeClient.begin();  
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, password);
  timer.setInterval(1000L, sendData1);
}

void loop() { 
  if(relayPin == 0){
    time(); 
  }
  time(); 
  temperature();
  rain();
  schedule();
  penyiraman();
  Blynk.run();
  timer.run();
  
  if(rain_status == "Hujan"){
    delay(3000);
    delayOneDay();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


BLYNK_WRITE(V2){
   schedPin = param.asInt();
} 

BLYNK_WRITE(V4){
   relayPin = param.asInt();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void time(){
  timeClient.update();
/*
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);
*/
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  String weekDay = weekDays[timeClient.getDay()];
  // Serial.printf("Day, Hour:Minutes:Seconds  ->  %s,\t%d:%d:%d \n", weekDay, currentHour, currentMinute, currentSecond);
}

void temperature() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
  Serial.println(F("Failed to read from DHT sensor!"));
  return;
  }
  temp = dht.computeHeatIndex(t, h, false);
//   Serial.print(temp);
//   Serial.print(F("°C "));
  // if (temp > 32) {
  //   relayPin = 1;
  //   digitalWrite(RELAY_PIN,HIGH);
  // } else if (temp < 27) {
  //   relayPin = 0;
  //   digitalWrite(RELAY_PIN,LOW);
  // }
}


void rain(){              
  // Pembacaan rain sensor
  rain_value = digitalRead(RAIN_PIN);
  if(rain_value <= LOW){
    rain_status = "Hujan";
    delay(3000);
  }else{
    rain_status = "Cerah";
  }

}

void schedule(){
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  if(schedPin == 1){
    // Serial.println("Scheduling Status: ON");
    if (currentHour == hour && currentMinute == minute && currentSecond == second){
      relayPin = 1;
      digitalWrite(RELAY_PIN,HIGH);
    }
    else if (currentHour == hour && currentMinute == minute && currentSecond == second+3){
      relayPin = 0;
      digitalWrite(RELAY_PIN,LOW);
    }
  }
//   else{
//     Serial.println("Scheduling Status: OFF");
//   }
}

void penyiraman(){
  // Baca sensor untuk mendeteksi awal penggunaan
  if (relayPin == 1) {
    digitalWrite(RELAY_PIN,HIGH);
    delay(1000);  // Penghitungan detik penyiram menyala
    activeTime++;
  }

   banyakAir = (activeTime) * literPerDetik;  // Hitung banyaknya air dalam liter
   Serial.printf("Debit air: %.2f\n", banyakAir);

  // Baca sensor untuk mendeteksi akhir penggunaan
  if (relayPin == 0) {
    water_out = banyakAir + banyakAir_new;
    banyakAir_new = banyakAir;
    activeTime = 0;
    digitalWrite(RELAY_PIN,LOW);
  }
}

void delayOneDay(){
  Serial.println("Delaying one day...");
  delay( 86400000 );
}

void sendData1(){
    Serial.println("\n//////////////////////////////////////////////////////////////////////////////////////////////////////");
    Serial.println("Send data \n"); 

    Serial.printf("Temperature: %0.2f\n", temp);
    Blynk.virtualWrite(V0, temp); 

    Serial.printf("Relay Value: %d  \n", relayPin);
    // Blynk.virtualWrite(V4, relayPin); 

    Serial.printf("Rain status: %s\n", rain_status);
    Blynk.virtualWrite(V3, rain_status);

    Serial.printf("Sched_Pin status: %d\n", schedPin);

    Serial.printf("Banyak Air status: %0.2f\n", water_out);
    Blynk.virtualWrite(V1, water_out);

}
