BIN = testunifold
SRC = guniprop.c gunidecomp.c gutf8.c main.c
IN := "TesTÜ"
KSRC := /usr/src/linux-headers-3.8.0-26-generic

all: $(BIN)
	$(PWD)/$(BIN) $(IN)

$(BIN): $(SRC) $(wildcard *.h)
	gcc -Wall -O2 -o $(BIN) $(SRC)

clean:
	-$(MAKE) -C $(KSRC) M=`pwd` clean
	rm -f $(BIN)

module:
	$(MAKE) -C $(KSRC) M=`pwd`

obj-$(CONFIG_NLS_UTF8) += nls_utf8.o
nls_utf8-y := gunidecomp.o guniprop.o gutf8.o

