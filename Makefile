CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm
SRC_DIR = src
INC_DIR = include
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = shell.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)