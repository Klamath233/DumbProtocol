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
	string ipaddress = "";
	int portNumber = 0;
	int windowSize = 0;
	float pLossPoss = 0.0, pCoruPoss = 0.0;
	if (argc < 4)
	{
		fprintf(stderr, "Error: invalid number of arguments\nExitting...\n");
		exit(1);
	}

	ipaddress = argv[1];
	portNumber = atoi(argv[2]);
	windowSize = atoi(argv[3]);
	pLossPoss = atof(argv[4]);
	pCoruPoss = atof(argv[5]);
	if (pLossPoss < 0.0 || pLossPoss > 1.0 ||
	    pCoruPoss < 0.0 || pCoruPoss > 1.0)
	{
		fprintf(stderr, "Error, invalud possibility\nExitting\n");
		exit(1);
	}

	RunSender(ipaddress, portNumber, windowSize, pLossPoss, pCoruPoss);
	return 0;

	
}
