CFLAGS = -O3 -Wall -std=c++14
CC = g++
LIB = -lpthread

rainfall: rainfall_seq.cpp rainfall_pt.cpp
	$(CC) $(CFLAGS) -o rainfall_seq rainfall_seq.cpp
	$(CC) $(CFLAGS) -o rainfall_pt rainfall_pt.cpp $(LIB)

clean:
	rm rainfall_seq rainfall_pt
