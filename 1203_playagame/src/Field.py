#!/usr/bin/python
from random import randint, choice
from copy import deepcopy

class Field:
    data = None
    xDim = 0
    yDim = 0
    maxValue = 20
    minValue = -20

    def __init__(self, xdim, ydim):
        self.data = [[0] * xdim for x in xrange(ydim)]
        self.xDim = xdim
        self.yDim = ydim

        self.randomize()

    # do random walks around a random position
    def randomize(self):
        pattern = [[-1, 2, -1], [2, 4, 2], [-1, 2, -1]]
        reps = randint(20, 40)

        for r in range(0, reps):
            randX = randint(0, self.xDim - 1)
            randY = randint(0, self.xDim - 1)

            for i in range(randint(10, 20)):
                offsetX = choice([-1, 0, 1])
                offsetY = choice([-1, 0, 1])
                oldData = deepcopy(self.data)

                self.apply(randX + offsetX, randY + offsetY, pattern)
                if self.isFUBAR():
                    self.data = oldData

    def dump(self):
        print "\r\n".join([" ".join([("%2d" % p) for p in x]) for x in self.data])


    def isClean(self):
        return sumValues() == 0

    def sumValues(self):
        return sum([sum([abs(p) for p in x]) for x in self.data])
    

    def isFUBAR(self):
        for y in range(self.yDim):
            for x in range(self.yDim):
                if self.data[y][x] < self.minValue or self.data[y][x] > self.maxValue:
                    return True
        return False

    def apply(self, posx, posy, pat):
        patYDim = len(pat)
        patXDim = len(pat[0])
        offsetY = patYDim / 2
        offsetX = patXDim / 2

        for y in range(patYDim):
            for x in range(patXDim):
                subx = posx + x - offsetX
                suby = posy + y - offsetY

                if subx >= 0 and subx < self.xDim and suby >= 0 and suby < self.yDim:
                    self.data[suby][subx] += pat[y][x]
                


if __name__ == '__main__':
    x = Field(5,8)
    x.data[1][2] = 8
    x.data[2][2] = -8
    x.data[4][2] = -1
    x.data[3][3] = 2

    x.dump()

    mypat = [[1,2,3],[4,5,6],[7,8,9]]

    print "---"
    x.apply(3,3, mypat)
    x.dump()


