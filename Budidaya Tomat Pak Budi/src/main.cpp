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



//#define RAIN_PIN 25
#define TEMP_PIN 25                          
//#define RELAY_PIN 32
//#define SCHED_PIN 33
#define RAIN_PIN 26
//#define TEMP_PIN 35                          
#define RELAY_PIN 15
#define SCHED_PIN 33
#define DHTTYPE DHT22
#define TEMP_BLYNK_VPIN V0
#define RAIN_BLYNK_VPIN V3
#define IRRG_BLYNK_VPIN V1


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
int relayPin=0;
int temp=0;
int schedPin = 0;
int hour=0;
int minute=0;
int second=0;

// Rain Module variables
int RAIN;
// DHT related variables
float hic;

BLYNK_WRITE(V4){
  int pin=param.asInt();
  digitalWrite(RELAY_PIN,pin);
}
BLYNK_WRITE(V2){
  int pin=param.asInt();
  schedPin=pin;
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
void temperature(){              // Pembacaan 
  temp = analogRead(TEMP_PIN);      
  relayPin = digitalRead(RELAY_PIN);
  // Serial.printf("Moisture value: %d,\tPersen tanah: %d \n",soil_value, persen_tanah)
  if (temp < 27) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temp > 30) {
    digitalWrite(RELAY_PIN, LOW);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void sendRainData() {
  Serial.println("Sending Rain data");
  Blynk.virtualWrite(TEMP_BLYNK_VPIN, RAIN);
}

void sendIrrigationData(){
  Serial.println("Sending Irrigation Data");
  Blynk.virtualWrite(IRRG_BLYNK_VPIN, water_out);
}
void rain(){              // Pembacaan rain sensor
  rain_value = digitalRead(RAIN_PIN);
  RAIN = rain_value;
  if(rain_value == LOW){
    Serial.println("Oh! It's Raining...\n");
    digitalWrite(RELAY_PIN, HIGH);
    sendRainData();
    delay(3000);
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
  Blynk.virtualWrite(TEMP_BLYNK_VPIN, hic);
}

void readAndSendDhtData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  relayPin = digitalRead(RELAY_PIN); 
  if (isnan(h) || isnan(t)) {
  Serial.println(F("Failed to read from DHT sensor!"));
  return;
  }
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print(hic);
  Serial.print(F("°C "));
  if (hic < 27) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (hic > 30) {
    digitalWrite(RELAY_PIN, LOW);
  };
  sendDhtData();
}

void reandAndSendSensorsData() {
  readAndSendDhtData();
  rain();
  sendIrrigationData();
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

void penyiraman(){
  int relayPin = digitalRead(RELAY_PIN);
  float banyakAir;
  float banyakAir_new = 0.0;
  float water_out;
  //int schedSwitch = digitalRead(SCHED_PIN);
  // Baca sensor untuk mendeteksi awal penggunaan
  // Baca sensor untuk mendeteksi awal penggunaan
  if (relayPin == HIGH) {
    delay(1000);  // Penghitungan detik penyiram menyala
    activeTime++;
  }

   banyakAir = (activeTime) * literPerDetik;  // Hitung banyaknya air dalam liter
   Serial.printf("Debit air: %.2f\n", banyakAir);

  // Baca sensor untuk mendeteksi akhir penggunaan
  if (relayPin == LOW) {
    water_out = banyakAir + banyakAir_new;
    banyakAir_new = banyakAir;
  }
}

void schedule(){
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  int schedSwitch = digitalRead(SCHED_PIN);
  if(schedPin == 1){
    Serial.println("Scheduling started...");
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
// } 

void setup(){
  Serial.begin(115200);
  pinMode(TEMP_PIN, INPUT);
  //pinMode(SCHED_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("DHT startup!");
  dht.begin();
  //setupDht();
  timeClient.begin();  
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, password);
  //timer.setInterval(5000L, sendData1);
  //timer.setInterval(1000L, reandAndSendSensorsData);
}

void loop() { 
  //Blynk.virtualWrite(TEMP_BLYNK_VPIN, hic);
  //Blynk.virtualWrite(HUMID_BLYNK_VPIN, h);
  /*f(relayPin == LOW){
    time(); 
  }*/
  reandAndSendSensorsData();
  //temperature();
  //int sched = digitalRead(SCHED_PIN);
  //Serial.printf("Penyiram: %d \n",sched);
  schedule();
  penyiraman();
  Blynk.run();
  timer.run();
  // delay(1000); //udah kena delay dari fungsi time
}
