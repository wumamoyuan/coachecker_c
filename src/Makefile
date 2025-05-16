CC = gcc
CFLAGS = -Wall -Wextra -I.
LDFLAGS = -L../lib -ll

# 获取所有.c文件
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# 目标可执行文件
TARGET = ../bin/coachecker

# 默认目标
all: $(TARGET)

# 链接规则
$(TARGET): $(OBJS)
	@mkdir -p ../bin
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
