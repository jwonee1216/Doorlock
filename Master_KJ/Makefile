SRC_PATH = ./src
LIB_PATH = ./include

#FLAGS = -pthread -DDEBUG

FLAGS = -pthread

TAR = main

ALL : $(TAR)

$(TAR) : master 
	gcc -o $(TAR) $(TAR).c *.o -I$(LIB_PATH) $(FLAGS)

master : queue server
	gcc -c $(SRC_PATH)/master.c -I$(LIB_PATH) $(FLAGS)

queue :
	gcc -c $(SRC_PATH)/queue.c -I$(LIB_PATH) 

server :
	gcc -c $(SRC_PATH)/server.c -I$(LIB_PATH) 

clean :
	rm -r main
	rm -r *.o
