CFLAGS := -Wno-everything
LDFLAGS := 

CLIBFILES := tools.c md5.c sha1.c sha256.c
TARGETS := xmastree

FLAG := ../flag

all: $(TARGETS)
	strip $(TARGETS)

xmastree: xmastree.c

$(TARGETS): $(CLIBFILES)
	clang $(CFLAGS) $@.c $(CLIBFILES) -o $@ $(LDFLAGS)

xmastree.c: code-generator.py $(FLAG)
	./code-generator.py $(FLAG) > $@

clean:
	rm -f $(TARGETS) xmastree.c *~

