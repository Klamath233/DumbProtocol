#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "sen_connection.h"

using namespace std;

int
main (int argc, char **argv)
{
	string portNumber = "";
	int windowSize = 0;
	float pLossPoss = 0.0, pCoruPoss = 0.0;
	if (argc < 4)
	{
		fprintf(stderr, "Error: invalid number of arguments\nExitting...\n");
		exit(1);
	}

	portNumber = argv[1];
	windowSize = atoi(argv[2]);
	pLossPoss = atof(argv[3]);
	pCoruPoss = atof(argv[4]);
	if (pLossPoss < 0.0 || pLossPoss > 1.0 ||
	    pCoruPoss < 0.0 || pCoruPoss > 1.0)
	{
		fprintf(stderr, "Error, invalud possibility\nExitting\n");
		exit(1);
	}

	RunSender(portNumber, windowSize, pLossPoss, pCoruPoss);
	return 0;

	
}
