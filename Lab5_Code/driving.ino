////////////////////////////////////////////////////////////////////////////////
// DRIVING FUNCTIONS TREAT AS BUILT IN 
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
    if(debug){
      String args[] = {"Left encoder count: ", String(leftEncoderCount), "  Right encoder count: ", String(rightEncoderCount)};
      smartPrint(args, 4);

      int distVal = readFrontDist();
      Serial.print("Front distance value: ");
      Serial.println(distVal);
    }

    if(readFrontDist() <= 30){
      stop_motors();
      while(readFrontDist() <= 30);
    }
    
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

    if(debug){
      Serial.print(errorLeft);
      Serial.print(" ");
      Serial.print(cmdLeft);
      Serial.print("\t");
      Serial.print(errorRight);
      Serial.print(" ");
      Serial.println(cmdRight);
    }
  }

  int leftError = leftEncoderCount - countsDesired;
  int rightError = rightEncoderCount - countsDesired;
  leftSteadyError += leftError;
  rightSteadyError += rightError;

  //debug printouts for encoder ticks
  if(debug){
    Serial.print("Left #:");
    Serial.print(leftEncoderCount);
    Serial.print(" ");
    Serial.print("Right #:");
    Serial.print(rightEncoderCount);
    Serial.println(" ");
  }

  stop_motors();
}
void driveOneBox(){
  drive(30.48 * boxConstant);
  stop_motors();
  delay(1000);
}
void driveBack(float mult){
  drive(30.48 * -1 * boxConstant * mult);
  stop_motors();
  delay(1000);
}
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
  while (errorLeft > turnTolerance || errorRight > turnTolerance){

    if(debug){
      String args[] = {"Left encoder count: ", String(leftEncoderCount), "  Right encoder count: ", String(rightEncoderCount)};
      smartPrint(args, 4);
    }
    
    // according to the PID formula, what should the current PWMs be?
    if(!steadyErrorOn){
      leftSteadyError = 0;
      rightSteadyError = 0; 
    }
    
    // cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft + (-1 * leftSteadyError), true);
    // cmdRight = computeCommand(GAIN_B, deadband_B, errorRight + (-1 * rightSteadyError), true);
    cmdLeft = computeCommand(GAIN_A, deadband_A, errorLeft, true);
    cmdRight = computeCommand(GAIN_B, deadband_B, errorRight, true);
    cmdLeft += 15;

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

    if(debug){
      Serial.print(errorLeft);
      Serial.print(" ");
      Serial.print(cmdLeft);
      Serial.print("\t");
      Serial.print(errorRight);
      Serial.print(" ");
      Serial.println(cmdRight);
    }
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

  stop_motors();
}

// compute command 
int computeCommand(int gain, int deadband, int error, boolean isTurn)
//  gain, deadband, and error, both are integer values
{
  if(isTurn){
    if (error <= turnTolerance) { // if error is acceptable, PWM = 0
      return (0);
    }
    int cmdDir = (gain * turnGainConst * error); // Proportional control
    cmdDir = constrain(cmdDir,deadband,200); // Bind value between motor's min and max
    cmdDir += 30;
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
// Turn functions
void turnLeft(){
  turn(true);
  stop_motors();
  delay(1000);
}
void turnRight(){
  turn(false);
  stop_motors();
  delay(1000);
}
void turnAround(){
  turnLeft();
  turnLeft();
}
// These are the encoder interupt funcitons, they should NOT be edited
void indexLeftEncoderCount()
{
  leftEncoderCount++;
}
void indexRightEncoderCount()
{
  rightEncoderCount++;
}
