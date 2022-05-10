project = proj05

.PHONY: all
all: 
	g++ -lpthread -Wall -o $(project) proj05.student.c

.PHONY: clean
clean:
	rm -f $(project)
	rm -f *.o

