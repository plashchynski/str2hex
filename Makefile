SRCS = main.c b64.c md5.c process.c
OBJS = $(SRCS:.c=.o)

all: str2hex

str2hex: $(OBJS)
	gcc $(OBJS) -o $@

.c.o:
	gcc -Wall -g -c $^ -o $@

clean:
	rm -f *.o
	rm -f str2hex
