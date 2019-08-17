//
// Created by makowskyb on 8/17/19.
//

#include "bee_lang.h"
#include "beeObject.h"


int msgType[2] = {foundSource, foundEnemy};








int* buildMsg(int msgType){
    int* msgPointer;
    int msg[3]; //0: type 1: x coord 2: y_coord

    switch (msgType)
    {
        case foundSource:
            msg[0] = msgType;
            msg[1] =
    }

}
