#define FORWARD             0
#define LEFT                1
#define RIGHT              -1
#define NONE               999

int theoreticalRun[23] = {FORWARD, RIGHT, FORWARD, FORWARD, RIGHT, FORWARD, RIGHT, FORWARD, LEFT, LEFT, FORWARD, FORWARD, FORWARD, FORWARD, LEFT, LEFT, FORWARD, RIGHT, FORWARD, FORWARD, FORWARD, RIGHT, FORWARD};
int optimalMoves[5] = {FORWARD, LEFT, FORWARD, RIGHT, FORWARD};
int newMoves[23] = {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE};

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
    if(lastMove == moves[i] && (moves[i] == RIGHT || moves[i] == LEFT)){ //finds two turns in a row
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

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Begin solving!");


  solve(theoreticalRun, 23, RIGHT, true);
  printMovesArray(newMoves, 23);

  Serial.println("Finished Solving");
  Serial.println("");
  //printMovesArray(newMoves, 23);
  delay(5000);
}

void solve(int moves[], int arrSize, int hand, boolean debug){
  while(rsolve(moves, arrSize, hand, debug));
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
  //copy into newMoves array
  if(debug){
    Serial.println("Reduced input to :");
    printMovesArray(moves, arrSize);
    Serial.println("Using strucural info: ");
    printIndexLength(r);
    Serial.println(" ");
  }

  for(int i = 0; i < arrSize; i++){
    newMoves[i] = moves[i];
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

