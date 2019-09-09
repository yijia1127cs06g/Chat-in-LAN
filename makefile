# Compiler
CC = /usr/bin/gcc

# Name of program
PROG = lanchat

# The name of the object files
OBJS = lanchat.o

# All the header and c files
SRCS = lanchat.c

# Add -I to the dir the curl include files are in
CFLAGS = -w --static

# Build the executable file
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) -lpthread

lanchat.o: lanchat.c
	$(CC) $(CFLAGS) -c lanchat.c -lpthread


# Clean up crew
clean: 
	rm -fv core* $(PROG) $(OBJS)

cleaner: clean
	rm -fv #* *~

