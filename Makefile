# Basic compilation macros
CC = gcc # GCC Compiler
CFLAGS = -ansi -Wall -pedantic # Flags
GLOBAL_DEPS = globals.h # Dependencies for everything
EXE_DEPS = assembler.o code.o fpass.o spass.o instructions.o table.o utilities.o writefiles.o # Deps for exe

## Executable
assembler: $(EXE_DEPS) $(GLOBAL_DEPS)
	$(CC) -g $(EXE_DEPS) $(CFLAGS) -o $@

## Main:
assembler.o: assembler.c $(GLOBAL_DEPS)
	$(CC) -c assembler.c $(CFLAGS) -o $@

## Code helper functions:
code.o: code.c code.h $(GLOBAL_DEPS)
	$(CC) -c code.c $(CFLAGS) -o $@

## First Pass:
fpass.o: first_pass.c first_pass.h $(GLOBAL_DEPS)
	$(CC) -c first_pass.c $(CFLAGS) -o $@

## Second Pass:
spass.o: second_pass.c second_pass.h $(GLOBAL_DEPS)
	$(CC) -c second_pass.c $(CFLAGS) -o $@

## Instructions helper functions:
instructions.o: instructions.c instructions.h $(GLOBAL_DEPS)
	$(CC) -c instructions.c $(CFLAGS) -o $@

## Table:
table.o: table.c table.h $(GLOBAL_DEPS)
	$(CC) -c table.c $(CFLAGS) -o $@

## Useful functions:
utilities.o: utilities.c instructions.h $(GLOBAL_DEPS)
	$(CC) -c utilities.c $(CFLAGS) -o $@

## Output Files:
writefiles.o: writefiles.c writefiles.h $(GLOBAL_DEPS)
	$(CC) -c writefiles.c $(CFLAGS) -o $@

# Clean Target (remove leftovers)
clean:
	rm -rf *.o
