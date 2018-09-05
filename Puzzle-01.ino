/*
   Puzzle-01

   by Dan & Vanilla

   TODO:
   1) count number of contiguous Blinks
   2) number them off uniquely
   3) distribute puzzle arrangement
      a) initially save some possible puzzles to distribute
      b) proceedurally generate possible puzzles

*/

enum gameModes {CREATE, SET, GAME};
Color connectionColors[4] = {OFF, RED, YELLOW, BLUE};

byte currentMode;
byte faceInfo[6];//stores the color of each individual face by
byte happinessInfo[6];//stores happiness of faces as brightness value

byte unhappyBrightness = 64;

#define BLINK_DURATION 500
Timer blinkTimer;
bool blinkOn = false;

void setup() {
  // put your setup code here, to run once:
  currentMode = CREATE;

}

void loop() {
  switch (currentMode) {
    case CREATE:
      createLoop();
      break;
    case SET:
      setLoop();
      break;
    case GAME:
      gameLoop();
      break;
  }

  switch (currentMode) {
    case CREATE:
      displayCreate();
      break;
    case SET:
      displaySet();
      break;
    case GAME:
      displayGame();
      break;
  }

  //send out communications, serialized but not bitwise
  FOREACH_FACE(f) {
    setValueSentOnFace(currentMode * 10 + faceInfo[f], f);
  }
}

void createLoop() {
  //detect connections
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//I have a neighbor and...
      if (faceInfo[f] == 0) {//I didn't have a neighbor before
        faceInfo[f] = rand(2) + 1;//random 1-4
        happinessInfo[f] = 255;
      }
    } else {//no neighbor
      faceInfo[f] = 0;
    }
  }

  //so we set face colors, now we evaluate for color mismatches
  FOREACH_FACE(f) {

    if (!isValueReceivedOnFaceExpired(f)) { //something here
      byte neighborColor = getLastValueReceivedOnFace(f) % 10;
      if (neighborColor != 0) { //this neighbor has a color
        if (neighborColor != faceInfo[f]) {//we have a mismatch
          switch (faceInfo[f]) {
            case 1:
              if (neighborColor == 2) {
                faceInfo[f] = 2;
              }
              break;
            case 2:
              if (neighborColor == 3) {
                faceInfo[f] = 3;
              }
              break;
            case 3:
              if (neighborColor == 1) {
                faceInfo[f] = 1;
              }
              break;
          }
        }
      }
    }
  }

  //look for double click to transition
  if (buttonDoubleClicked()) {
    currentMode = SET;
  }

  //look for neighbors in SET to move to transition
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //we have a neighbor
      if (getLastValueReceivedOnFace(f) / 10 == SET) {//this neighbor is in SET
        currentMode = SET;
      }
    }
  }
}

void setLoop() {
  //look at neighbors. If all are in GAME or SET, move to GAME
  byte neighborsInCreate = 0;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //someone there
      if (getLastValueReceivedOnFace(f) / 10 == CREATE) {
        neighborsInCreate++;
      }
    }
  }//end face loop

  //after the loop, check if neighborsInCreate is 0, then transition
  if (neighborsInCreate == 0) {
    currentMode = GAME;
  }
}

void gameLoop() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor
      if (getLastValueReceivedOnFace(f) % 10 == faceInfo[f]) {//our colors match
        happinessInfo[f] = 255;
      } else {//colors don't match
        happinessInfo[f] = unhappyBrightness;
      }
    } else {//no neighbor
      happinessInfo[f] = unhappyBrightness;
    }
  }

  //look for double click to transition
  if (buttonDoubleClicked() && !isAlone()) {
    currentMode = CREATE;
  }

  //look for neighbors in CREATE to move to transition
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //we have a neighbor
      if (getLastValueReceivedOnFace(f) / 10 == CREATE) {//this neighbor is in CREATE
        currentMode = CREATE;
      }
    }
  }
}

void displayCreate() {
  if (blinkTimer.isExpired()) {
    blinkTimer.set(BLINK_DURATION);
    blinkOn = !blinkOn;
  }
  if (blinkOn) {
    FOREACH_FACE(f) {
      Color faceColor = dim(connectionColors[faceInfo[f]], happinessInfo[f]);
      setColorOnFace(faceColor, f);
    }
  }
  else {
    FOREACH_FACE(f) {
      Color faceColor = dim(connectionColors[faceInfo[f]], happinessInfo[f]);
      setColorOnFace(dim(faceColor, 127), f);
    }
  }
}

void displaySet() {
  setColor(WHITE);
}

void displayGame() {
  FOREACH_FACE(f) {
    Color faceColor = dim(connectionColors[faceInfo[f]], happinessInfo[f]);
    setColorOnFace(faceColor, f);
  }
}

