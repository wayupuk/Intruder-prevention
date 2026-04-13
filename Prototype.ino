#include <LiquidCrystal_I2C.h>
#include  <Wire.h>
#include <DHT11.h>
#include "HUSKYLENS.h"
#include <Servo.h>

HUSKYLENS huskylens;
Servo myServo;  // Create a servo object
LiquidCrystal_I2C lcd(0x27,  16, 2);

int servopin = 9;
int triger = 6;
int echo = 7;
int buzzer = A3; // in case of use analog write
int LED_pin = 2;
long duration,cm;
int CeilingValue = 620;
int step = 3;
int hypothesis_distance = 40;
bool enable_buzzer = false;
bool verbose = false;
bool isScanning = false;
struct HuskyData {
  int angle;
  bool isFound;
};

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (((x-in_min)/in_max) * (out_max));
}

void PinUltraInitial(int echo,int triger){  
  pinMode(echo,INPUT);
  pinMode(triger,OUTPUT);

}

void ServoInit(int pin){
  myServo.attach(pin);
  myServo.write(90); 
}

void force_initial_servo(){
  myServo.write(90); 
}

float calculate_distance(int echo,int triger){
  digitalWrite(triger,LOW);
  delayMicroseconds(2);
  digitalWrite(triger,HIGH);
  delayMicroseconds(10);
  digitalWrite(triger,LOW);

  duration = pulseIn(echo, HIGH);
  cm = (duration / 29) /2 ;
  // delay(100);
  return cm ;
}

HuskyData huskylens_getvalue(int initial = 30){
    if (!huskylens.request() || !huskylens.isLearned() || !huskylens.available()) {
        if (verbose){
        Serial.println(F("Fail to request objects from HUSKYLENS!"));
        Serial.println(F("Object not learned!")); 
        Serial.println(F("Object disappeared!"));
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scanning...");

        return {90,false};
    
    } else {
        // Serial.println("Object found");
        HUSKYLENSResult result = huskylens.read();

        // The HuskyLens screen is 320 pixels wide. 
        // We map the object's X coordinate (0 to 320) to a servo angle (180 to 0).
        int targetAngle = map(result.xCenter, 0, 320, 180, 0);
        
        int moving_distance = 320 - result.xCenter;
        targetAngle = map(moving_distance, 0, 320, 0, 180);
        // Command the servo to move to the new angle
        
        lcd.setCursor(0, 0); // (Column 0, Row 0)
        lcd.print("Intruder!");

        return {180,true};

        // printResult(result);
        }
}

void setup() {

  Serial.begin(9600);
  lcd.init();
  //lcd.noBacklight();   // ปิด backlight
  lcd.backlight();       // เปิด backlight 
  //lcd.setCursor(0, 0); // (Column 0, Row 0)
  //lcd.print("Hello, World!");
  
  // 4. Print a message on the SECOND row
  //lcd.setCursor(0, 1); // (Column 0, Row 1)
  //lcd.print("I2C is working!");
  PinUltraInitial(7,6);
  ServoInit(9);
  pinMode(LED_pin,OUTPUT);

  Wire.begin();
    while (!huskylens.begin(Wire))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(1000);
    }

  huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING); //Switch the algorithm to object tracking.


}

void loop() {
  cm = calculate_distance(echo,triger);
  Serial.println(cm);

  if (enable_buzzer) {
    if (cm < hypothesis_distance) {
      analogWrite(buzzer,150);
      Serial.println("turn on buzzer");

    } else {
      analogWrite(buzzer,255);
      Serial.println("turn off buzzer");
    }
  }
  auto [angle, is_not_valid] = huskylens_getvalue(30);
  
//   if (cm > 0 && cm < 5){
//         if (!isScanning) { // Only run this once when someone first enters the 20cm zone

//       isScanning = true;
      
//       // Simulate the verification process
//       delay(1000); // Wait 2 seconds for "AI Processing"
      
//       // handleVerification(); 
//     } else {
//     // Reset the system when the person moves away
//     if (isScanning) {
//       lcd.clear();
//       lcd.print("System Ready");
//       isScanning = false;
//     }
//   }
//   delay(100);
// }

  if (is_not_valid ) {
    digitalWrite(LED_pin,HIGH);
    myServo.write(30);
    if (cm < hypothesis_distance){
      analogWrite(buzzer,150);
      Serial.println("turn on buzzer");

      delay(500); // need to use timer instead this is only prototype
      analogWrite(buzzer,255);
      Serial.println("turn off buzzer");

    }

      

  } else {
    digitalWrite(LED_pin,LOW);
    myServo.write(angle);

    if (cm < hypothesis_distance){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Not an intruder...");
    }

  }
  delay(1000);


}
