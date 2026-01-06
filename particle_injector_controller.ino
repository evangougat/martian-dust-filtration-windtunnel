/**
 * Dust Injector Control — Servo + Vibration Motor
 *
 * Controls the particle injection mechanism for a low-velocity wind tunnel
 * used in Martian dust filtration experiments.
 *
 * Operation:
 *  - A vibration motor (driven via PWM on a buzzer pin) pulses periodically
 *    to release fine regolith simulant.
 *  - A servo-actuated gate briefly closes immediately after each vibration
 *    pulse to prevent cascading particle release, then reopens shortly
 *    before the next pulse.
 *
 * Pulse timing, servo positions, and total cycle count are configurable
 * to control injected dust mass and repeatability.
 */

#include <Servo.h>

Servo injectorServo;

/* =========================
   Pin Assignments
   ========================= */
const int SERVO_PIN  = 6;    // Servo signal pin
const int BUZZER_PIN = 11;   // PWM-capable pin (avoid 9/10 on Uno with Servo library)

/* =========================
   Servo Positions (degrees)
   ========================= */
const int SERVO_OPEN_DEG  = 123; // Normal (open) position
const int SERVO_CLOSE_DEG = 130; // Brief closed position after pulse

/* =========================
   Vibration Pulse Parameters
   ========================= */
const uint8_t VIBE_PWM            = 255;   // Vibration motor intensity (0–255)
const unsigned long VIBE_ON_MS    = 500;   // Vibration ON duration
const unsigned long VIBE_OFF_MS   = 1500;  // Time between pulses
const int PULSE_CYCLES            = 1100;  // Total number of vibration pulses

/* =========================
   Servo Timing
   ========================= */
// Time before the next vibration pulse that the servo reopens
const unsigned long REOPEN_LEAD_MS = 250;

/* =========================
   Setup
   ========================= */
void setup() {
  Serial.begin(9600);

  injectorServo.attach(SERVO_PIN);
  injectorServo.write(SERVO_OPEN_DEG);

  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 0);

  delay(11000); // Optional startup delay (system stabilization)
  Serial.println("Dust injector initialized. Beginning pulse sequence...");
}

/* =========================
   Main Loop
   ========================= */
void loop() {
  for (int cycle = 0; cycle < PULSE_CYCLES; cycle++) {

    Serial.print("Pulse ");
    Serial.print(cycle + 1);
    Serial.print(" / ");
    Serial.println(PULSE_CYCLES);

    // --- Vibration ON ---
    analogWrite(BUZZER_PIN, VIBE_PWM);
    delay(VIBE_ON_MS);

    // --- Vibration OFF ---
    analogWrite(BUZZER_PIN, 0);

    // Immediately close injector gate to prevent dust cascading
    injectorServo.write(SERVO_CLOSE_DEG);

    // Hold closed until shortly before next vibration pulse
    unsigned long closedHold =
      (VIBE_OFF_MS > REOPEN_LEAD_MS) ? (VIBE_OFF_MS - REOPEN_LEAD_MS) : 0;
    delay(closedHold);

    // Reopen injector gate prior to next pulse
    injectorServo.write(SERVO_OPEN_DEG);

    // Final wait before next vibration cycle
    unsigned long reopenWait =
      (VIBE_OFF_MS > REOPEN_LEAD_MS) ? REOPEN_LEAD_MS : VIBE_OFF_MS;
    delay(reopenWait);
  }

  // End of experiment — shut down vibration motor
  analogWrite(BUZZER_PIN, 0);
  Serial.println("Pulse sequence complete.");

  // Halt execution
  while (true) { /* intentional stop */ }
}
