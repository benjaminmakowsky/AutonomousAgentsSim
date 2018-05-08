//Justin Anderson - May 2012
//Compile: gcc TesterBot.c libcAI.so -o TesterBot
//Run: ./TesterBot
#include "cAI.h"
#include <stdio.h>
#define PI 3.1415926536
AI_loop() {
    printf("selfX: %d \n",selfX());
    printf("selfY: %d \n",selfY());
    printf("selfHeadingDeg: %f \n",selfHeadingDeg());
    //printf("wallFeeler: %d \n",wallFeeler(100,0,1,1));
    //printf("wallFeelerRad: %d \n",wallFeelerRad(100,0.0,1,1));
    //printf("wallBetween: %d \n",wallBetween(87,600,87,1000,1,1));
  }
int main(int argc, char *argv[]) {
  return start(argc, argv);
}
