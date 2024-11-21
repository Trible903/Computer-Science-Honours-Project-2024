#include <math.h>

float measureDistance(struct Station station);
float norm(struct Position p);
struct Position trilateration(struct Station stationOne, struct Station stationTwo, struct Station stationThree);
void emitDisruptiveSignal(int trigPin, float duration);

//CONSTANT PIN INFORMATION
#define BUTTON_PIN 3
#define DOS_PIN 4
#define SOUND_SPEED 0.0344  // Speed of sound in cm/us
#define TRIG_PIN_ONE 8
#define ECHO_PIN_ONE 9
#define TRIG_PIN_TWO 11
#define ECHO_PIN_TWO 10
#define TRIG_PIN_THREE 6
#define ECHO_PIN_THREE 5
#define SCALE_RATIO 15
#define SIGNAL_DURATION 5000  // Change this value to set different durations

//DEBUG effects the output
#define DEBUG 2

//ATTACK determines the attack type
// 0 == no attack
// 1 == constant signal
#define ATTACK 1

//CONSTANT POSITIONAL DATA
#define X1 0
#define Y1 0
#define X2 4
#define Y2 2
#define X3 2
#define Y3 4

enum SensorState { WAITING,
                   MEASURING };

struct Position {
  float x;
  float y;
};

struct Station {
  int trigPin;
  int echoPin;
  enum SensorState state;
  float distance;
  struct Position pos;
};


struct Station stationOne = { TRIG_PIN_ONE, ECHO_PIN_ONE, WAITING, 0, { X1, Y1 } };
struct Station stationTwo = { TRIG_PIN_TWO, ECHO_PIN_TWO, WAITING, 0, { X2, Y2 } };
struct Station stationThree = { TRIG_PIN_THREE, ECHO_PIN_THREE, WAITING, 0, { X3, Y3 } };

struct Position objPosition = { 0, 0 };
bool buttonPressed = false;
unsigned long previousMillis = 0;  // Store the last time the signal was emitted
bool signalActive = false;         // Track if the signal is currently active

