CC = gcc -g -O0
CFLAGS = -Wall `pkg-config --cflags --libs gtk+-2.0`
OBJS =  y.tab.o lex.yy.o main.o ktu_create.o ktu_eval.o gtk_create.o
TARGET = hoge

all: $(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

lex.yy.c : ktune.l
	lex ktune.l



y.tab.c : ktune.y
	yacc -dv ktune.y


clean :
	rm -f $(TARGET) *.o y.tab.c y.tab.h lex.yy.c




