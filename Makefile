
CXXFLAGS=-Wall -O2 -std=c++17
LDFLAGS=-lutil

dosh: dosh.cc

clean:
	rm -f dosh

install: dosh
	sudo install -c -o root -g root -m 755 dosh /bin