void setup() {
  Serial.begin(9600);  // Starts the serial communication

  //DOS Pin/Sensor
  pinMode(BUTTON_PIN, INPUT);
  pinMode(DOS_PIN, OUTPUT);

  //Station Pins
  pinMode(stationOne.trigPin, OUTPUT);    // Sets the trigPin as an Output
  pinMode(stationOne.echoPin, INPUT);     // Sets the echoPin as an Input
  pinMode(stationTwo.trigPin, OUTPUT);    // Sets the trigPin as an Output
  pinMode(stationTwo.echoPin, INPUT);     // Sets the echoPin as an Input
  pinMode(stationThree.trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(stationThree.echoPin, INPUT);   // Sets the echoPin as an Input

  stationOne.state = MEASURING;
}

void loop() {
  //CONSTANT DOS ATTACK
  switch (ATTACK) {
    case 0:
      break;
    case 1:
      //Constant DOS
      buttonPressed = digitalRead(BUTTON_PIN);
      if (buttonPressed) {  
        // Activate ultrasonic signal for DoS (Denial of Service)
        //digitalWrite(DOS_PIN, HIGH);  // Activate signal while button is pressed
        emitDisruptiveSignal(DOS_PIN);
      } 
      break;
  }

  if (stationOne.state == MEASURING) {
    stationOne.distance = measureDistance(stationOne);
    stationOne.state = WAITING;
    stationTwo.state = MEASURING;
    stationThree.state = WAITING;
  } else if (stationTwo.state == MEASURING) {
    stationTwo.distance = measureDistance(stationTwo);
    stationOne.state = WAITING;
    stationTwo.state = WAITING;
    stationThree.state = MEASURING;
  } else {
    stationThree.distance = measureDistance(stationThree);
    stationOne.state = MEASURING;
    stationTwo.state = WAITING;
    stationThree.state = WAITING;
  }

  //objPosition = trilateration(stationOne, stationTwo, stationThree);
  objPosition = computeCoordinates2D(stationOne, stationTwo, stationThree);

  if (DEBUG == 1) {
    Serial.print("Distance One: ");
    Serial.print(stationOne.distance);
    Serial.print(" Distance Two: ");
    Serial.print(stationTwo.distance);
    Serial.print(" Distance Three: ");
    Serial.println(stationThree.distance);
  } else if (DEBUG == 2) {
    Serial.print(objPosition.x);
    Serial.print(",");
    Serial.print(objPosition.y);
    Serial.print(",");
    Serial.print(buttonPressed);
    Serial.print(",");
    Serial.print(stationOne.distance);
    Serial.print(",");
    Serial.print(stationTwo.distance);
    Serial.print(",");   
    Serial.println(stationThree.distance);
  } else {
    Serial.print(objPosition.x);
    Serial.print(",");
    Serial.println(objPosition.y);
  }
}

float measureDistance(struct Station station) {
  digitalWrite(station.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(station.trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(station.trigPin, LOW);

  long duration = pulseIn(station.echoPin, HIGH);
  return duration * SOUND_SPEED / (2 * 15);  // Convert to cm
}

void emitDisruptiveSignal(int trigPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}


struct Position computeCoordinates2D(struct Station stationOne, struct Station stationTwo, struct Station stationThree) {
    struct Position result;

    // Coordinates of the known points
    double x1 = stationOne.pos.x, y1 = stationOne.pos.y;
    double x2 = stationTwo.pos.x, y2 = stationTwo.pos.y;
    double x3 = stationThree.pos.x, y3 = stationThree.pos.y;

    double r1 = stationOne.distance;  
    double r2 = stationTwo.distance;
    double r3 = stationThree.distance;

    // Calculate terms for the linear equations
    double A = 2 * (x2 - x1);
    double B = 2 * (y2 - y1);
    double C = r1 * r1 - r2 * r2 - x1 * x1 - y1 * y1 + x2 * x2 + y2 * y2;

    double D = 2 * (x3 - x1);
    double E = 2 * (y3 - y1);
    double F = r1 * r1 - r3 * r3 - x1 * x1 - y1 * y1 + x3 * x3 + y3 * y3;

    // Solve the system of linear equations for y
    result.y = (F * A - C * D) / (E * A - B * D);

    // Solve for x using the value of y
    result.x = (C - B * result.y) / A;

    return result;
}

/*
float norm(struct Position p)  // get the norm of a vector
{
  return pow(pow(p.x, 2) + pow(p.y, 2), .5);
}
*/

/*
struct Position trilateration(struct Station stationOne, struct Station stationTwo, struct Station stationThree) {

  //possible reimplement this on my own?

  struct Position resultPose;
  //unit vector in a direction from point1 to point 2
  float p2p1Distance = pow(pow(stationTwo.pos.x - stationOne.pos.y, 2) + pow(stationTwo.pos.y - stationOne.pos.y, 2), 0.5);
  struct Position ex = { (stationTwo.pos.x - stationOne.pos.x) / p2p1Distance, (stationTwo.pos.y - stationOne.pos.y) / p2p1Distance };
  struct Position aux = { stationThree.pos.x - stationOne.pos.x, stationThree.pos.y - stationOne.pos.y };
  //signed magnitude of the x component
  float i = ex.x * aux.x + ex.y * aux.y;
  //the unit vector in the y direction.
  struct Position aux2 = { stationThree.pos.x - stationOne.pos.x - i * ex.x, stationThree.pos.y - stationOne.pos.y - i * ex.y };
  struct Position ey = { aux2.x / norm(aux2), aux2.y / norm(aux2) };
  //the signed magnitude of the y component
  float j = ey.x * aux.x + ey.y * aux.y;
  //coordinates
  float x = (pow(stationOne.distance / SCALE_RATIO, 2) - pow(stationTwo.distance / SCALE_RATIO, 2) + pow(p2p1Distance, 2)) / (2 * p2p1Distance);
  float y = (pow(stationOne.distance / SCALE_RATIO, 2) - pow(stationThree.distance / SCALE_RATIO, 2) + pow(i, 2) + pow(j, 2)) / (2 * j) - i * x / j;
  //result coordinates
  float finalX = stationOne.pos.x + x * ex.x + y * ey.x;
  float finalY = stationOne.pos.y + x * ex.y + y * ey.y;
  resultPose.x = finalX;
  resultPose.y = finalY;
  return resultPose;
}
*/
