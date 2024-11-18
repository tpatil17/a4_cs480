#************************************************************************
# Assignment 4
# Name: Tanishq Patil
# RED IDs: 132639686
# Class: CS 480
#************************************************************************

#Makefile

#.RECIPEPREFIX +=

# Specify compiler
CC = gcc
# Compiler flags, if you want debug info, add -g
CCFLAGS = -std=c11 -g -Wall -Wextra -c -D_POSIX_C_SOURCE=199309L -fsanitize=thread -pthread
CFLAGS = -g -c

# object files
OBJS = main.o monitor.o log.o queue.o

# Program name
PROGRAM = dineseating

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.c 
	$(CC) $(CCFLAGS) main.c

monitor.o : monitor.c monitor.h seating.h
	$(CC) $(CCFLAGS) monitor.c

log.o : log.c log.h seating.h
	$(CC) $(CCFLAGS) log.c

queue.o : queue.c queue.h
	$(CC) $(CCFLAGS) queue.c

# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -rf $(OBJS) *~ $(PROGRAM)