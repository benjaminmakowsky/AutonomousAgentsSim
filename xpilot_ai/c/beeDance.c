//
// Created by makowskyb on 8/17/19.
//

#include "beeDance.h"
#include "beeObject.h"

//Local Variables
static char danceMoves[4] = {none,none,none,none}; //Array to hold dance moves
int msgType[2] = {foundSource, foundEnemy};


int danceTree[10] = {0,1,2,3,4,5,6,7,8,9};

//Helper Methods
int getParent(int child){
    return danceTree[child/2 -1];
}


char* buildDance(int msgType){
    int* msgPointer;
    int msg[3]; //0: type 1: x coord 2: y_coord

    switch (msgType)
    {
        case foundSource:
            msg[0] = msgType;
    }

}
