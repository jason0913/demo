.PHONY:clean

CC = gcc
RM = rm
FLAGS = -Wall -O2
RMFLAG = -rf

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
EXE = demo.exe

$(EXE): $(OBJS)
	echo "$$SRCS = $(SRCS)"
	echo "$$OBJS = $(OBJS)"
	$(CC) -o $@ $^
%.o: %.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(RMFLAG) $(EXE) $(BOJS)

