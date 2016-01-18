PROGRAM = fork_test

all: $(PROGRAM)

$(PROGRAM): *.c
	$(CC) -o $@ $+

.PHONY: all
