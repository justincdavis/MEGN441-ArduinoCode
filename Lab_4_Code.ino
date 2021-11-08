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

int moves[MOVESIZE];
int optimalMoves[5] = {FORWARD, LEFT, FORWARD, RIGHT, FORWARD};

// Define IR Distance sesnsor Pins
#define frontIR A0
#define sideIR  A1

// Define the distance tolerance that indicates a wall is present
#define wallToSide 16.0 //cm
#define minDistToWall 7.0
#define boxConstant 0.85

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
  // this next line setup the PinChange Interrupt
  PCintPort::attachInterrupt(EncoderMotorLeft, indexLeftEncoderCount, CHANGE);
  // if you "really" want to know what's going on read the PinChange.h file :)
  /////////////////////////////////////////////////
  Serial.print("Now setting up the Right Encoder: Pin ");
  Serial.print(EncoderMotorRight);
  Serial.println();
  pinMode(EncoderMotorRight, INPUT_PULLUP);     //set the pin to input
  PCintPort::attachInterrupt(EncoderMotorRight, indexRightEncoderCount, CHANGE);

  Serial.println("Finished Setup");
} /////////////// end of setup ////////////////////////////////////

/////////////////////// loop() ////////////////////////////////////
void loop()
{
  Serial.println("Executing Loop");
  while (digitalRead(pushButton) == 1); // wait for button push
  while (digitalRead(pushButton) == 0); // wait for button release
  Serial.println("Exploring the maze!");
  explore();
  Serial.println("Finished exploring!");
  stop_motors();
  Serial.println("Began solving the maze!");
  solve();
  Serial.println("Finished solving the maze!");
  while (1) { //Inifnite number of runs, so you don't have to re-explore everytime a mistake happens
    while (digitalRead(pushButton) == 1); // wait for button push
    while (digitalRead(pushButton) == 0); // wait for button release
    runMaze();
    stop_motors();
  }
}

