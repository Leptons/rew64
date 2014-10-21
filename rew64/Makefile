PROGRAM = rew64
CPU_DIR = ../CPU
TARGET_DIR = obj
VPATH = $(CPU_DIR):$(TARGET_DIR)
OBJS = rew64.o utils.o inst.o mem.o run.o
CC = gcc
CFLAGS += -Wall -I $(CPU_DIR)


$(PROGRAM): $(OBJS)
	$(CC) $^ $(CFLAGS) -o $(TARGET_DIR)/$(PROGRAM)
	
%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
