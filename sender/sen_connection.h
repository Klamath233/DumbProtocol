#ifndef SEN_CONNECTION_H
#define SEN_CONNECTION_H

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;

void RunSender(string portNumber, int windowSize, int pLossPoss, int pCoruPoss);


#endif
