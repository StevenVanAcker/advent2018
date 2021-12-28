#!/usr/bin/python

from Field import Field

class RenderedField:
    colorOK = "\x1b[32;1;42%sm"
    colorBAD = "\x1b[33;1;43%sm"
    colorVERYBAD = "\x1b[31;1;41%sm"
    cursorSelect = ";4;7;34"
    cursorUnselect = ""
    colorRESET = "\x1b[0m"
    def __init__(self, f):
        self.field = f
        self.cursorX = 0
        self.cursorY = 0

    def getColor(self, val, cursor):
        c = self.cursorUnselect
        if cursor:
            c = self.cursorSelect

        if abs(val) == 0:
            return self.colorOK % c
        if abs(val) <= 9:
            return self.colorBAD % c
        return self.colorVERYBAD % c

    def __str__(self):
        out = ""
        f = self.field

        for y in range(f.yDim):
            for x in range(f.xDim):
                val = f.data[y][x]
                cursor = (x == self.cursorX and y == self.cursorY)
                out += self.getColor(val, cursor)
                if val < 0:
                    out += "-"
                else:
                    out += " "
                if abs(val) <= 9:
                    out += ("%d " % abs(val))
                else:
                    out += ("%d" % abs(val))
                out += self.colorRESET

            out += "\r\n"
                
        return out

    def setCursor(self, x, y):
        self.cursorX = x
        self.cursorY = y

if __name__ == '__main__':
    x = Field(25,25)
    x.data[1][2] = 8
    x.data[2][2] = -8
    x.data[4][2] = -1
    x.data[3][3] = 2

    mypat = [[1,2,3],[4,5,6],[7,8,9]]

    x.apply(3,3, mypat)

    print RenderedField(x)

