//
// Created by makowskyb on 9/17/19.
//

#ifndef XPILOT_LE_BEEOBSERVE_H
#define XPILOT_LE_BEEOBSERVE_H

#include <stdbool.h>
#define endOfWord '_'
#define left 'l'
#define right 'r'
#define max_dance_moves 25 //1 for dance type, 2*12 for max moves for coordinate 9999


/// Observes the ship with the specified ID to determine what dance they are conveying
/// \param ship_idx the id of the ship to observe
/// \return the dance observed
int observeDance(int ship_idx);

/// Observes a ship to determine all the moves it sees during a dance
/// \param ship_id the id of the ship to observe
/// \return a pointer to the array of the dance moves recorded
char* observeDanceMoves(int ship_id);

/// Determines whether or not a bee is dancing by determining if it is changing heading
/// \param ship_id the id of the ship to observe
/// \return boolean value if bee heading is changing or not
bool beeIsDancing(int ship_id);

/// Used to determine coordinates transmitted during dance
/// \param coord char represent x or y
/// \param dance the dance to be observed
/// \return to corresponding coordinate
int interpretCoord(char coord, char* dance);

/// Converts dances moves to integers
/// \param moves array holding the moves to interpret
/// \return int value of the moves observed
char convertToNumber(char* moves);
#endif //XPILOT_LE_BEEOBSERVE_H
