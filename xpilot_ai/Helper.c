//Evan Gray - May 2012
//Compile: gcc Helper.c libcAI.so -o Helper
//Run: ./Helper
#include "cAI.h"
#include <stdio.h>
int frames=0;
AI_loop() {
  frames++;
  if (frames==20) {
    thrust(1);
    frames=0;
  }
  else thrust(0);
  turnLeft(1);
  fireShot();
}
int main(int argc, char *argv[]) {
  return start(argc, argv);
}
