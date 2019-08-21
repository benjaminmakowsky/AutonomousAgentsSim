//
// Created by makowskyb on 8/17/19.
//
#ifndef XPILOT_LE_BEE_DANCE_H
#define XPILOT_LE_BEE_DANCE_H

#include <stdbool.h>

//Defined values
#define endOfMSGSig 16    //16 frames for end of message
#define endOfWordSig 8    //8 frames for end of word
#define endOfSymbolSig 4  //4 frames for end of symbol
#define foundSource 0
#define foundEnemy 1
#define left 'l'
#define right 'r'
#define endOfSequence 'p' //p for pause
#define none '\0'

//Array to hold message types ie (0: found honey, 1: found enemy hive) etc
//Using an int so that when the message is transmitted its all in ints for the coordinates
extern int msgTypes[2];

/// Relays the coordinates of the message through dance
/// \param coords the coordinates to relay
/// \return whether or not the coordinates have been relayed
bool relayCoords(int coords);

/// Performs the dance for the bee
/// \param msgType The msg to dance
/// \return whether or not the dance was completed
bool dance(int msgType);







/**********************************************************
 * Helper Methods
 *********************************************************/

/// Builds the message for the dance
/// \param msgType
/// \return
char* buildDance(int coords);

/// Performs dance movements according to dance symbol
/// \param symbol The symbol to dance
/// \return true/false depending on if the move has been finished
bool relayMsg(int symbol);

/// Function to get the depth of a number from tree
/// \param number the number being looked for
/// \return the depth
int getDepthOfNumber(int number);


/// Performs the dance sequence
/// \param sequence
/// \return
bool performSequence(char* sequence);

#endif //XPILOT_LE_BEE_DANCE_H
