/* a closed loop proportional control
   ms 20200926
*/
// Include Libraries
#include <PinChangeInt.h>

////////////////////////////////////////////////////
// Copy constants and definitions from Lab 3
////////////////////////////////////////////////////

//PREPROCESSOR DEFINITIONS
#define FORWARD  0
#define LEFT     1
#define RIGHT   -1
#define pushButton 2
#define A        1
#define B        2
#define pwmA    3
#define dirA    12
#define pwmB    11
#define dirB    13
#define SHIELD true
#define MOVESIZE 50

// Lab Specific definitions

// Defining these allows us to use letters in place of binary when
// controlling our motors
#define pushButton 2 
// install a Pullup button with its output into Pin 2
/* If you'd like to use additional buttons as bump sensors, define their pins 
 *  as descriptive names, such as bumperLeft etc. 
 */

// Drive constants - dependent on robot configuration
#define EncoderCountsPerRev 42.0
#define DistancePerRev      26.1
#define wheelDiameter       8.31

//robot dimensions
#define track               12.4

//These are to build your moves array, a la Lab 2
#define FORWARD             0
#define LEFT                1
#define RIGHT              -1
#define NONE               999

// these next two are the digital pins we'll use for the encoders
// You may change these as you see fit.
#define EncoderMotorLeft  7
#define EncoderMotorRight 8

//A IS LEFT
//B IS RIGHT
// Proportional Control constants
// what are your ratios of PWM:Encoder Count error?
#define GAIN_A 3.25
#define GAIN_B 3.5
// how many encoder counts from your goal are accepteable?
#define forwardTolerance 2
#define turnTolerance 1

//turning constant
#define turnConst 0
#define turnGainConst 1.0

// minimum power settings
// Equal to the min PWM for your robot's wheels to move
// May be different per motor
#define deadband_A 100
#define deadband_B 100

//DEBUGGGGGGGINGGGG
boolean debug = true;
boolean drinkDebug = true;
boolean irdebug = false;

// TURN ON STEADY STATE ERROR
#define steadyErrorOn false

// Lab specific variables
volatile unsigned int leftEncoderCount = 0;
volatile unsigned int rightEncoderCount = 0;
volatile int leftSteadyError = 0;
volatile int rightSteadyError = 0;
////////////////////////////////////////////////////
// Copy constants and definitions from Lab 3
////////////////////////////////////////////////////

int routeMoves[] = {500, LEFT, 20, RIGHT, 60};
int moveSize = 0;

// Define IR Distance sesnsor Pins
#define frontIR A0
#define sideIR  A1

// Define piezo element 
#define piezo A2
#define forceThresh 300
boolean drinkOn = false;

// Define flex sensor
#define flex A3
#define flexThresh 830
boolean isFlexed = false;

#define ledPin 6
#define buzzPin  9

// Define the distance tolerance that indicates a wall is present
#define wallToSide 16.0 //cm
#define minDistToWall 7.0
#define boxConstant 0.85

//CUSTOM UTILITY FUNCTIONS
String moveMap(int m){
  if(m == FORWARD)
      return "F";
  else if(m == LEFT)
    return "L";
  else if(m == RIGHT)
    return "R";
  return "N"; 
}

