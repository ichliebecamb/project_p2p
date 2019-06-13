VPATH=.
cc=g++
serfile=Server.o \
		Server_main.o \
		libevent.o
clifile=Client.o \
		Client_main.o \
		mediaplay.o \
		libevent.o
exe:server client
server:$(serfile)
	g++ -o server $(serfile)
client:$(clifile)
	g++ -o client $(clifile)
$(serfile):
$(clifile):

clean:
	rm *.o server client
