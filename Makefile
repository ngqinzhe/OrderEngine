CC = g++
CFLAGS = -Wall -Werror -std=c++20 -O2

main: main.cpp orderEngine.cpp
	$(CC) $(CFLAGS) main.cpp orderEngine.cpp