void printMovesArray(int arr[], int arrSize){
  Serial.print("[");
  for(int i = 0; i < arrSize; i++){
    Serial.print(i);
    if(i < arrSize - 1)
      Serial.print(", ");
  }
  Serial.println("]");
  Serial.print("[");
  for(int i = 0; i < arrSize; i++){
    int temp = arr[i];
    Serial.print(moveMap(temp));
    if(i >= 10)
      Serial.print(" ");
    if(i < arrSize - 1){
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

void smartPrint(String args[], int s){
  for(int i = 0; i < s; i++){
    if(i < s -1)
      Serial.print(args[i]);
    else
      Serial.println(args[i]);
  }
}

///////////////////////////////////////////////
// SENSOR READING FUNCTIONS
float readFrontDist() { 
  // If IR distance sensor
  int reading = analogRead(frontIR);
  float dist = 1 / ((float(reading) - 1.7559) / 5684.3);
  return dist;
}
float readSideDist() {
  // If IR distance sensor
  int reading = analogRead(sideIR);
  float dist = 1 / ((float(reading) - 40.293) / 4453.2);
  return dist;
}
int readFlexSensor(){
  return analogRead(flex);
}
int readForceSensor(){
  return analogRead(piezo);
}
/////////////////////////////////////////
void waitForFlex(){
  while(1){ //wait for flex to be greater than threshold
    int flex = readFlexSensor();
    if(drinkDebug){
      Serial.print("Flex sensor value: ");
      Serial.println(flex);
    }
    if(flex > flexThresh)
      break;
  }
}

void waitForForce(){
  while(1){ //wait for force to be greater than threshold
    int force = readForceSensor();
    if(drinkDebug){
      Serial.print("Force sensor value: ");
      Serial.println(force);
    }
    if(force > forceThresh)
      break;
  }
}

void waitForUnflex(){
  while(1){ //wait for flex to be unflexed
    int flex = readFlexSensor();
    if(drinkDebug){
      Serial.print("Flex sensor value: ");
      Serial.println(flex);
    }
    if(flex < flexThresh)
      break;
  }
}

void detectDrinkPut(){
  digitalWrite(ledPin, HIGH);
  waitForFlex();
  waitForForce();
  digitalWrite(ledPin, LOW);
}

void detectDrinkTaken(){
  digitalWrite(ledPin, HIGH);
  waitForForce();
  waitForUnflex();
  digitalWrite(ledPin, LOW);
}

void setup() {
  // set stuff up
  Serial.begin(9600);
  motor_setup();
  pinMode(pushButton, INPUT_PULLUP);
  // add additional pinMode statements for any bump sensors
  // Attaching Wheel Encoder Interrupts
  Serial.print("Encoder Testing Program ");
  Serial.print("Now setting up the Left Encoder: Pin ");
  Serial.print(EncoderMotorLeft);
  Serial.println();
  pinMode(EncoderMotorLeft, INPUT_PULLUP); //set the pin to input
  PCintPort::attachInterrupt(EncoderMotorLeft, indexLeftEncoderCount, CHANGE);
  Serial.print("Now setting up the Right Encoder: Pin ");
  Serial.print(EncoderMotorRight);
  Serial.println();
  pinMode(EncoderMotorRight, INPUT_PULLUP);     //set the pin to input
  PCintPort::attachInterrupt(EncoderMotorRight, indexRightEncoderCount, CHANGE);
  
  tone(buzzPin, 329.63, 300);

  Serial.println("Finished Setup");
} /////////////// end of setup ////////////////////////////////////

/////////////////////// loop() ////////////////////////////////////
void loop()
{ 
  digitalWrite(ledPin, HIGH);
  while (digitalRead(pushButton) == 1){ // wait for button push
    tone(buzzPin, 50, 50);
  }
  while (digitalRead(pushButton) == 0); // wait for button release
  digitalWrite(ledPin, LOW);
  Serial.println("Executing Loop");
  while (1) { //Inifnite number of runs, so you don't have to re-explore everytime a mistake happens
    Serial.println("Waiting for a drink");
    detectDrinkPut();
    tone(buzzPin, 329.63, 300);
    Serial.println("Acquired a drink");
    delay(2000);
    
    Serial.println("Running the route");
    runRoute();
    tone(buzzPin, 329.63, 300);
    Serial.println("Finished running the route");
    delay(2000);
    
    Serial.println("Waiting for drink to be taken");
    detectDrinkTaken();
    tone(buzzPin, 329.63, 300);
    Serial.println("Drink has been taken");
    delay(2000);
    
    Serial.println("Playing fur elise");
    playShortFurElise();
    Serial.println("Finished played fur elise");
    delay(2000);
    
    Serial.println("Running reverse route");
    runReverseRoute();
    turnAround();
    Serial.println("Finished running reverse route");
  }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// end of loop() /////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void runRoute() {
  for(int i = 0; i < sizeof(routeMoves)/2; i++){
  // copy for loop from Lab 3 to run through finished maze path
    if(routeMoves[i]==LEFT){
      turnLeft();
    }
    else if(routeMoves[i]==RIGHT){
      turnRight();      
    }
    else if(routeMoves[i] == FORWARD){
      driveOneBox();
    }
    else if(routeMoves[i] == NONE){
      stop_motors();
    }
    else{
      drive(routeMoves[i]);
    }
    stop_motors();
    delay(500);
  } 
  stop_motors();
  Serial.print("All done");
  delay(2000);
}

void runReverseRoute(){
  turnAround();
  delay(1000);
  runRoute();
}
