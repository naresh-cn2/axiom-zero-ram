# Target: All builds everything
all: libaxiom.so

# How to build the library
libaxiom.so: axiom.c
	gcc -O3 -march=native -shared -o libaxiom.so -fPIC axiom.c

# Clean: Removes the binaries so you can start fresh
clean:
	rm -f libaxiom.so
