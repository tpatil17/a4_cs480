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
CCFLAGS = -std=c11 -g -Wall -c
CFLAGS = -g -c

# object files
OBJS = main.o #BitMasker.o log.o PageTableLevel.o tracereader.o tlb.o

# Program name
PROGRAM = dineseating

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.c 
	$(CC) $(CCFLAGS) main.c

#PageTableLevel.o : PageTableLevel.c PageTableLevel.h 
#	$(CC) $(CCFLAGS) PageTableLevel.c

#BitMasker.o : BitMasker.c BitMasker.h
#	$(CC) $(CCFLAGS) BitMasker.c

#tracereader.o : tracereader.c tracereader.h
#	$(CC) $(CCFLAGS) tracereader.c

#log.o : log.c log.h
#	$(CC) $(CCFLAGS) log.c

#tlb.o : tlb.c tlb.h
#	$(CC) $(CCFLAGS) tlb.c

# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -rf $(OBJS) *~ $(PROGRAM)