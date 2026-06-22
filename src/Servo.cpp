#include "Servo.h"

float currentAngle = 0.0;
int angleToOpen = 160;

void setupServo()
{
    // Initialize the servo object and attach it to the specified pin
    atmServo.attach(SERVO_PIN, 700, 2300);
    atmServo.write(0); // Start with the servo in the closed position
    currentAngle = 0.0;
    atmServo.detach(); // Detach the servo to save power until it's needed
}

void openAtmDoor()
{
    moveServoToPosition(angleToOpen);
}

void closeAtmDoor()
{
    moveServoToPosition(0);
}

void moveServoToPosition(int angle)
{
    // if (angle < 0 || angle > 180)
    // {
    //     Serial.println("Invalid angle. Must be between 0 and 180.");
    //     return;
    // }
    atmServo.attach(SERVO_PIN, 700, 2300); // Re-attach the servo if it was detached
    atmServo.write(angle);
    currentAngle = angle;
    Serial.print("Moved servo to angle: ");
    Serial.println(angle);
    delay(1500);       // Wait for the servo to move to the position
    atmServo.detach(); // Detach the servo to save power
}

void handleServoSerialInput()
{
    if (Serial.available() > 0)
    {
        int angle = Serial.parseInt();
        // Clear any remaining characters in the buffer
        while (Serial.available() > 0)
        {
            Serial.read();
        }
        moveServoToPosition(angle);
    }
}