#!/usr/bin/python

import sys, time, string
from TelnetProtocol import TelnetProtocol
from DataWheel import *


victoryfile = sys.argv[1]
flagfile = sys.argv[2]

charset = string.ascii_uppercase
wheelSpacing = 2
wheelWidth = 4
marginLines = 4

colorOffset = {
    "black": 0,
    "red": 1,
    "green": 2,
    "yellow": 3,
    "blue": 4,
    "magenta": 5,
    "cyan": 6,
    "white": 7,
    "none": 9,
}

DEBUG = False
rotateAllParallel = False

class Vault:
    def __init__(self, wheelCount = 5):
        self.animationDelay = 0.03
        self.typeDelay = 0
        self.inputBuffer = ""
        self.guess = None
        self.outputBuffer = ""
        self.telnetproto = None
        self.returnCode = False

        self.wheels = [DataWheel(chars=charset, spacing=wheelSpacing) for x in range(wheelCount)]
        self.solution = "".join([random.choice(charset) for x in range(wheelCount)])

        self.startX = 5
        self.startY = 6

        self.cursorX = 0
        self.cursorY = 0

    def currentCode(self):
        return "".join([x.resultOfMove() for x in self.wheels])

    def clearScreen(self):
        print "\x1b[2J",
        print "\x1b[34;1m",
        sys.stdout.flush()

    def clearLine(self, y):
        sys.stdout.write("\x1b[%d;%dH" % (y,0))
        sys.stdout.write("\x1b[2K\x1b[G")
        sys.stdout.flush()

    def clearToEOL(self):
        sys.stdout.write("\x1b[K")
        sys.stdout.flush()

    def resetColor(self):
        sys.stdout.write("\x1b[0m")
        sys.stdout.flush()

    def setBgColor(self, color):
        colcode = 40 + colorOffset[color]
        sys.stdout.write("\x1b[%dm" % (colcode))
        sys.stdout.flush()

    def setColor(self, color, bold=False):
        colcode = 30 + colorOffset[color]
        if bold:
            sys.stdout.write("\x1b[%d;1m" % (colcode))
        else:
            sys.stdout.write("\x1b[%dm" % (colcode))
        sys.stdout.flush()

    def goto(self, x,y):
        sys.stdout.write("\x1b[%d;%dH" % (y,x))
        sys.stdout.flush()

    def setCursor(self, x, y):
        self.cursorX = x
        self.cursorY = y
        self.gotoCursor()

    def gotoCursor(self):
        self.goto(self.cursorX, self.cursorY)

    def putChar(self, x,y,c):
        self.goto(x,y)
        sys.stdout.write("%s" % (c,))
        sys.stdout.flush()

    def putText(self, startX, startY, text):
        for i in range(len(text)):
            self.putChar(i + startX, startY, text[i])
            time.sleep(self.typeDelay)

    def rotationsToMessage(self, msg, best=True):
        rots = []
        for x,w in enumerate(self.wheels):
            move = w.bestMove(msg[x])
            if not best:
                move += (wheelSpacing+1) * len(charset) * random.randint(-2, 2)
            rots += [move]

        return rots


    def randomRotations(self, maxval, aligned=True):
        mult = 1
        if aligned:
            mult = (wheelSpacing+1)
        return [int(random.randint(-maxval, maxval)/mult)*mult for x in self.wheels]

    def calculateRotations(self, rotations):
        if len(rotations) != len(self.wheels):
            print("invalid input")
            return None

        # rotations are given in lettercount, not spacingcount
        rotations = [(wheelSpacing+1) * int(x) for x in rotations]

        for x,w in enumerate(self.wheels):
            move = w.bestMove(self.solution[x])

            # VULN (dataleak):
            #   if the user guessed wrong, then perform his amount of rotations
            #   if the user guessed right, then perform the best move, no matter what the user asked for
            if w.resultOfMove(move) != w.resultOfMove(rotations[x]):
                move = rotations[x]

            rotations[x] = move
        return rotations

    def drawBox(self):
        width = wheelWidth * (len(self.wheels) + 1) - 1
        spaceline = " " * width
        markline = ">" + " " * (width-2) + "<"

        # color boxes first
        self.setBgColor("blue")
        self.putText(self.startX - 1, -5 + self.startY, spaceline)
        self.setBgColor("cyan")
        self.putText(self.startX - 1, -4 + self.startY, spaceline)
        self.putText(self.startX - 1, -3 + self.startY, spaceline)
        self.setBgColor("white")
        self.putText(self.startX - 1, -2 + self.startY, spaceline)
        self.putText(self.startX - 1, -1 + self.startY, spaceline)
        self.putText(self.startX - 1, 0 + self.startY, markline)
        self.putText(self.startX - 1, 1 + self.startY, spaceline)
        self.putText(self.startX - 1, 2 + self.startY, spaceline)
        self.setBgColor("cyan")
        self.putText(self.startX - 1, 3 + self.startY, spaceline)
        self.putText(self.startX - 1, 4 + self.startY, spaceline)
        self.setBgColor("blue")
        self.putText(self.startX - 1, 5 + self.startY, spaceline)

        self.resetColor()
        # wheel cutouts
        for l in range(-marginLines, marginLines + 1):
            for x in range(len((self.wheels))):
                self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX - 1, l + self.startY, " ")
                self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX + 0, l + self.startY, " ")
                self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX + 1, l + self.startY, " ")


    def animateRotations(self, rotations, parallel=False):
        # clear screen
        self.clearScreen()
        self.drawBox()

        # first, draw all the wheels
        for l in range(-marginLines, marginLines + 1):
            for x,w in enumerate(self.wheels):
                if l == 0:
                    self.setColor("red", bold=True)
                else:
                    self.setColor("none", bold=False)
                self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX, l + self.startY, w.resultOfMove(l))

        # next, do the animations
        if parallel:
            done = False
            while not done:
                done = True

                # move all the wheels
                for x,w in enumerate(self.wheels):
                    if rotations[x] > 0:
                        w.move(1)
                        rotations[x] -= 1
                        done = False
                    elif rotations[x] < 0:
                        w.move(-1)
                        rotations[x] += 1
                        done = False

                # next, draw all the wheels
                for l in range(-marginLines, marginLines + 1):
                    for x,w in enumerate(self.wheels):
                        if l == 0:
                            self.setColor("red", bold=True)
                        else:
                            self.setColor("none", bold=False)
                        self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX, l + self.startY, w.resultOfMove(l))

                time.sleep(self.animationDelay)
        else:
            for x,w in enumerate(self.wheels):
                for step in range(abs(rotations[x])):
                    move = rotations[x]
                    w.move(1 if move > 0 else -1)
                    for l in range(-marginLines, marginLines + 1):
                        if l == 0:
                            self.setColor("red", bold=True)
                        else:
                            self.setColor("none", bold=False)
                        self.putChar(x*wheelWidth + (wheelWidth/2) + self.startX, l + self.startY, w.resultOfMove(l))
                    time.sleep(self.animationDelay)

        self.resetColor()

    def getUserGuess(self):
        # try parsing the user input and return it if valid
        guess = []
        validchars = string.digits + " " + "-"

        if len(self.inputBuffer) > 200:
            return (None, "Input too long.")

        if self.inputBuffer == "":
            return (None, "For each of the {} wheels shown above, enter the amount of letters to move up or down to form the access code, then press enter.".format(len(self.wheels)))


        if len([c for c in self.inputBuffer if c not in validchars]) > 0:
            return (None, "Invalid characters. Only digits, '-' and space are allowed")

        guess = [x for x in self.inputBuffer.strip().split() if x != ""]

        allok = False
        try:
            allok = all([x == str(int(x)) for x in guess])
        except:
            pass

        if not allok:
            return (None, "Not all specified values are valid numbers.")

        guess = [int(x) for x in guess]

        if len(guess) != len(self.wheels):
            return (None, "Expecting {} values, but only {} specified.".format(len(self.wheels), len(guess)))

        allok = False
        try:
            allok = all([x >= -100 and x <= 100 for x in guess])
        except:
            pass

        if not allok:
            return (None, "Not all specified values are in the range -100 to 100.")

        return (guess, "OK")

    def clearAndWritePrompt(self):
        promptX = 1
        promptline = 14
        prompt = "Enter access code: "

        self.clearLine(promptline)
        self.setColor("blue", True)
        self.putText(promptX, promptline, prompt)
        self.resetColor()
        self.putText(promptX + len(prompt), promptline, self.inputBuffer)
        self.setCursor(promptX + len(prompt) + len(self.inputBuffer), promptline)

    def clearAndWriteOutput(self):
        outputX = 1
        outputY = 16

        self.clearLine(outputY)
        self.setColor("none", False)
        self.putText(outputX, outputY, ">> " + self.outputBuffer)
        self.resetColor()

    def victory(self):
        flagX = 11
        flagY = 4
        endX = 0
        endY = 50
        flagcolor = "red"
        flagbgcolor = "black"

        victorytext = open(victoryfile).read()
        flagtext = open(flagfile).read().strip()

        self.clearScreen()
        self.resetColor()
        self.goto(0,0)

        self.setColor("green", bold=True)
        for i in range(20):

            dotcount = 1 + (i % 3)
            dots = "." * dotcount

            self.clearLine(5)
            self.putText(5,5,"Vault unlocked. Opening door "+dots)
            time.sleep(0.5)


        self.clearScreen()
        self.resetColor()
        self.goto(0,0)
        sys.stdout.write(victorytext)

        self.setColor(flagcolor)
        self.setBgColor(flagbgcolor)
        self.putText(flagX, flagY, flagtext)
        time.sleep(3)

        self.resetColor()
        self.goto(0,0)
        sys.stdout.write(victorytext)

        sys.exit(0)

    def init(self, e, v):
        print "\xff\xfd\x22", # telnet proto init

        rots = self.randomRotations(100)
        self.animateRotations(rots, parallel=True)

        self.outputBuffer = "For each of the {} wheels shown above, enter the amount of letters to move up or down to form the access code, then press enter.".format(len(self.wheels))

        self.clearAndWriteOutput()
        self.clearAndWritePrompt()

        if DEBUG:
            lala = " ".join(["{}".format(x/3) for x in self.rotationsToMessage(self.solution)])
            self.putText(1, 20, "{}".format(lala))


    def handleKeyPress(self, e, (key,)):
        if key == "\n":
            if self.guess == None:
                self.outputBuffer = "Invalid input."
            else:
                rots = self.calculateRotations(self.guess)
                self.animateRotations(rots, parallel=rotateAllParallel)

                if self.currentCode() == self.solution:
                    self.victory()
                else:
                    self.outputBuffer = "Wrong access code: Access Denied!"
                    self.clearAndWriteOutput()

                    time.sleep(2)

                    rots = self.randomRotations(100)
                    self.animateRotations(rots, parallel=True)

                    if DEBUG:
                        lala = " ".join(["{}".format(x/3) for x in self.rotationsToMessage(self.solution)])
                        self.putText(1, 20, "{}".format(lala))


            self.inputBuffer = ""
            self.clearAndWriteOutput()
            self.clearAndWritePrompt()
            return

        else:
            # make backspace work
            if ord(key) == 127:
                # backspace
                if len(self.inputBuffer) > 0:
                    self.inputBuffer = self.inputBuffer[:-1]
                    self.cursorX -= 1
            else:
                if len(self.inputBuffer) < 300:
                    self.inputBuffer += key
                    self.cursorX += 1


            (self.guess, reason) = self.getUserGuess()
            if self.guess == None:
                self.outputBuffer = reason
            else:
                self.outputBuffer = "OK. Press enter to rotate the wheels.".format(self.guess)

            self.clearAndWriteOutput()
            self.clearAndWritePrompt()
    

    def start(self):
        self.telnetproto = TelnetProtocol(sys.stdin)
        self.telnetproto.registerHandler("init", "init", self) 
        self.telnetproto.registerHandler("keypress", "handleKeyPress", self) 
        self.telnetproto.registerHandler("closed", sys.exit) 
        self.telnetproto.start()
        return self.returnCode

if __name__ == '__main__':
    wg = Vault(wheelCount = 20)
    if wg.start():
        print "Continuing..."
    else:
        print "Done"

