#define FORWARD             0
#define LEFT                1
#define RIGHT              -1
#define NONE               999

int theoreticalRun[23] = {FORWARD, RIGHT, FORWARD, FORWARD, RIGHT, FORWARD, RIGHT, FORWARD, LEFT, LEFT, FORWARD, FORWARD, FORWARD, FORWARD, LEFT, LEFT, FORWARD, RIGHT, FORWARD, FORWARD, FORWARD, RIGHT, FORWARD};
int optimalMoves[5] = {FORWARD, LEFT, FORWARD, RIGHT, FORWARD};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Begin solving!");
  solve(theoreticalRun, 23);
  Serial.println("Finished Solving");
  delay(5000);
}

void solve(int moves[], int arrSize) {
  printMovesArray(moves, arrSize);
  // Write your own algorithm to solve the maze using the list of moves from explore
  int newMoves[arrSize]; //set to size of old array since it cannot be larger
  for(int i = 0; i < arrSize; i++){ //init to NONE
    newMoves[i] = NONE;
  }
  int newMovesLength = 0; //counter for new moves added
  int lastMove = NONE; //set to a value it cannot be, saves last move
  for(int i = 0; i < arrSize; i++){
    if(moves[i] == NONE)
      break;
    if(lastMove == moves[i] && (moves[i] == RIGHT || moves[i] == LEFT)){ //finds two turns in a row
      Serial.println("EVALUATING DEAD END");
      Serial.print("Found a 180 turn at index: ");
      Serial.println(i);
      //MATCHES FORWARDS ON EITHER SIDE
      int forwardAfterCount = 0;
      int forwardBeforeCount = 0;
      int a = i+1;
      int b = i-2;
      //iterates forward through the array until it finds a move that isnt forward
      Serial.println("Iterating forward through the array");
      while(a < arrSize && moves[a] == FORWARD){
        String args[] = {"moves[", String(a), "] = ", moveMap(moves[a])};
        smartPrint(args, 4);
        forwardAfterCount++;
        a++;
      }
      //iterates backwards through the array until it finds a move that isnt forward
      Serial.println("Iterating backwards through the array");
      
      while(b > 0 && moves[b] == FORWARD){
        String args[] = {"moves[", String(b), "] = ", moveMap(moves[b])};
        smartPrint(args, 4);
        forwardBeforeCount++;
        b--;
      }
      //the min is the amount of forwards on either side
      int matchingForwards = min(forwardAfterCount, forwardBeforeCount);
      Serial.print("  F befores: ");
      Serial.print(forwardBeforeCount);
      Serial.print("  F afters: ");
      Serial.println(forwardAfterCount);
      Serial.print("Matching forwards: ");
      Serial.println(matchingForwards);

      //need to go back one plus the matching Forwards and start replacing there again
      newMovesLength = newMovesLength - 1 - matchingForwards;
      if(moves[i] == RIGHT){
        newMoves[newMovesLength] = LEFT;
      }
      else{
        newMoves[newMovesLength] = RIGHT;
      }
      newMovesLength++;
      i += matchingForwards;
      Serial.println("FINISHED EVALUATING DEAD END");
      Serial.println("");
    }
    lastMove = moves[i];
    newMoves[newMovesLength] = moves[i];
    newMovesLength++;
  }
  printMovesArray(newMoves, arrSize);
  //PRINT THE MOVES
  //PRINT THE NEW MOVES
  //COPY THE NEW MOVES TO THE MOVES ARRAY AND FILL IN THE REST OF MOVES WITH NULL
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
