/******************************************************************************
  Author: Smartbuilds.io
  YouTube: https://www.youtube.com/channel/UCGxwyXJWEarxh2XWqvygiIg
  Fork your own version: https://github.com/EbenKouao/arduino-robot-arm
  Check out the full article: https://smartbuilds.io/diy-arduino-robot-arm-controlled-hand-gestures/
  Date: 06/01/2021
  Robot Arm
  Version 1.0
  Creator: smartbuilds.io
  Description: Robotic Arm Mark II - Servo Motor

  To use the module connect it to your Arduino as follows:

  PCA9685...........Uno/Nano
  GND...............GND
  OE................N/A
  SCL...............A5
  SDA...............A4
  VCC...............5V

******************************************************************************/

/* Include the HCPCA9685 library */
#include "HCPCA9685.h" 

/* I2C slave address for the device/module. For the HCMODU0097 the default I2C address
   is 0x40 */
#define  I2CAdd 0x40

/* Create an instance of the library */
HCPCA9685 HCPCA9685(I2CAdd);

/* Port information */
#define GRIPPER 5
#define WRIST_2 4
#define WRIST_1 3
#define SHOULDER 2
#define ELBOW_R 1
#define ELBOW_L 0

//initial parking position of the motor
const int servo_elbow_L_parking_pos = 60; // elbow
const int servo_elbow_R_parking_pos = 60; // ??
const int servo_shoulder_parking_pos = 70; // shoulder
const int servo_wrist_1_parking_pos = 47; // wrist joint 1
const int servo_wrist_2_parking_pos = 63; // wrist joint 2
const int servo_gripper_parking_pos = 63; // gripper

//Degree of robot servo sensitivity - Intervals
int servo_elbow_L_pos_increment = 20;
int servo_elbow_R_pos_increment = 20;
int servo_shoulder_pos_increment = 20;
int servo_wrist_1_pos_increment = 50;
int servo_wrist_2_pos_increment = 60;
int servo_gripper_pos_increment = 40;

//Keep track of the current value of the motor positions
int servo_elbow_L_parking_pos_i = servo_elbow_L_parking_pos;
int servo_elbow_R_parking_pos_i = servo_elbow_R_parking_pos;
int servo_shoulder_parking_pos_i = servo_shoulder_parking_pos;
int servo_wrist_1_parking_pos_i = servo_wrist_1_parking_pos;
int servo_wrist_2_parking_pos_i = servo_wrist_2_parking_pos;
int servo_gripper_parking_pos_i = servo_gripper_parking_pos;


//Minimum and maximum angle of servo motor
int servo_elbow_L_min_pos = 10;
int servo_elbow_L_max_pos = 180;

int servo_elbow_R_min_pos = 10;
int servo_elbow_R_max_pos = 180;

int servo_shoulder_min_pos = 10;
int servo_shoulder_max_pos = 400;

int servo_wrist_1_min_pos = 10;
int servo_wrist_1_max_pos = 380;

int servo_wrist_2_min_pos = 10;
int servo_wrist_2_max_pos = 380;

int servo_gripper_min_pos = 10;
int servo_gripper_max_pos = 120;

int servo_L_pos = 0;
int servo_R_pos = 0;
int servo_shoulder_pos = 0;
int servo_wrist_1_pos = 0;
int servo_wrist_2_pos = 0;
int servo_gripper_pos = 0;

char state = 0; // Changes value from ASCII to char
int response_time = 5;
int response_time_4 = 2;
int loop_check = 0;
int response_time_fast = 20;
int action_delay = 600;

//Posiion of motor for example demos
unsigned int Pos;

// Define pin connections & motor's steps per revolution
const int dirPin = 4;
const int stepPin = 5;
const int stepsPerRevolution = 120;
int stepDelay = 4500;
const int stepsPerRevolutionSmall = 60;
int stepDelaySmall = 9500;

// Define Count
int count_loop = 0;
int count_nothing = 0;

void setup() {
  // Declare pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  /* Initialise the library and set it to 'servo mode' */
  HCPCA9685.Init(SERVO_MODE);

  /* Wake the device up */
  HCPCA9685.Sleep(false);

  Serial.begin(9600); // Initialise default communication rate of the Bluetooth module

  delay(3000);
  //wakeUp(); -- Uncomment for Example Demo 1
  //flexMotors(); -- Uncomment for Example Demo 1

}