////////////////////////////////////////////////////////////////////////////////
float readFrontDist() { 
  // If IR distance sensor
  int reading = analogRead(frontIR);
  float dist = 1 / ((float(reading) - 1.7559) / 5684.3);
  return dist;
}
////////////////////////////////////////////////////////////////////////////////
float readSideDist() {
  // If IR distance sensor
  int reading = analogRead(sideIR);
  float dist = 1 / ((float(reading) - 40.293) / 4453.2);
  return dist;
}
//////////////////////////////// end of loop() /////////////////////////////////
void explore() {
  delay(1000); //have a delay so button is not instantly pressed
  if(irdebug){
    while(digitalRead(pushButton) == 1){
      Serial.print("Front IR: ");
      Serial.println(readFrontDist());
      Serial.print("Side IR: ");
      Serial.println(readSideDist());
      delay(500);
    }
    delay(1000); //have a delay so button is not instantly pressed
  }
  int moveCounter = 0;
  while (digitalRead(pushButton) == 1) { //while maze is not solved
    // Read distances
    float front = readFrontDist();
    float side = readSideDist();
    if(front < minDistToWall){
        Serial.println("Attempting to move away from wall!");
        driveBack(0.25);
    }
    if (side > wallToSide) {// If side is not a wall
      // turn and drive forward
      // Record actions
      turnRight();
      moves[moveCounter++] = RIGHT;
      driveOneBox();
      moves[moveCounter++] = FORWARD;
    }
    else if (front > wallToSide) {// else if front is not a wall
      // drive forward
      // Record action
      driveOneBox();
      moves[moveCounter++] = FORWARD;
    } else {
      // turn away from side
      // Record action
      turnLeft();
      moves[moveCounter++] = LEFT;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
// exploreMoves = [F, R, F, F, R, F, R, F, L, L, F, F, F, F, L, L, F, R, F, F, F, R, F];
////////////////////////////////////////////////////////////////////////////////
void solve() {
  // Write your own algorithm to solve the maze using the list of moves from explore
  int newMoves[MOVESIZE];
  for(int i = 0; i < MOVESIZE; i++){
    newMoves[i] = NONE;
  }
  int newMovesLength = 0;
  int lastMove = NONE; //set to a value it cannot be
  for(int i = 0; i < sizeof(moves)/2; i++){
    if(lastMove == moves[i] && (moves[i] == RIGHT || moves[i] == LEFT)){
      //performed a 180 degree turn DEADEND
      //need to remove both turns and count forwards on each side, then replace with one of the opposite turn
      int forwardAfterCount = 0;
      int forwardBeforeCount = 0;
      int a = i;
      int b = i;
      while(a < sizeof(moves)/2 && moves[a] == moves[a+1]){
        forwardAfterCount++;
        a++;
      }
      while(b > 0 && moves[b] == moves[b-1]){
        forwardBeforeCount++;
        b--;
      }
      int matchingForwards = min(forwardAfterCount, forwardBeforeCount);
      //need to go back one plus the matching Forwards and start replacing there again
      newMovesLength = newMovesLength - 1 - matchingForwards;
      if(moves[i] == RIGHT){
        newMoves[newMovesLength] = LEFT;
      }
      else{
        newMoves[newMovesLength] = RIGHT;
      }
      newMovesLength++;
    }
    lastMove = moves[i];
    newMoves[newMovesLength] = moves[i];
  }
}
////////////////////////////////////////////////////////////////////////////////
void runMaze() {
  for(int i = 0; i < sizeof(moves)/2; i++){
    // copy for loop from Lab 3 to run through finished maze path
    if(moves[i]==LEFT){
      turnLeft();
    }
    else if(moves[i]==RIGHT){
      turnRight();
    }
    else{
      driveOneBox();
    }
    stop_motors();
    delay(1000);
  }
  stop_motors();
  Serial.print("All done");
  return;
}
////////////////////////////////////////////////////////////////////////////////
// Copy your drive function from Lab 3
////////////////////////////////////////////////////////////////////////////////
void drive(float distance){
  // create variables needed for this function
  int countsDesired, cmdLeft, cmdRight, errorLeft, errorRight;

  // Find the number of encoder counts based on the distance given, and the 
  // configuration of your encoders and wheels
  countsDesired = int(distance / DistancePerRev * EncoderCountsPerRev);

  if(distance < 0){
    countsDesired *= -1;
  }

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
  while (errorLeft > forwardTolerance || errorRight > forwardTolerance){
    // according to the PID formula, what should the current PWMs be?
    cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft, false);
    cmdRight = computeCommand(GAIN_B, deadband_B, errorRight, false);

    if(distance < 0){
      cmdLeft *= -1;
      cmdRight *= -1;
    }

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

//  leftSteadyError += (countsDesired - leftEncoderCount);
//  rightSteadyError += (countsDesired - rightEncoderCount);

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

void driveOneBox(){
  drive(30.48 * boxConstant);
  stop_motors();
  delay(750);
}

void driveBack(float mult){
  drive(30.48 * -1 * boxConstant * mult);
  stop_motors();
  delay(750);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Copy your turn function from Lab 3
// Write a function for turning with PID control, similar to the drive function
void turn(bool is_left){
  // create variables needed for this function
  int countsDesired, cmdLeft, cmdRight, errorLeft, errorRight;

  // Find the number of encoder counts based on the distance given, and the 
  // configuration of your encoders and wheels
  float distance = 3.1415 * track;
  distance = distance * 0.25 * turnGainConst; //need only a quarter rotation for a left or right turn
  countsDesired = ceil(distance / DistancePerRev * EncoderCountsPerRev) + turnConst;

  if(debug){
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print("  ");
    Serial.print("Encoder Count: ");
    Serial.print(countsDesired);
    Serial.print(" ");
  }

  // reset the current encoder counts
  leftEncoderCount = 0;
  rightEncoderCount = 0;
  
  // we make the errors greater than our tolerance so our first test gets us into the loop
  errorLeft = turnTolerance + 1;
  errorRight =  turnTolerance + 1;

  // Begin PID control until move is complete
  while (errorLeft > turnTolerance || errorRight > turnTolerance)
  {
    // according to the PID formula, what should the current PWMs be?
    if(!steadyErrorOn){
      leftSteadyError = 0;
      rightSteadyError = 0; 
    }
    
    cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft + (-1 * leftSteadyError), true);
    cmdRight = computeCommand(GAIN_B, deadband_B, errorRight + (-1 * rightSteadyError), true);

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

  int leftError = leftEncoderCount - countsDesired;
  int rightError = rightEncoderCount - countsDesired;
  leftSteadyError += leftError;
  rightSteadyError += rightError;
  
  if(debug){
    Serial.print("Left #:");
    Serial.print(leftEncoderCount);
    Serial.print(" ");
    Serial.print("Right #:");
    Serial.print(rightEncoderCount);
    Serial.println(" ");

    Serial.print("Accumulated steady errors, left: ");
    Serial.print(leftSteadyError);
    Serial.print("  right: ");
    Serial.print(rightSteadyError);
    Serial.println(" ");
  }
}

//////////////////////////////////////////////////////////
// Copy your computeCommand function from Lab 3
//////////////////////////////////////////////////////////
int computeCommand(int gain, int deadband, int error, boolean isTurn)
//  gain, deadband, and error, both are integer values
{
  if(isTurn){
    if (error <= turnTolerance) { // if error is acceptable, PWM = 0
      return (0);
    }
    int cmdDir = (gain * turnGainConst * error); // Proportional control
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

void turnLeft(){
  turn(true);
  stop_motors();
  delay(750);
}
void turnRight(){
  turn(false);
  stop_motors();
  delay(750);
}
//////////////////////////////////////////////////////////
// Copy your reactive functions from Lab 3
// NEED TO IMPLEMENT REACTIVE CONTROK
// WANT TO IMPLEMENT BACKUP TO STRAIGHEN ROBOT FUNCTION
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
