##DumbProtocol
An application layer Go-Back-N reliable file transfer program on UDP.
This application uses libevent to implement a FSM (finite state machine).

##Usage

Make file:
    
    make

Run:

    ./server <portNumber> <windowSize> <lostRate> <corruptionRate>
    ./client <serverIP> <portNumber> <fileName> <lostRate> <corruptionRate>   

	The executables will be generated in server/ and client/ folder,
	respectively.

##Developer
Xi Han 504136747 <xihan94@ucla.edu>

Runhang Li 204134617 <marklrh@gmail.com>


