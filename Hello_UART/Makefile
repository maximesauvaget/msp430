TARGET := uart.elf
CC	:= msp430-gcc
CFLAGS	:= -Os -Wall -mmcu=msp430g2553
OBJS	:= main.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS)

clean:
	rm -rf $(TARGET) $(OBJS)

prog: $(TARGET)
	mspdebug -q rf2500 "prog $(TARGET)"
