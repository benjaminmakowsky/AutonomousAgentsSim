#include <stdio.h>
#include <time.h>
#include <stdlib.h>
int frames=0;
int track;
AI_loop() {
	frames++;
	thrust(0);
	//Feel up them walls ;)
	if (firstFrame)
		firstFrame=false;
	if (selfSpeed() == 0)
		track = (int)selfHeadingDeg();
	else
		track = (int)selfTrackingDeg();
	wallHeadL = wallFeeler(300, angleAdd(15,track), 0, 0);
	wallHeadR = wallFeeler(300, angleDiff(15,track), 0, 0);
	wallHeadB = wallFeeler(300, angleAdd(180,(int)selfHeadingDeg()), 0, 0);
	target = angleAdd(track,(int)selfHeadingDeg());
	close2target = (target < 25 || target > 335);
	//Thrusting Rules ;)
	if (selfSpeed() < 2)
		thrust(1);
	else if (close2target)
		thrust(1);
	else if (wallHeadB < 50)
		thrust(1);
	else
		thrust(0);
	//You spin me right round Rules ;)
	turnLeft(0);
	turnRight(0);
	printf("selfHeadingDeg: %f \n",selfHeadingDeg());
	printf("aimdir: %d \n",aimdir(0));
	if (wallHeadL < 300 && close2target!=true)
		turnRight(1);
	else if (wallHeadR < 300&& close2target!=true)
		turnLeft(1);
	else if(screenEnemyXId(closestShipId())!= -1&&close2target!=true)
		turn(angleDiff((int)selfHeadingDeg(),aimdir(0)));
	//FIRE
	if (screenEnemyXId(closestShipId()) !=1)
		fire=angleDiff(aimdir(0), (int)selfHeadingDeg());
	else
		fire=500;
	if ((c[0]<fire)&&(fire<c[1]))
		fireShot();
	score=selfScore();
	printf("frames: %d score: %f", frames,score);
	if (selfAlive()==0&&aliveLast==1){
		file=fopen("fit.txt","w");
		fprintf(file,"%f\n", score);
		fclose(file);
		quitAI();
	}
	else
		aliveLast = selfAlive();
        }
int main(int argc, char *argv[]) {
  return start(argc, argv);
}
