#include <Arduino.h>


#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL605-FzYIY"
#define BLYNK_TEMPLATE_NAME "DDB"
#define BLYNK_AUTH_TOKEN "yuAzKSHfrCcg2w26dD9V5pCMb3QvojeL"

#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp32.h>

#define RAIN_PIN 34
#define TEMP_PIN 35                          
#define RELAY_PIN 32
#define SCHED_PIN 12

// calculate debit water
const float literPerDetik = 0.0666666666666;  // Debit air dalam liter per detik
int activeTime;  // Waktu penyiraman dalam detik

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 25200, 60000);
BlynkTimer timer;

const char *ssid     = "Raihan Fakhar";
const char *password = "87654321";

int rain_value =0;
float banyakAir=0;
int relayPin=0;
int temp=0;

int hour=0;
int minute=0;
int second=0;


//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  void time(); 
  void temperature();
  void rain();
  void schedule();
  void penyiraman();
  void delayOneDay();
//////////////////////////////////////////////////////////////////////////////////////////////////////

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

  timeClient.begin();  
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, password);
  // timer.setInterval(5000L, sendData1);
  // timer.setInterval(1000L, sendData2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() { 
  if(relayPin == LOW){
    time(); 
  }
  temperature();
  schedule();
  rain();
  penyiraman();
  Blynk.run();
  timer.run();
  
  // delay(1000); //udah kena delay dari fungsi time
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void temperature(){              // Pembacaan Soil Sensor
  temp = analogRead(TEMP_PIN);      
  relayPin = digitalRead(RELAY_PIN);
  // Serial.printf("Moisture value: %d,\tPersen tanah: %d \n",soil_value, persen_tanah);

  if (temp < 27) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temp > 30) {
    digitalWrite(RELAY_PIN, LOW);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void rain(){              // Pembacaan rain sensor
  rain_value = analogRead(RAIN_PIN);
  // Serial.printf("Rain sensor value: %d\n", rain_value);
  if(rain_value > 25){
    delayOneDay();
    }
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

  Serial.printf("Day, Hour:Minutes:Seconds  ->  %s,\t%d:%d:%d \n", weekDay, currentHour, currentMinute, currentSecond);
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////

void penyiraman(){
  int relayPin = digitalRead(RELAY_PIN);
  float banyakAir_new = 0.0;
  int schedSwitch = digitalRead(SCHED_PIN);

  // Baca sensor untuk mendeteksi awal penggunaan
  if (relayPin == HIGH) {
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

void delayOneDay(){
  Serial.println("Delay one day");
  delay( 86400000 );
}

// BLYNK_WRITE(V3){
//   int data = param.asInt();
//   digitalWrite(RELAY_PIN, data);
// }