void loop() {

  if (Serial.available() > 0) { // Checks whether data is coming from the serial port

    state = Serial.read(); // Reads the data from the serial port
    Serial.print(state); // Prints out the value sent

    count_loop = count_loop + 1;

    if (count_loop > 50) {
      count_loop = 0;
      count_nothing = 0;
    }

    if (state == 'nothing') {
      count_nothing = count_nothing + 1;
    }

    if (count_nothing > 30) { // robot-arm position return
      HCPCA9685.Servo(GRIPPER, servo_gripper_parking_pos); // gripper return
      delay(response_time);
      HCPCA9685.Servo(WRIST_2, servo_wrist_2_parking_pos); // wrist 2 return
      delay(response_time_4);
      HCPCA9685.Servo(WRIST_1, servo_wrist_1_parking_pos); // wrist 1 return
      delay(response_time);
      HCPCA9685.Servo(ELBOW_L, servo_elbow_L_parking_pos); // elbow return
      HCPCA9685.Servo(ELBOW_R, (servo_elbow_L_max_pos - servo_elbow_L_parking_pos));
      delay(response_time);
      HCPCA9685.Servo(SHOULDER, servo_shoulder_parking_pos); // shoulder return
      delay(response_time);
    }

    if (state == 'right'){ 
      baseRotateRight();
      delay(response_time);
    }

    if (state == 'left') { 
      baseRotateLeft();
      delay(response_time);
    }

    if (state == 'up') { 
       shoulderServoUp();
      delay(response_time);
    }

    if (state == 'down') { 
       shoulderServoDown();
      delay(response_time);
    }

    if (state == 'grip_close') {
      gripperServoClose();
      delay(response_time);
    }

    if (state == 'grip_open') {
      gripperServoOpen();
      delay(response_time);
    }
  }
}

//Boiler plate function - These functions move the servo motors in a specific direction for a duration.

void gripperServoClose() {  // Close Claw Grip

  if (servo_gripper_parking_pos_i > servo_gripper_min_pos) {
    HCPCA9685.Servo(GRIPPER, servo_gripper_parking_pos_i);
    delay(response_time); //Delay the time takee to turn the servo by the given increment
    Serial.println(servo_gripper_parking_pos_i);
    servo_gripper_parking_pos_i = servo_gripper_parking_pos_i - servo_gripper_pos_increment;
  }
}

void gripperServoOpen() {  // Open Claw Grip

  if (servo_gripper_parking_pos_i < servo_gripper_max_pos) {
    HCPCA9685.Servo(GRIPPER, servo_gripper_parking_pos_i);
    delay(response_time);
    Serial.println(servo_gripper_parking_pos_i);
    servo_gripper_parking_pos_i = servo_gripper_parking_pos_i + servo_gripper_pos_increment;

  }

}

void wristServo2CW() {  // wrist 2 Clock-wise

  if (servo_wrist_2_parking_pos_i > servo_wrist_2_min_pos) {
    HCPCA9685.Servo(WRIST_2, servo_wrist_2_parking_pos_i);
    delay(response_time_4);
    Serial.println(servo_wrist_2_parking_pos_i);
    servo_wrist_2_parking_pos_i = servo_wrist_2_parking_pos_i - servo_wrist_2_pos_increment;

  }

}

void wrist2Servo2CCW() {  // wrist 2 Counter-Clock wise

  if (servo_wrist_2_parking_pos_i < servo_wrist_2_max_pos) {
    HCPCA9685.Servo(WRIST_2, servo_wrist_2_parking_pos_i);
    delay(response_time_4);
    Serial.println(servo_wrist_2_parking_pos_i);
    servo_wrist_2_parking_pos_i = servo_wrist_2_parking_pos_i + servo_wrist_2_pos_increment;

  }

}

void  wristServo1Down() {  // Move Wrist 1 Down

  if (servo_wrist_1_parking_pos_i < servo_wrist_1_max_pos) {
    HCPCA9685.Servo(WRIST_1, servo_wrist_1_parking_pos_i);
    delay(response_time);
    Serial.println(servo_wrist_1_parking_pos_i);

    servo_wrist_1_parking_pos_i = servo_wrist_1_parking_pos_i + servo_wrist_1_pos_increment;

  }


}

void  wristServo1Up() {  // Move Wrist 1 UP

  if (servo_wrist_1_parking_pos_i > servo_wrist_1_min_pos) {
    HCPCA9685.Servo(WRIST_1, servo_wrist_1_parking_pos_i);
    delay(response_time);
    Serial.println(servo_wrist_1_parking_pos_i);

    servo_wrist_1_parking_pos_i = servo_wrist_1_parking_pos_i - servo_wrist_1_pos_increment;

  }

}


