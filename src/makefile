.PHONY:clean

CC = gcc
RM = rm
#FLAGS = -Wall -O2
FLAGS = -Wall -lpthread -D__DEBUG__ -g
RMFLAG = -rf

INCLUDE = ../include/
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
EXE = demo.exe

$(EXE): $(OBJS)
	@echo "$$SRCS = $(SRCS)"
	@echo "$$OBJS = $(OBJS)"
	$(CC) -o $@ $^ $(FLAGS)
%.o: %.c
	$(CC) -c $^  -I $(INCLUDE) -o $@ $(FLAGS)

clean:
	$(RM) $(RMFLAG) $(EXE) $(OBJS)

