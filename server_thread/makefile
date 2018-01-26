LD=`pkg-config --cflags --libs opencv`
FLAGS=-g -lpthread -Wall
G=g++ -std=c++11

all:client server 
client:client.h client.cpp efun.h efun.cpp
	$(G) $(FLAGS) -o client client.cpp efun.cpp efun.h 
server:main.cpp server.h server.cpp efun.h Queue.h Queue.cpp efun.cpp Epoll.h Epoll.cpp ThreadPool.h ThreadPool.cpp Data.h Log.h Log.cpp Lock.h
	$(G) $(FLAGS) -o server main.cpp server.h server.cpp efun.h Queue.h Queue.cpp efun.cpp Epoll.h Epoll.cpp ThreadPool.h ThreadPool.cpp Data.h Log.h Log.cpp Lock.h $(LD)
clean:
	rm client server
