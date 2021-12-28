TESTUSER = Steven

CFLAGS := $(shell pkg-config --cflags 'libprotobuf-c >= 1.0.0')
LDFLAGS := $(shell pkg-config --libs 'libprotobuf-c >= 1.0.0') -lcrypto

CLIBFILES := udpsanta.pb-c.c tools.c requests.c crypto.c network.c responses.c userdata.c
TARGETS := server client

all: $(TARGETS)

client: client.c
server: server.c

$(TARGETS): $(CLIBFILES)
	gcc $(CFLAGS) $@.c $(CLIBFILES) -o $@ $(LDFLAGS)

udpsanta.pb-c.c: udpsanta.proto
	protoc-c udpsanta.proto --c_out=.

udpsanta.pb-c.h: udpsanta.proto
	protoc-c udpsanta.proto --c_out=.

clean:
	rm -f *.pb-c.* *.bin $(TARGETS) key.priv key.pub

clientstuff: 
	@mkdir -p ~/.udpsanta
	@cp data/keys/$(TESTUSER).pub ~/.udpsanta/public.key
	@cp data/keys/$(TESTUSER).priv ~/.udpsanta/private.key
	@/bin/echo -en "$(TESTUSER)" > ~/.udpsanta/user.name

