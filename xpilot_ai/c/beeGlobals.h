//
// Created by makowskyb on 8/14/19.
//

#ifndef XPILOT_LE_BEEGLOBALS_H
#define XPILOT_LE_BEEGLOBALS_H

#include <stdio.h>
//Macro open a log file appending
#define X_COORD_START 1
#define OPENLOG()   fp = fopen(LogFile, "a");
#define CLOSE_LOG() fclose(fp);
extern char bugstring[50];
extern char LogFile[20];
extern FILE *fp;

#endif //XPILOT_LE_BEEGLOBALS_H
