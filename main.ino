#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Initialize I2C LCD (Address: 0x27 or 0x3F, 16x2 chars)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize SoftwareSerial for Bluetooth (Pins: RX, TX)
SoftwareSerial BTSerial(2, 3); 

// Sensor Pin Definitions
const int PULSE_PIN = A0;   // Heartbeat / Pulse sensor
const int BP_PIN = A1;      // Simulated Blood Pressure sensor (e.g., analog pressure cuff interface)
const int IR_PIN = 4;       // IR Obstacle / Patient presence sensor
const int LED_PIN = 13;     // Status Indicator LED

// Variables for readings
int pulseValue = 0;
int bpValue = 0;
int irState = HIGH;
unsigned long lastSendTime = 0;
const long interval = 2000; // Send Bluetooth update every 2 seconds

void setup() {
  // Start Serial Monitor for debugging
  Serial.begin(9600);
  
  // Start Bluetooth communication
  BTSerial.begin(9600);

  // Initialize pins
  pinMode(PULSE_PIN, INPUT);
  pinMode(BP_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initialize LCD display
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Health Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read sensor values
  pulseValue = analogRead(PULSE_PIN);
  bpValue = analogRead(BP_PIN);
  irState = digitalRead(IR_PIN);

  // Map analog readings to realistic display metrics (adjust based on your specific sensor calibration)
  int heartRate = map(pulseValue, 0, 1023, 50, 140); // Simulated BPM range
  int systolicBP = map(bpValue, 0, 1023, 90, 180);   // Simulated Systolic BP range

  // Check patient presence via IR sensor
  if (irState == LOW) { // Assuming active low for IR sensor detection
    digitalWrite(LED_PIN, HIGH);
    
    // Update LCD Display
    lcd.setCursor(0, 0);
    lcd.print("HR: ");
    lcd.print(heartRate);
    lcd.print(" bpm   ");

    lcd.setCursor(0, 1);
    lcd.print("BP: ");
    lcd.print(systolicBP);
    lcd.print(" mmHg  ");

    // Send real-time data via Bluetooth at defined intervals
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= interval) {
      lastSendTime = currentTime;
      
      BTSerial.print("HEART_RATE:");
      BTSerial.print(heartRate);
      BTSerial.print(",BP_SYS:");
      BTSerial.print(systolicBP);
      BTSerial.println(",STATUS:OK");
      
      // Mirror to Serial Monitor
      Serial.print("Sent -> HR: ");
      Serial.print(heartRate);
      Serial.print(" | BP: ");
      Serial.println(systolicBP);
    }
  } else {
    // Patient not detected
    digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Patient Absent ");
    lcd.setCursor(0, 1);
    lcd.print("Waiting...     ");
    
    BTSerial.println("STATUS:PATIENT_ABSENT");
  }

  delay(200); // Small refresh delay
}
