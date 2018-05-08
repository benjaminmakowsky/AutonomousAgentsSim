#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SCHEMESIZE 2
#define POPSIZE 10
#define MAXGEN 20
int diff[2] = {-180,180};
int* scheme[SCHEMESIZE];
int pop[POPSIZE][SCHEMESIZE];
double fit[POPSIZE];
int maxFitIndex() {
  int i;
  int maxi=0;
  for (i=1;i<POPSIZE;i++) if (fit[i]>fit[maxi]) maxi=i;
  return maxi;
}
double getFit(int* c) {
  int i;
  FILE *file;
  char line[50];
  double f;
  file = fopen("chr.txt","w");
  for (i=0;i<SCHEMESIZE;i++) fprintf(file,"%d\n",c[i]);
  fclose(file);
  printf("START BOT:");
  for (i=0;i<2;i++) printf("%d,",c[i]);
  printf("\n");
  system("./cFrameXP");
  file = fopen("fit.txt","r");
  fgets(line,50,file);
  sscanf(line,"%lf",&f);
  fclose(file);
  printf("BOT FIT:%f\n",f);
  return f;
}
void main() {
  int i;
  int j;
  srand(time(NULL));
  scheme[0]=diff;
  scheme[1]=diff;
  for (i=0;i<POPSIZE;i++) {
    for (j=0;j<SCHEMESIZE;j++) pop[i][j]=((rand()%(scheme[j][1]-scheme[j][0]))+scheme[j][0]);
    fit[i]=-1.0;
  }
  int gen;
  for (gen=0;gen<MAXGEN;gen++) {
    for (i=0;i<POPSIZE;i++) fit[i] = getFit(pop[i]);
    int p1[SCHEMESIZE];
    i = maxFitIndex();
    for (j=0;j<SCHEMESIZE;j++) p1[j]=pop[i][j];
    fit[i]=-1.0;
    int p2[SCHEMESIZE];
    i = maxFitIndex();
    for (j=0;j<SCHEMESIZE;j++) p2[j]=pop[i][j];
    for (i=0;i<POPSIZE;i++) {
      int sp = rand()%SCHEMESIZE;
      if (rand()%2==1) {
        for (j=0;j<sp;j++) pop[i][j]=p1[j];
        for (j=sp;j<SCHEMESIZE;j++) pop[i][j]=p2[j];
      } else {
        for (j=0;j<sp;j++) pop[i][j]=p2[j];
        for (j=sp;j<SCHEMESIZE;j++) pop[i][j]=p1[j];
      }
      for (j=0;j<SCHEMESIZE;j++) if (rand()%20==1) pop[i][j]=((rand()%(scheme[j][1]-scheme[j][0]))+scheme[j][0]);
      fit[i]=-1.0;
    }
  }
}