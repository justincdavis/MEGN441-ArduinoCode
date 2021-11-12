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

int runMoves[MOVESIZE];
int moveSize = 0;

// Define IR Distance sesnsor Pins
#define frontIR A0
#define sideIR  A1

// Define the distance tolerance that indicates a wall is present
#define wallToSide 16.0 //cm
#define minDistToWall 7.0
#define boxConstant 0.85

//CUSTOM UTILITY FUNCTIONS
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

void setup() {
  // set stuff up
  Serial.begin(9600);
  motor_setup();
  pinMode(pushButton, INPUT_PULLUP);
  // add additional pinMode statements for any bump sensors

  //fill runMoves with NONE
  for(int i = 0; i < MOVESIZE; i++){
    runMoves[i] = NONE;
  }

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
  delay(1000);
  Serial.println("Finished exploring!");
  stop_motors();
  delay(1000);
  Serial.println("Began solving the maze!");
  solve(runMoves, MOVESIZE, RIGHT, false);
  Serial.println("Finished solving the maze!");
  delay(1000);
  while (1) { //Inifnite number of runs, so you don't have to re-explore everytime a mistake happens
    Serial.println("Beginning infinite loop for run maze");
    while (digitalRead(pushButton) == 1); // wait for button push
    while (digitalRead(pushButton) == 0); // wait for button release
    Serial.println("Beginning to run the maze");
    runMaze();
    stop_motors();
    Serial.println("Finished running the maze");
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
      runMoves[moveCounter++] = RIGHT;
      driveOneBox();
      runMoves[moveCounter++] = FORWARD;
    }
    else if (front > wallToSide) {// else if front is not a wall
      // drive forward
      // Record action
      driveOneBox();
      runMoves[moveCounter++] = FORWARD;
    } else {
      // turn away from side
      // Record action
      turnLeft();
      runMoves[moveCounter++] = LEFT;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
// exploreMoves = [F, R, F, F, R, F, R, F, L, L, F, F, F, F, L, L, F, R, F, F, F, R, F];
////////////////////////////////////////////////////////////////////////////////
struct indexLength {
    int index;
    int length;
    int replacement;
    int pattern;
};

void printIndexLength(struct indexLength m){
  Serial.print("Index: ");
  Serial.print(m.index);
  Serial.print("  Length: ");
  Serial.print(m.length);
  Serial.print("  Replacement: ");
  Serial.print(moveMap(m.replacement));
  Serial.print("  Pattern: ");
  Serial.println(m.pattern);
}

struct indexLength identifyTurnaround(int moves[], int arrSize, int hand, boolean debug){
  int nothand = NONE;
  if(hand == 1)
    nothand = -1;
  else  
    nothand = 1;
  struct indexLength r;
  r.index = 0;
  r.length = 0;
  r.replacement = NONE;
  r.pattern = 0;
  int lastMove = NONE; //set to a value it cannot be, saves last move
  for(int i = 0; i < arrSize; i++){
    if(moves[i] == NONE)
      break;
    if(lastMove == moves[i] && moves[i] == nothand){ //finds two turns in a row
      if(debug){
        Serial.println("");
        Serial.println("EVALUATING DEAD END");
        Serial.print("Found a 180 turn at index: ");
        Serial.println(i);
      }
      //MATCHES FORWARDS ON EITHER SIDE
      int forwardAfterCount = 0;
      int forwardBeforeCount = 0;
      int a = i+1;
      int b = i-2;
      //iterates forward through the array until it finds a move that isnt forward
      if(debug)
        Serial.println("Iterating forward through the array");
      while(a < arrSize && moves[a] == FORWARD){
        if(debug){
          String args[] = {"moves[", String(a), "] = ", moveMap(moves[a])};
          smartPrint(args, 4);
        }
        forwardAfterCount++;
        a++;
      }
      //iterates backwards through the array until it finds a move that isnt forward
      if(debug)
        Serial.println("Iterating backwards through the array");
      
      while(b > 0 && moves[b] == FORWARD){
        if(debug){
          String args[] = {"moves[", String(b), "] = ", moveMap(moves[b])};
          smartPrint(args, 4);
        }
        forwardBeforeCount++;
        b--;
      }
      //the min is the amount of forwards on either side
      int matchingForwards = min(forwardAfterCount, forwardBeforeCount);
      if(debug){
        Serial.print("  F befores: ");
        Serial.print(forwardBeforeCount);
        Serial.print("  F afters: ");
        Serial.println(forwardAfterCount);
        Serial.print("Matching forwards: ");
        Serial.println(matchingForwards);
      }

      int starting = i - 1 - matchingForwards;
      int ending = i + matchingForwards;

      if(debug){
        String args[] = {"Starting: ", String(starting), "  Ending: ", String(ending)};
        smartPrint(args, 4);
      }


      //look for pattern 1, handed before start and handed plus forward after ending
      Serial.println("Checking Pattern 1");
      if(starting > 0){
        if(moves[starting-1] == hand){
          if(ending < arrSize - 2){
            if(moves[ending+1] == hand && moves[ending+2] == FORWARD){
              Serial.println("  FOUND PATTERN");
              r.index = starting-1;
              r.length = ending - starting + 3;
              r.replacement = FORWARD;
              r.pattern = 1;
              return r;
            }
          }
        }
      }
      //look for pattern 2, which is a hand before the start
      Serial.println("Checking Pattern 2");
      if(starting > 0){
        if(moves[starting-1] == hand){
            Serial.println("  FOUND PATTERN");
            r.index = starting - 1;
            r.length = ending - starting + 1;
            r.replacement = nothand;
            r.pattern = 2;
            return r;
        }
      }
      //look for pattern 3, which is a hand after the end
      Serial.println("Checking Pattern 3");
      if(ending < arrSize - 2){
        if(moves[ending+1] == hand){
          Serial.println("  FOUND PATTERN");
          r.index = starting;
          r.length = ending - starting + 1;
          r.replacement = nothand;
          r.pattern = 3;
          return r;
        }
      }
      //if it is not any of the above 3, then it is the fourth pattern
      Serial.println("Checking Pattern 4");
      Serial.println("  FOUND PATTERN");
      r.index = starting;
      r.length = ending-starting;
      r.replacement = hand;
      r.pattern = 4;
      return r;
    }
    lastMove = moves[i];
  }
  return r;
}

boolean rsolve(int moves[], int arrSize, int hand, boolean debug) {
  Serial.println("Input array to solve: ");
  printMovesArray(moves, arrSize);

  struct indexLength r = identifyTurnaround(moves, arrSize, hand, debug);
  boolean reduced = false;

  if(r.pattern == 1 || r.pattern == 2 || r.pattern == 3){ //pattern 1,2,3 case
    reduced = true;
    moves[r.index] = r.replacement;
    //copy from the ending + 1 to the end of the array
    //then fill from where the 'new index was to the end of the array with NONE
    int newIndex = r.index+1;
    for(int i = r.index+r.length+1; i < arrSize; i++){
      moves[newIndex] = moves[i];
      newIndex++;
    }
    for(int i = newIndex; i < arrSize; i++){
      moves[i] = NONE;
    }
  }
  else if(r.pattern == 4){  
    reduced = true;
    moves[r.index] = r.replacement;
    moves[r.index+1] = r.replacement;
    //copy from the ending + 1 to the end of the array
    //then fill from where the 'new index was to the end of the array with NONE
    int newIndex = r.index+2;
    for(int i = r.index+r.length+1; i < arrSize; i++){
      moves[newIndex] = moves[i];
      newIndex++;
    }
    for(int i = newIndex; i < arrSize; i++){
      moves[i] = NONE;
    }
  }
  else{
    if(debug){
      Serial.println("No more simplifications can be made!");
    }
  }
  //copy into runMoves array
  if(debug){
    Serial.println("Reduced input to :");
    printMovesArray(moves, arrSize);
    Serial.println("Using strucural info: ");
    printIndexLength(r);
    Serial.println(" ");
  }

  for(int i = 0; i < arrSize; i++){
    runMoves[i] = moves[i];
  }
  return reduced;
}

String moveMap(int m){
  if(m == FORWARD)
      return "F";
  else if(m == LEFT)
    return "L";
  else if(m == RIGHT)
    return "R";
  else
    return "N"; 
}

void solve(int moves[], int arrSize, int hand, boolean debug){
  while(rsolve(moves, arrSize, hand, debug));
}
////////////////////////////////////////////////////////////////////////////////
void runMaze() {
  if(runOptimal){
    for(int i = 0; i < sizeof(optimalMoves)/2; i++){
      // copy for loop from Lab 3 to run through finished maze path
      if(optimalMoves[i]==LEFT){
        turnLeft();
      }
      else if(optimalMoves[i]==RIGHT){
        turnRight();
      }
      else{
        driveOneBox();
      }
      stop_motors();
    }
  }
  else{
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
    }
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
