TARGET_FRONTEND = front_end2
TARGET_BACKEND = back_end

HEADER_FILES = $(wildcard *.h)
C_SOURCE_FILES = $(wildcard *.c)
OBJFILES_FRONTEND = $(filter-out back_end.o, $(C_SOURCE_FILES:.c=.o))
OBJFILES_BACKEND = $(filter-out front_end2.o, $(C_SOURCE_FILES:.c=.o))

all: $(TARGET_FRONTEND) $(TARGET_BACKEND)

clean:
	rm -f *.o
	rm -f *.fifo
	rm -f *.gch
	rm -f $(TARGET_FRONTEND) $(TARGET_BACKEND)

$(TARGET_FRONTEND): $(OBJFILES_FRONTEND) $(HEADER_FILES) $(TARGET_BACKEND)
	gcc -g -o $@ $^

$(TARGET_BACKEND): $(OBJFILES_BACKEND) $(HEADER_FILES)
	gcc -g -o $@ $^

%.o: %.c $(HEADER_FILES) 
	gcc -g -c $<
