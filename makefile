.PHONY:clean

CC = gcc
RM = rm
#FLAGS = -Wall -O2
FLAGS = -Wall -lpthread -g
RMFLAG = -rf

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
EXE = demo.exe

$(EXE): $(OBJS)
	@echo "$$SRCS = $(SRCS)"
	@echo "$$OBJS = $(OBJS)"
	$(CC) -o $@ $^ $(FLAGS)
%.o: %.c
	$(CC) -c $^ -o $@ $(FLAGS)

clean:
	$(RM) $(RMFLAG) $(EXE) $(OBJS)

