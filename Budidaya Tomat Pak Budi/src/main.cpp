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
//#include "BlynkEdgent.h"


#define RAIN_PIN 25
#define TEMP_PIN 26                          
#define RELAY_PIN 2
#define SCHED_PIN 33
#define DHTTYPE DHT22
#define TEMP_BLYNK_VPIN V0
#define HUMID_BLYNK_VPIN V1
#define RAIN_BLYNK_VPIN V3

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
int relayPin=0;
int temp=0;

int hour=0;
int minute=0;
int second=0;

// Rain Module variables
int RAIN;
// DHT related variables
int   DHT_ENABLED = 0;
float DHT_HUMIDITY;
float DHT_HUMIDITY_IGNORED_DELTA = 0.01;
float DHT_TEMPERATURE;
float DHT_TEMPERATURE_IGNORED_DELTA = 0.01;

BLYNK_WRITE(V4){
  int pin=param.asInt();
  digitalWrite(RELAY_PIN,pin);
}

//DHT setup
void setupDht() {
  Serial.println("DHT startup!");
  dht.begin();
  DHT_ENABLED = 1;
}

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//void time(); 
//void temperature();
//void rain();
void schedule();
void penyiraman();
void delayOneDay();
//void sendRainData();
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*void temperature(){              // Pembacaan 
  temp = analogRead(TEMP_PIN);      
  relayPin = digitalRead(RELAY_PIN);
  // Serial.printf("Moisture value: %d,\tPersen tanah: %d \n",soil_value, persen_tanah)
  if (temp < 27) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temp > 30) {
    digitalWrite(RELAY_PIN, LOW);
  }
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
void sendRainData() {
  Serial.println("Sending Rain data");
  Blynk.virtualWrite(TEMP_BLYNK_VPIN, RAIN);
}
void rain(){              // Pembacaan rain sensor
  rain_value = digitalRead(RAIN_PIN);
  RAIN = rain_value;
  if(rain_value == LOW){
    Serial.println("Oh! It's Raining...\n");
    digitalWrite(RELAY_PIN, HIGH);
    sendRainData();
    delayOneDay();
    }else{
      sendRainData();
    }
}
void delayOneDay(){
  Serial.println("Delaying one day...");
  delay( 86400000 );
}
void sendDhtData() {
  Serial.println("Sending DHT data");
  Blynk.virtualWrite(TEMP_BLYNK_VPIN, DHT_TEMPERATURE);
  Blynk.virtualWrite(HUMID_BLYNK_VPIN, DHT_HUMIDITY);
}
void readAndSendDhtData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  relayPin = digitalRead(RELAY_PIN);
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT");
  } else {
    float humidityDelta = abs(humidity - DHT_HUMIDITY) - DHT_HUMIDITY_IGNORED_DELTA;
    float temperatureDelta = abs(temperature - DHT_TEMPERATURE) - DHT_HUMIDITY_IGNORED_DELTA;
    if (humidityDelta > 0 || temperatureDelta > 0) {
      DHT_HUMIDITY = humidity;
      DHT_TEMPERATURE = temperature;
      Serial.printf("Humidity: %f%%. Temperature: %f*C.\n", humidity, temperature);
    if (temperature < 27) {
      digitalWrite(RELAY_PIN, LOW);
    } else if (temperature > 30) {
      digitalWrite(RELAY_PIN, HIGH);
    };
      sendDhtData();
    }
  }
}
void reandAndSendSensorsData() {
  readAndSendDhtData();
  rain();
  Serial.println("Sending data from sensors");
}
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
  Serial.printf("Day, Hour:Minutes:Seconds  ->  %s,\t%d:%d:%d \n", weekDay, currentHour, currentMinute, currentSecond);
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////
void penyiraman(){
  int relayPin = digitalRead(RELAY_PIN);
  float banyakAir_new = 0.0;
  // Baca sensor untuk mendeteksi awal penggunaan
  if (relayPin == HIGH) {
    Serial.println("Start Watering...");
    delay(1000);  // Penghitungan detik penyiram menyala
    ++activeTime;
  }
  // Hitung banyaknya air dalam liter
   banyakAir = activeTime * literPerDetik;  
   Serial.printf("Debit air: %.2f\n", banyakAir);
  // Baca sensor untuk mendeteksi akhir penggunaan
  if (relayPin == LOW) {
    // Tampilkan hasil pada monitor serial
    // Serial.printf("Banyaknya Air yang Keluar: %.6f Liter", water_out);
    activeTime = 0;
  }
}
void schedule(){
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  int schedSwitch = digitalRead(SCHED_PIN);
  if(schedSwitch== HIGH){
    if (currentHour == hour && currentMinute == minute && currentSecond == second){
      digitalWrite(RELAY_PIN, HIGH);
    }
    else if (currentHour == hour && currentMinute == minute && currentSecond == second+3){
      digitalWrite(RELAY_PIN, LOW);
    }
  }
}

// BLYNK_WRITE(V3){
//   int data = param.asInt();
//   digitalWrite(RELAY_PIN, data);
// } omg what the fuck

void setup(){
  Serial.begin(115200);
  pinMode(TEMP_PIN, INPUT);
  pinMode(SCHED_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  setupDht();
  timeClient.begin();  
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, password);
  // timer.setInterval(5000L, sendData1);
  // timer.setInterval(1000L, sendData2);
}
void loop() { 
  if(relayPin == LOW){
    time(); 
  }
  reandAndSendSensorsData();
  //temperature();
  schedule();
  penyiraman();
  Blynk.run();
  timer.run();
  // delay(1000); //udah kena delay dari fungsi time
}
