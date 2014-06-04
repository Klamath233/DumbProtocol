all: libevent server client

libevent:
	cd src/libevent && echo Configuring libevent. && ./configure --prefix=$(PWD) --silent && echo "Installing libevent." && make install > /dev/null 2> /dev/null
server:
	mkdir server && cd server && c99 -o server ../src/server.c -I$(PWD)/include -levent -L$(PWD)/lib

client:
	mkdir client && cd client && c99 -o client ../src/client.c -I$(PWD)/include -levent -L$(PWD)/lib

.PHONY:
clean:
	rm -rf bin lib include server client
