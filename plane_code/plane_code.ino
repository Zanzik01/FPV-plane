#include "CrsfSerial.h"
#include <Servo.h>
#include <Adafruit_BMP280.h>

#define PIN_VOLTAGE_SENSOR        A0
#define PIN_SERVO_ELEVATOR         3
#define PIN_SERVO_LEFT_AILERON     5
#define PIN_SERVO_RIGHT_AILERON    10
#define PIN_GAS                    9
#define PIN_BUZZER                6

Servo Servo_left_aileron;
Servo Servo_right_aileron;
Servo Servo_elevator;
Servo ESC;

Adafruit_BMP280 bmp;

int notes[] = {
  392, 392, 392, 311, 466, 392, 311, 466, 392,
  587, 587, 587, 622, 466, 369, 311, 466, 392,
  784, 392, 392, 784, 739, 698, 659, 622, 659,
  415, 554, 523, 493, 466, 440, 466,
  311, 369, 311, 466, 392,
};

int times[] = {
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 250, 100, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 750,
};

typedef struct remoteData_s {
  uint16_t voltage;
} remoteData_t;

remoteData_t remoteData;

float adc_voltage = 0.0;
float in_voltage = 0.0;
 
float R1 = 30000.0;
float R2 = 7500.0; 
 
// Значение опорного напряжения
float ref_voltage = 5.0;

int adc_value = 0;

float expRunningAverageAdaptive(float newVal) {
  static float filVal = 0;
  float k;
  // резкость фильтра зависит от модуля разности значений
  if (abs(newVal - filVal) > 1.5) k = 0.9;
  else k = 0.03;
  
  filVal += (newVal - filVal) * k;
  return filVal;
}

CrsfSerial crsf(Serial, 500000);

static void crsfLinkUp() {
  digitalWrite(LED_BUILTIN, HIGH);
}

static void crsfLinkDown() {
  digitalWrite(LED_BUILTIN, LOW);
 }

int ch1 = 1500;
int ch2 = 1500;
int ch3 = 1500;
int ch4 = 1500;
int ch5 = 1500;
int ch6 = 1500;
int ch7 = 1500;
int ch8 = 1500;
int ch9 = 1500;
int ch10 = 1500;

void setup() {
  Serial.begin(500000);
  crsf.onLinkUp = &crsfLinkUp;
  crsf.onLinkDown = &crsfLinkDown;

  for (int i = 0; i < 39; i++) {
    tone(PIN_BUZZER, notes[i], times[i]*2);
    delay(times[i]*2);
    noTone(PIN_BUZZER);
  }

  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while(1);
  }

  Servo_left_aileron.attach(PIN_SERVO_LEFT_AILERON);
  Servo_right_aileron.attach(PIN_SERVO_RIGHT_AILERON);
  Servo_elevator.attach(PIN_SERVO_ELEVATOR);
  ESC.attach(PIN_GAS);

}

void loop() {
  crsf.loop();
  // руль высоты
  ch2 = map(crsf.getChannel(2),2000,1000,55,125);
  Servo_elevator.write(ch2);
  // Элероны
  ch4 = map(crsf.getChannel(4),2000,1000,55,125);
  Servo_left_aileron.write(ch4);
  Servo_right_aileron.write(ch4);

  // Арм
  if (crsf.getChannel(5) == 2000){
    // газ
    ch3 = map(crsf.getChannel(3),2000,1000,55,125);
    ESC.write(ch3);
    // Serial.println("АРМ");
    } else {
      ESC.write(1000);
      // Serial.println("Дизарм");
      }
  // Протокол "Потеряшка"
  if (crsf.getChannel(6) == 2000){
    tone(PIN_BUZZER, 1000); //пищалка орет
  }else{
      noTone(PIN_BUZZER);//пищалка не орет
      }
  // Протокол "высота"
  if (crsf.getChannel(7) == 1000){
    // Вывод данных с барометра в монитор порта для теста
    // Serial.print(F("Temperature = "));
    // Serial.print(bmp.readTemperature());
    // Serial.println(" *C");
    
    // Serial.print(F("Pressure = "));
    // Serial.print(bmp.readPressure());
    // Serial.println(" Pa");
 
    // Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1012.23)); //переменная подстраивается под высоту местности
    // Serial2.print(bmp.readAltitude(1012.23));
    // Serial.println(" m");
  }
  // Протокол "напряжение"
  if (crsf.getChannel(8) == 1000){
    // ВЫВОД ДАННЫХ НАПРЯЖЕНИЯ БАТАРЕИ
    adc_value = analogRead(PIN_VOLTAGE_SENSOR);
   
    adc_voltage  = (adc_value * ref_voltage) / 1024.0; 
   
    in_voltage = adc_voltage / (R2/(R1+R2)) ; 
   
    //Serial.print("Input Voltage = ");
    Serial.println(in_voltage, 2);
  }
}
