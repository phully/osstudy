all: NetworkTransfer.exe

NetworkTransfer.exe: NetworkTransfer.c
	gcc -o $@ $< -lws2_32
	
clean:
	rm -f NetworkTransfer.exe
