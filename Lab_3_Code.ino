/* a closed loop proportional control
   ms 20200926
*/
// Include Libraries
#include <PinChangeInt.h>

//A IS LEFT
//B IS RIGHT

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

// Lab Specific definitions

// Defining these allows us to use letters in place of binary when
// controlling our motors
#define pushButton 2 // install a Pullup button with its output into Pin 2
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

// these next two are the digital pins we'll use for the encoders
// You may change these as you see fit.
#define EncoderMotorLeft  7
#define EncoderMotorRight 8

// Proportional Control constants
// what are your ratios of PWM:Encoder Count error?
#define GAIN_A 3.25
#define GAIN_B 3.5
// how many encoder counts from your goal are accepteable?
#define forwardTolerance 2
#define turnTolerance 1

//turning constant
#define turnConst 1

// minimum power settings
// Equal to the min PWM for your robot's wheels to move
// May be different per motor
#define deadband_A 95
#define deadband_B 95

//DEBUGGGGGGGINGGGG
boolean debug = true;

// Lab specific variables
volatile unsigned int leftEncoderCount = 0;
volatile unsigned int rightEncoderCount = 0;
//int moves[] = {40, LEFT, 40, RIGHT, 40};
//int moves[] = {LEFT, RIGHT, LEFT, RIGHT, LEFT, RIGHT};
int moves[] = {45, RIGHT, 27, RIGHT, 52, LEFT, 54, LEFT, 26};

void setup() {
  // set stuff up
  Serial.begin(9600);
  motor_setup();
  pinMode(pushButton, INPUT_PULLUP);
  
  // add additional pinMode statements for any bump sensors
  

  // Attaching Wheel Encoder Interrupts
  Serial.print("Encoder Testing Program \n");
  Serial.print("Now setting up the Left Encoder: Pin ");
  Serial.print(EncoderMotorLeft);
  Serial.println();
  pinMode(EncoderMotorLeft, INPUT_PULLUP); //set the pin to input
  // this next line setup the PinChange Interrupt
  PCintPort::attachInterrupt(EncoderMotorLeft, indexLeftEncoderCount, CHANGE);
  // if you "really" want to know what's going on read the PinChange.h file :)
  /////////////////////////////////////////////////
  Serial.print("Now setting up the Right Encoder: Pin ");
  Serial.print(EncoderMotorRight);
  Serial.println();
  pinMode(EncoderMotorRight, INPUT_PULLUP);     //set the pin to input
  PCintPort::attachInterrupt(EncoderMotorRight, indexRightEncoderCount, CHANGE);
} /////////////// end of setup ////////////////////////////////////

/////////////////////// loop() ////////////////////////////////////
void loop()
{
  while (digitalRead(pushButton) == 1){
    if(debug){
      Serial.print("LEFT ENCODER COUNT");
      Serial.print(leftEncoderCount);
      Serial.print(" ");
      Serial.print("RIGHT ENCODER COUNT");
      Serial.println(rightEncoderCount);
    }
  }
  while (digitalRead(pushButton) == 0); // wait for button release
  for (int i = 0; i < sizeof(moves)/2; i++) { // Loop through entire moves list
//    while (digitalRead(pushButton) == 1);
//    while (digitalRead(pushButton) == 0); // wait for button release
    if(moves[i]==LEFT){
      turn(true);
    }
    else if(moves[i]==RIGHT){
      turn(false);
    }
    else{
      drive(moves[i]);
    }
    run_motor(A, 0);
    run_motor(B, 0);
    delay(1000);
  }
  run_motor(A, 0);
  run_motor(B, 0);
  Serial.print("All done");
  return;
}
//////////////////////////////// end of loop() /////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void drive(float distance){
  // create variables needed for this function
  int countsDesired, cmdLeft, cmdRight, errorLeft, errorRight;

  // Find the number of encoder counts based on the distance given, and the 
  // configuration of your encoders and wheels
  countsDesired = int(distance / DistancePerRev * EncoderCountsPerRev);

  if(debug){
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print("  ");
    Serial.print("Encoder Count: ");
    Serial.print(countsDesired);
    Serial.print("  ");
  }

  // reset the current encoder counts
  leftEncoderCount = 0;
  rightEncoderCount = 0;
  
  // we make the errors greater than our tolerance so our first test gets us into the loop
  errorLeft = forwardTolerance + 1;
  errorRight =  forwardTolerance + 1;

  // Begin PID control until move is complete
  while (errorLeft > forwardTolerance || errorRight > forwardTolerance)
  {
    // according to the PID formula, what should the current PWMs be?
    cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft, false);
    cmdRight = computeCommand(GAIN_B, deadband_B, errorRight, false);

    // Set new PWMs
    run_motor(A, -cmdLeft);
    run_motor(B, -cmdRight);

    // Update encoder error
    errorLeft = countsDesired - leftEncoderCount;
    errorRight = countsDesired - rightEncoderCount;

    // If using bump sensors, check here for collisions
    // and call correction function

    //Some print statements, for debugging
