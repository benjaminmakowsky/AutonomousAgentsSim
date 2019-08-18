//
// Created by makowskyb on 8/17/19.
//

#ifndef XPILOT_LE_BEE_DANCE_H
#define XPILOT_LE_BEE_DANCE_H

//Defined values
#define foundSource 0
#define foundEnemy 1
#define left 'l'
#define right 'r'
#define none '\0'

//Array to hold message types ie (0: found honey, 1: found enemy hive) etc
//Using an int so that when the message is transmitted its all in ints for the coordinates
extern int msgType[2];

/// Builds the message for the dance
/// \param msgType
/// \return
char* buildDance(int msgType);



#endif //XPILOT_LE_BEE_DANCE_H
