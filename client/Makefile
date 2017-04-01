all: signal_extract.c
	gcc -g -Wall -o signal_extract signal_extract.c radiotap.c -l pcap 

clean:
	rm -rf *.o signal_extract