//    Serial.print(errorLeft);
//    Serial.print(" ");
//    Serial.print(cmdLeft);
//    Serial.print("\t");
//    Serial.print(errorRight);
//    Serial.print(" ");
//    Serial.println(cmdRight);

  }

  //debug printouts for encoder ticks
  if(debug){
    Serial.print("Left #:");
    Serial.print(leftEncoderCount);
    Serial.print(" ");
    Serial.print("Right #:");
    Serial.print(rightEncoderCount);
    Serial.println(" ");
  }
}
////////////////////////////////////////////////////////////////////////////////

void driveOneBox(float multi){
  drive(30.48 * multi);
}

// Write a function for turning with PID control, similar to the drive function
void turn(bool is_left){
  // create variables needed for this function
  int countsDesired, cmdLeft, cmdRight, errorLeft, errorRight;

  // Find the number of encoder counts based on the distance given, and the 
  // configuration of your encoders and wheels
  float distance = 3.1415 * track;
  distance = distance * 0.25 * turnConst; //need only a quarter rotation for a left or right turn
  countsDesired = ceil(distance / DistancePerRev * EncoderCountsPerRev);

  if(debug){
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print("  ");
    Serial.print("Encoder Count: ");
    Serial.print(countsDesired);
    Serial.print(" ");
  }

  // reset the current encoder counts
  if(is_left){
    leftEncoderCount = 0;
    rightEncoderCount = 0;
  }
  else{
    leftEncoderCount = 0;
    rightEncoderCount = 0;
  }
  
  // we make the errors greater than our tolerance so our first test gets us into the loop
  errorLeft = turnTolerance + 1;
  errorRight =  turnTolerance + 1;

  // Begin PID control until move is complete
  while (errorLeft > turnTolerance || errorRight > turnTolerance)
  {
    // according to the PID formula, what should the current PWMs be?
    cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft, true);
    cmdRight = computeCommand(GAIN_B, deadband_B, errorRight, true);

    // Set new PWMs
    if(is_left){
      run_motor(A, cmdLeft);
      run_motor(B, -cmdRight);
    }
    else{
      run_motor(A, -cmdLeft);
      run_motor(B, cmdRight);
    }

    // Update encoder error
    errorLeft = countsDesired - leftEncoderCount;
    errorRight = countsDesired - rightEncoderCount;

    // If using bump sensors, check here for collisions
    // and call correction function

//    //Some print statements, for debugging
//    Serial.print(errorLeft);
//    Serial.print(" ");
//    Serial.print(cmdLeft);
//    Serial.print("\t");
//    Serial.print(errorRight);
//    Serial.print(" ");
//    Serial.println(cmdRight);
    //debug printouts for encoder ticks
  }
  if(debug){
    Serial.print("Left #:");
    Serial.print(leftEncoderCount);
    Serial.print(" ");
    Serial.print("Right #:");
    Serial.print(rightEncoderCount);
    Serial.println(" ");
  }
}


//////////////////////////////////////////////////////////


// If you want, write a function to correct your path due 
// to an active bump sensor. You will want to compensate somehow 
// for any wheel encoder counts that happend during this manuever


//////////////////////////////////////////////////////////
int computeCommand(int gain, int deadband, int error, boolean isTurn)
//  gain, deadband, and error, both are integer values
{
  if(isTurn){
    if (error <= turnTolerance) { // if error is acceptable, PWM = 0
      return (0);
    }
    int cmdDir = (gain * 1 * error); // Proportional control
    cmdDir = constrain(cmdDir,deadband,200); // Bind value between motor's min and max
    return(cmdDir);
  }
  else{
    if (error <= forwardTolerance) {
      return (0);
    }
    int cmdDir = (gain * error); // Proportional control
    cmdDir = constrain(cmdDir,deadband,200); // Bind value between motor's min and max
    return(cmdDir);
  }
}


//////////////////////////////////////////////////////////

// These are the encoder interupt funcitons, they should NOT be edited

void indexLeftEncoderCount()
{
  leftEncoderCount++;
}
//////////////////////////////////////////////////////////
void indexRightEncoderCount()
{
  rightEncoderCount++;
}
