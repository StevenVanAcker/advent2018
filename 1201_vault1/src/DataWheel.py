#!/usr/bin/env python3

import random

class DataWheel():
    def __init__(self, chars="0123456789", spacing=1, pos=None):
        self.chars = chars
        self.spacing = spacing

        xformat = "{{:<{}}}".format(spacing+1) if spacing > 0 else "{}"
        self.spacedChars = "".join([xformat.format(x) for x in chars])

        if pos == None:
            self.currentPos = (spacing+1) * random.randint(-10000, 10000)
        else:
            self.currentPos = pos

        self.currentPos = self._wrapNum(self.currentPos)
            
    def _wrapNum(self, n):
        val = n
        maxval = len(self.spacedChars)
        while val >= maxval:
            val -= maxval
        while val < 0:
            val += maxval
        return val

    def move(self, offset):
        self.currentPos = self._wrapNum(self.currentPos + offset)

    def resultOfMove(self, lineoffset = 0):
        val = self._wrapNum(self.currentPos + lineoffset)
        return self.spacedChars[val]

    def bestMove(self, tochar):
        idx = self.spacedChars.find(tochar)
        wheellen = len(self.spacedChars)

        move = idx - self.currentPos

        if idx < self.currentPos:
            # 0 .... T .... C .... max
            wrapmove = move + wheellen 
        else:
            # 0 .... C .... T .... max
            wrapmove = move - wheellen 

        #print("Move from {} to {} move {} wrapmove {}".format(self.currentPos, idx, move, wrapmove))
        if abs(move) < abs(wrapmove):
            return move
        return wrapmove



if __name__ == "__main__":
    import sys, string

    chars = string.ascii_uppercase

    for sp in range(0, 10):
        dw = DataWheel(chars=chars, spacing=sp)
        for p in range(len(dw.spacedChars)):
            dw = DataWheel(chars=chars, spacing=sp, pos=p)
            for c in chars:
                m = dw.bestMove(c)
                nc = dw.resultOfMove(m)
                #print("{} -> {}: best={}, result={}".format(dw.resultOfMove(), c, m, nc))
                if c != nc or (2 * abs(m)) > len(dw.spacedChars):
                    print("FAIL")
                    raise(ValueError("FAIL"))
    sys.exit(0)

