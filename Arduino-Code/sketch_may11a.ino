
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/sleep.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int trigPin =0;
const int echoPin = 2;
const int led1 = 10; // اتنقل من D4
const int led2 = 9; // اتنقل من D5
const int led3 = 8;
const int buzzer = 7;
const int pushButton = 6;

const int comPin = 4; // الكومون المشترك → OUTPUT LOW دايما
const int side1 = 3; // الناحية الأولى → Sleep (INT1)
const int side2 = 5; // الناحية التانية → Awake

long duration;
int distance;
unsigned long previousMillis = 0;
unsigned long buttonPrevMillis = 0;
bool blinkState = false;
bool showInches = false;

// ===================== ISR =====================

void wakeUp() { } // بتصحي الـ CPU بس

// ===================== Helpers =====================

void allOff() {
digitalWrite(led1, LOW);
digitalWrite(led2, LOW);
digitalWrite(led3, LOW);
digitalWrite(buzzer, LOW);
}

void initDisplay() {
Wire.begin();
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.setTextColor(WHITE);
display.setTextSize(2);
}

void showMessage(const char* msg) {
display.clearDisplay();
display.setCursor(0, 20);
display.print(msg);
display.display();
}

void setup() {
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);
pinMode(led1, OUTPUT);
pinMode(led2, OUTPUT);
pinMode(led3, OUTPUT);
pinMode(buzzer, OUTPUT);
pinMode(pushButton, INPUT_PULLUP);
pinMode(12,OUTPUT);
pinMode(1,OUTPUT);
digitalWrite(12,HIGH);
digitalWrite(1,HIGH);

// السلايد سويتش
pinMode(comPin, OUTPUT);
digitalWrite(comPin, LOW); // الكومون دايما LOW
pinMode(side1, INPUT_PULLUP); // الناحية Sleep
pinMode(side2, INPUT_PULLUP); // الناحية Awake

initDisplay();

showMessage("System ON");
delay(2000);

// INT1 على D3 (side1) - يصحى لما السويتش يبعد عن Side1
attachInterrupt(digitalPinToInterrupt(side1), wakeUp, CHANGE);
}

// ===================== Loop =====================

void loop() {

// --- Sleep: لما السويتش يوصل COM بـ Side1 → D3 يبقى LOW ---
if (digitalRead(side1) == LOW) {
showMessage("System OFF");
delay(500);
allOff();


set_sleep_mode(SLEEP_MODE_PWR_DOWN);
sleep_enable();
sleep_mode(); // بينام هنا
sleep_disable(); // بيصحى لما يزحلق للناحية التانية

// إعادة تشغيل I2C بعد الصحيان
initDisplay();
showMessage("System ON");
delay(1000);
return;
}

// --- Push Button: تبديل cm / inch ---
if (digitalRead(pushButton) == LOW) {
unsigned long now = millis();
if (now - buttonPrevMillis >= 300) {
buttonPrevMillis = now;
showInches = !showInches;
showMessage(showInches ? ">> inch" : ">> cm");

delay(600);
}
}

// --- Ultrasonic ---
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH, 25000);
distance = duration * 0.034 / 2;

display.clearDisplay();
display.setTextSize(2);
display.setCursor(0, 0);

if (distance > 0 && distance <= 60) {

if (showInches) {
float inches = distance / 2.54;
display.print("D:");
display.print(inches, 1);
display.print("in");
} else {
display.print("Dist:");
display.print(distance);
display.print("cm");
}

// Zones (بتشتغل على cm دايما)
if (distance <= 50 && distance > 30) {
digitalWrite(led1, HIGH);
digitalWrite(led2, LOW);
digitalWrite(led3, LOW);
digitalWrite(buzzer, LOW);
}
else if (distance <= 30 && distance > 20) {
digitalWrite(led1, LOW);

digitalWrite(led2, HIGH);
digitalWrite(led3, LOW);
digitalWrite(buzzer, LOW);
}
else if (distance <= 20 && distance > 10) {
digitalWrite(led1, LOW);
digitalWrite(led2, LOW);
digitalWrite(led3, HIGH);
digitalWrite(buzzer, LOW);
}
else if (distance <= 10) {
digitalWrite(led1, LOW);
digitalWrite(led2, LOW);
unsigned long now = millis();
if (now - previousMillis >= 150) {
previousMillis = now;
blinkState = !blinkState;
digitalWrite(led3, blinkState);
digitalWrite(buzzer, blinkState);
}

}
else {
allOff();
}

} else {
display.print("Out of");
display.setCursor(0, 22);
display.print("range");
allOff();
}

display.display();
delay(50);
}
