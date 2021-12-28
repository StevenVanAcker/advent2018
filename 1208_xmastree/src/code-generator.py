#!/usr/bin/env python3

import sys, string, pprint, random, hashlib
flag = open(sys.argv[1]).read().strip()

chunksize = 3
fakeopts = 2

charset = "".join([x for x in string.printable if x not in string.whitespace])

# this is the list of all possible indexes. We must use each one at least once
AllIndexes = list(range(len(flag))) * chunksize
random.shuffle(AllIndexes)

# (real + fake + else) ^ depth
num = (fakeopts + 2) ** len(flag)
print("Generating with depth {} = {} hashes".format(len(flag), num), file=sys.stderr)

def getRandomString(n):
    return "".join([random.choice(charset) for _ in range(n)])

def print_branch(isTrunk):
    ret = None

    if isTrunk:
        ret = "This is the trunk, it has a tiny flag pinned to it!";
    else:
        ret = random.choice([
        "This branch is empty",
        "This branch is empty",
        "This branch is empty",
        "This branch has a red ball",
        "This branch has a green ball",
        "This branch has a lightbulb",
        "There is some glitter on this branch",
        "There is some snow on this branch",
        ])

    return 'printf("{}\\n");'.format(ret)

def md5(m):
    return hashlib.md5(m.encode()).hexdigest()

def sha1(m):
    return hashlib.sha1(m.encode()).hexdigest()

def sha256(m):
    return hashlib.sha256(m.encode()).hexdigest()

def generate_ifthenelse(varname, data):
    random.shuffle(data)
    out = ""

    for i, (k, c) in enumerate(data):
        if i == 0:
            out += 'if(!strcmp({}, "{}")) {{ {} }}'.format(varname, k, c)
        else:
            out += ' else if(!strcmp({}, "{}")) {{ {} }}'.format(varname, k, c)
    out += ' else {{ {} }}'.format(print_branch(False))

    return out

def prelude(fnname, inputbuffer, varname, indexes):
    return "{}({}, {}, {});".format(fnname, inputbuffer, varname, ",".join([str(x) for x in indexes]))


def generate_code(indexes, isTrunk):
    if len(indexes) < chunksize:
        print("len(indexes) < chunksize")
        sys.exit(1)

    thischunk = indexes[:chunksize]
    rest = indexes[chunksize:]

    if isTrunk:
        thischunkstr = "".join([flag[x] for x in thischunk])
    else:
        thischunkstr = getRandomString(chunksize)

    data = []
    fns = [ (md5, "md5"), (sha1, "sha1"), (sha256, "sha256") ]
    fn, fnname = random.choice(fns)
    # are we on the final chunk?
    if len(indexes) == chunksize:
        data = [(fn(getRandomString(chunksize)), print_branch(False)) for _ in range(fakeopts)]
        data += [(fn(thischunkstr), print_branch(isTrunk))]

    else:
        data = [(fn(getRandomString(chunksize)), generate_code(rest, False)) for _ in range(fakeopts)]
        data += [(fn(thischunkstr), generate_code(rest, isTrunk))]

    return prelude(fnname, "userstring", "hash", thischunk) + generate_ifthenelse("hash", data)
    

code = generate_code(AllIndexes, True)
print("""
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

#define hashfn(fn, inb, outb, x, y, z) \\
	char tempb[3]; \\
	char outb[100]; \\
	tempb[0] = inb[x]; \\
	tempb[1] = inb[y]; \\
	tempb[2] = inb[z]; \\
        fn(tempb, 3, outb);\\
        \\

#define md5(inb, outb, x, y, z) hashfn(__m5, inb, outb, x, y, z)
#define sha1(inb, outb, x, y, z) hashfn(__s1, inb, outb, x, y, z)
#define sha256(inb, outb, x, y, z) hashfn(__s2, inb, outb, x, y, z)

char *userstring;

int main(int argc, char **argv) {{
    if(argc < 2) {{
        printf("No arg\\n");
        exit(1);
    }}

    userstring = argv[1];

    if(strlen(userstring) > {}) {{
        printf("Input is too long\\n");
        exit(1);
    }}

{}
}}
""".format(len(flag), code))

    
