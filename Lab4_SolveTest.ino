#define FORWARD             0
#define LEFT                1
#define RIGHT              -1
#define NONE               999

int theoreticalRun[23] = {FORWARD, RIGHT, FORWARD, FORWARD, RIGHT, FORWARD, RIGHT, FORWARD, LEFT, LEFT, FORWARD, FORWARD, FORWARD, FORWARD, LEFT, LEFT, FORWARD, RIGHT, FORWARD, FORWARD, FORWARD, RIGHT, FORWARD};
int optimalMoves[5] = {FORWARD, LEFT, FORWARD, RIGHT, FORWARD};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Started Solving");
  solve(theoreticalRun, 23);
  Serial.println("Finished Solving");
}

void solve(int moves[], int arrSize) {
  int maxMoves = 30;
  // Write your own algorithm to solve the maze using the list of moves from explore
  int newMoves[maxMoves];
  for(int i = 0; i < maxMoves; i++){
    newMoves[i] = NONE;
  }
  int newMovesLength = 0;
  int lastMove = NONE; //set to a value it cannot be
  for(int i = 0; i < arrSize; i++){
    if(lastMove == moves[i] && (moves[i] == RIGHT || moves[i] == LEFT)){
      //performed a 180 degree turn DEADEND
      //need to remove both turns and count forwards on each side, then replace with one of the opposite turn
      int forwardAfterCount = 0;
      int forwardBeforeCount = 0;
      int a = i;
      int b = i;
      while(a < arrSize && moves[a] == moves[a+1]){
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
  //PRINT THE MOVES
  //PRINT THE NEW MOVES
  //COPY THE NEW MOVES TO THE MOVES ARRAY AND FILL IN THE REST OF MOVES WITH NULL
}

void printIntArray(int arr[], int arrSize){
  Serial.print("[");
  for(int i = 0; i < arrSize; i++){
    Serial.print(arr[i]);
    if(i < arrSize - 1){
      Serial.print(", ");
    }
  }
  Serial.println("]");
}