void  elbowServoDown() {  // Move Elbow Down

  if (servo_elbow_L_parking_pos_i < servo_elbow_L_max_pos) {
    HCPCA9685.Servo(ELBOW_L, servo_elbow_L_parking_pos_i);
    HCPCA9685.Servo(ELBOW_R, (servo_elbow_L_max_pos - servo_elbow_L_parking_pos_i));

    delay(response_time);
    Serial.println(servo_elbow_L_parking_pos_i);

    servo_elbow_L_parking_pos_i = servo_elbow_L_parking_pos_i + servo_elbow_L_pos_increment;
    servo_elbow_R_parking_pos_i = servo_elbow_L_max_pos - servo_elbow_L_parking_pos_i;

  }
}

void  elbowServoUp() { // Move Elbow Up
  if (servo_elbow_L_parking_pos_i > servo_elbow_L_min_pos) {
    HCPCA9685.Servo(ELBOW_L, servo_elbow_L_parking_pos_i);
    HCPCA9685.Servo(ELBOW_R, (servo_elbow_L_max_pos - servo_elbow_L_parking_pos_i));

    delay(response_time);
    Serial.println(servo_elbow_L_parking_pos_i);


    servo_elbow_L_parking_pos_i = servo_elbow_L_parking_pos_i - servo_elbow_L_pos_increment;
    servo_elbow_R_parking_pos_i = servo_elbow_L_max_pos - servo_elbow_L_parking_pos_i;

  }

}

void  shoulderServoDown() {  // Move Shoulder Down

  if (servo_shoulder_parking_pos_i < servo_shoulder_max_pos) {
    HCPCA9685.Servo(SHOULDER, servo_shoulder_parking_pos_i);
    delay(response_time);
    Serial.println(servo_shoulder_parking_pos_i);

    servo_shoulder_parking_pos_i = servo_shoulder_parking_pos_i + servo_shoulder_pos_increment;

  }

}

void  shoulderServoUp() {  // Move Shoulder Up


  if (servo_shoulder_parking_pos_i > servo_shoulder_min_pos) {
    HCPCA9685.Servo(SHOULDER, servo_shoulder_parking_pos_i);
    delay(response_time);
    Serial.println(servo_shoulder_parking_pos_i);

    servo_shoulder_parking_pos_i = servo_shoulder_parking_pos_i - servo_shoulder_pos_increment;

  }
}

void baseRotateLeft() {  // Move stepper Left
  //clockwise
  digitalWrite(dirPin, HIGH);
  // Spin motor
  for (int x = 0; x < stepsPerRevolution; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  delay(response_time); // Wait a second
}


void baseRotateRight() {  // Move stepper Right

  //counterclockwise
  digitalWrite(dirPin, LOW);
  // Spin motor
  for (int x = 0; x < stepsPerRevolution; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  delay(response_time); // Wait a second
}

void wakeUp() {

  //Pre-Program Function - Wake Up Robot on Start

  if (loop_check == 0) {

    //    //Shoulder Raise
    for (Pos = 0; Pos < 10; Pos++)
    {

      HCPCA9685.Servo(1, Pos);
      delay(response_time_fast);
    }

    //  //Move Elbow Backwards
    for (Pos = 400; Pos > 390; Pos--)
    {

      HCPCA9685.Servo(2, Pos);

      delay(response_time_fast);
    }

    //Move Wrist 1 Forward
    for (Pos = 10; Pos < 20; Pos++)
    {
      HCPCA9685.Servo(3, Pos);
      delay(response_time);
    }

    //Move Wrist 2 Backwards
    for (Pos = 380; Pos > 50; Pos--)
    {
      HCPCA9685.Servo(4, Pos);
      delay(response_time);
    }

    //Move Wrist 2 Backwards
    for (Pos = 50; Pos < 150; Pos++)
    {
      HCPCA9685.Servo(4, Pos);
      delay(response_time);
    }

    //Move Wrist 1 Forward
    for (Pos = 19; Pos < 100; Pos++)
    {
      HCPCA9685.Servo(3, Pos);
      delay(response_time);
    }
    loop_check = 0;

  }
}

void flexMotors() {

  //Example Demo Pre-program Function to Make Robot Wake Up (Motor by Motor)

  if (loop_check == 0) {

    delay(action_delay);

    //Move Wrist 1 Forward
    for (Pos = 100; Pos > 10; Pos--)
    {
      HCPCA9685.Servo(3, Pos);
      delay(10);
    }

    delay(action_delay);

    //Move Wrist 1 Forward
    for (Pos = 10; Pos < 70; Pos++)
    {
      HCPCA9685.Servo(3, Pos);
      delay(10);
    }

    delay(action_delay);

    baseRotateLeft();
    delay(action_delay);


    //Move Wrist 2 Backwards
    for (Pos = 200; Pos < 380; Pos++)
    {
      HCPCA9685.Servo(4, Pos);
      delay(10);
    }

    delay(action_delay);

    loop_check = 1;


  }
}