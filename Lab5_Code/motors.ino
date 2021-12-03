//A IS LEFT
//B IS RIGHT
void motor_setup() {
    pinMode  (pwmA, OUTPUT ); 
    pinMode  (dirA, OUTPUT );
    pinMode  (pwmB, OUTPUT );
    pinMode  (dirB, OUTPUT );
    // initialize all pins to zero
    digitalWrite  (pwmA, 0);
    digitalWrite  (dirA, 0);
    digitalWrite  (pwmB, 0);
    digitalWrite  (dirB, 0);
    return;
    //end function
}

// int motor is the defined A or B
// pwm = the power cycle you want to use
void run_motor(int motor, int pwm) {
  int dir = (pwm/abs(pwm))> 0; // returns if direction is forward (1) or reverse (0)
  pwm = abs(pwm); // only positive values can be sent to the motor

  switch (motor) { // find which motor to control
    case A: // if A, write A pins
      digitalWrite  (dirA, dir); // dir is either 1 (forward) or 0 (reverse)
      analogWrite  (pwmA, pwm); // pwm is an analog value 0-255
      break; // end case A
    case B: // if B, write B pins
      digitalWrite  (dirB, dir); // dir is either 1 (forward) or 0 (reverse)
      analogWrite  (pwmB, pwm); // pwm is an analog value 0-255
      break; // end case A
  } // end switch statement
  return;
} // end function

void stop_motors(){
  run_motor(A, 0);
  run_motor(B, 0);
}
