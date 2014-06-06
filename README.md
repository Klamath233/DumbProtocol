##DumbProtocol
An application layer Go-Back-N reliable file transfer program on UDP.
This application uses libevent to implement a FSM (finite state machine).

##Usage

Make file:
    
    make

Run:

    ./server <portNumber> <windowSize> <lostRate> <corruptionRate>
    ./client <serverIP> <portNumber> <fileName> <lostRate> <corruptionRate>   

##Developer
Xi Han 

Runhang Li

