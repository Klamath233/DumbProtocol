


Sender: sender.o sen_connection.o
	g++ -o $@ sender.o sen_connection.o

sender.o: sender.cpp
	g++ -c $<

sen_connection.o: sen_connection.cpp
	g++ -c $<

.PHONY:
sclean:
	rm -rf *.out *.o *.gch Sender
