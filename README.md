##DumbProtocol
An application layer Go-Back-N reliable transfer protocol using UDP.
This application uses libev to implement timeout feature.

##Usage

Make file:
    
    make

Run:

    ./server <portNumber> <windowSize> <lostRate> <corruptionRate>
    ./client <serverIP> <portNumber> <fileName> <lostRate> <corruptionRate>   

##Developer
Xi Han 

Runhang Li


