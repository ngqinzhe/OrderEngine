CC = g++
CFLAGS = -Wall -Werror -std=c++20 -O2
BOOST = -I/opt/homebrew/Cellar/boost/1.76.0/include

network: server.cpp client.cpp
	$(CC) $(CFLAGS) $(BOOST) server.cpp ../orderEngine.cpp -o server
	$(CC) $(CFLAGS) $(BOOST) client.cpp -o client
