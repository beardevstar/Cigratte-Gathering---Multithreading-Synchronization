CC = g++
CFLAGS  = -g -Wall

TARGET = cigbut

SRCS = hw2_output.c \
	person.cpp \
	area.cpp \
	order.cpp \
	cigbut.cpp

LFALGS = -lpthread -lm

all: $(TARGET)
	@echo  Cigbuts Compiled Successfully.

$(TARGET) : 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LFALGS)
	$(RM) *.o

clean:
	$(RM) *.o $(TARGET)