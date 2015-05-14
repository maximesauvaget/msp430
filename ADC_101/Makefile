TARGET := adc.elf
CC	:= msp430-gcc
CFLAGS	:= -Os -Wall -mmcu=msp430g2553 
OBJS	:= main.o msplib.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS) -lm

clean:
	rm -rf $(TARGET) $(OBJS)

prog: $(TARGET)
	mspdebug -q rf2500 "prog $(TARGET)"
