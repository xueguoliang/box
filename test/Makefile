

all:
	g++ server.cpp cJSON.c -o server.bin -I../src -L../src -lbox -lpthread
	g++ client.cpp cJSON.c -o client.bin -I../src -lpthread


clean:
	rm -rf server.bin client.bin


