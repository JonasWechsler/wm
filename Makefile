CFLAGS?=-Wall -Werror -std=c++1y -g
PKG?=`pkg-config --cflags --libs xcb`

all:
	gcc $(CFLAGS) *.cc -o main $(PKG)

clean:
	rm -f main *.o *.d

