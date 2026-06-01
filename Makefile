CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude
SRCDIR = src
OBJDIR = obj
TESTDIR = tests

SRCS = $(SRCDIR)/arena.c $(SRCDIR)/palaver.c $(SRCDIR)/session.c \
       $(SRCDIR)/coalition.c $(SRCDIR)/convergence.c $(SRCDIR)/dialogue.c \
       $(SRCDIR)/palaver_api.c

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

LIB = libpalaver.a
TEST_BIN = test_palaver

.PHONY: all clean test

all: $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIB): $(OBJS)
	ar rcs $@ $^

test: $(LIB) $(TESTDIR)/test_main.c
	$(CC) $(CFLAGS) -o $(TEST_BIN) $(TESTDIR)/test_main.c -L. -lpalaver -lm
	./$(TEST_BIN)

clean:
	rm -rf $(OBJDIR) $(LIB) $(TEST_BIN)
