#!/usr/bin/python

import sys, time
from Field import Field
from RenderedField import RenderedField
from TelnetProtocol import TelnetProtocol
from WargamesMenu import WargamesMenu
from HelpScreen import HelpScreen







allowCheat = False







realfield = Field(25,25)
renderedField = RenderedField(realfield)
posX = 0
posY = 0
currentMode = "+1"
lastResult = ""
moneySpent = 0
moneyPerUnit = 1.337

victoryText = None
skipIntro = False
cheatEnabled = False


if allowCheat:
    skipIntro = True


def log(m):
    pass
    #f = open("/tmp/logfile", "a")
    #f.write(m)
    #f.close()

def getPattern(p):
    return {
        "+1": [[   5,   0,  -5],[  -1,   1,  -2],[  -1,  -2,  -2]],
        "+2": [[   1,   0,  -5],[  -2,   3,  -1],[   1,   0,  -1]],
        "+3": [[  -1,   1,   2],[   1,  -1,   1],[  -1,  -1,   0]],
        "+4": [[   1,   0,  -1],[   0,  -1,   0],[  -1,  -1,  -1]],
        "+5": [[  -3,   0,   3],[   1,  -1,   2],[   1,   1,   1]],
        "+6": [[  -4,   0,   5],[   2,  -1,   2],[   1,   2,   2]],
        "+7": [[   2,   1,  -2],[   0,   0,  -1],[  -1,  -2,  -1]],
        "+8": [[  -2,  -1,   2],[   0,   0,   1],[   0,   2,   1]],
        "+9": [[   2,   0,  -2],[  -1,   0,  -1],[   0,  -1,  -1]],

        "-1": [[  -5,  -0,   5],[   1,  -1,   2],[   1,   2,   2]],
        "-2": [[  -1,  -0,   5],[   2,  -3,   1],[  -1,  -0,   1]],
        "-3": [[   1,  -1,  -2],[  -1,   1,  -1],[   1,   1,  -0]],
        "-4": [[  -1,  -0,   1],[  -0,   1,  -0],[   1,   1,   1]],
        "-5": [[   3,  -0,  -3],[  -1,   1,  -2],[  -1,  -1,  -1]],
        "-6": [[   4,  -0,  -5],[  -2,   1,  -2],[  -1,  -2,  -2]],
        "-7": [[  -2,  -1,   2],[  -0,  -0,   1],[   1,   2,   1]],
        "-8": [[   2,   1,  -2],[  -0,  -0,  -1],[  -0,  -2,  -1]],
        "-9": [[  -2,  -0,   2],[   1,  -0,   1],[  -0,   1,   1]],
    }[p]

def getPatternCost(pid):
    return sum([sum([abs(p) for p in x]) for x in getPattern(pid)]) * moneyPerUnit

def moveCursorUp(l):
    global posY
    posY -= 1
    if posY < 0:
        posY = 0
    return "moved 1 up"

def moveCursorDown(l):
    global posY, realfield
    posY += 1
    if posY >= realfield.yDim:
        posY = realfield.yDim - 1
    return "moved 1 down"

def moveCursorLeft(l):
    global posX
    posX -= 1
    if posX < 0:
        posX = 0
    return "moved 1 left"

def moveCursorRight(l):
    global posX, realfield
    posX += 1
    if posX >= realfield.xDim:
        posX = realfield.xDim - 1
    return "moved 1 right"

def changeMode(l):
    global tn, currentMode
    print "\r                                               \rSelect fire mode (-: muons, +: tachyons): ",
    sys.stdout.flush()
    m = tn.readChar()
    mult = 1
    if m == "-":
        mult = -1
    else: 
        if m == "+":
            mult = 1
        else:
            return "Invalid fire mode %s" % m

    print "\r                                               \rSelect '%s' fire pattern (1-9): " % m,
    sys.stdout.flush()
    p = tn.readChar()
    if p.isdigit():
        currentMode = "%s%s" % (m, p)
        return "Fire mode and pattern changed to %s%s" % (m, p)
    else:
        return "Invalid fire pattern %s" % p

def fire(l):
    global posX, posY, realfield, currentMode, moneySpent
    realfield.apply(posX, posY, getPattern(currentMode))
    moneySpent += getPatternCost(currentMode)
    return "Fired pattern %s to (%d, %d)" % (currentMode, posX, posY)

def exitGame(l):
    print "\r                                               \rByebye\r\n",
    sys.stdout.flush()
    sys.exit(0)

def setWinCondition(l):
    global cheatEnabled
    if allowCheat:
        cheatEnabled = True
        print "Cheat enabled!"
        time.sleep(1)
    else:
        print "Cheatcodes are disabled. No cheating allowed, you haxx0r ;)"

def unknownCommand(l):
    return "unknown command"

def showScreen():
    global renderedField, lastResult, posX, posY, moneySpent, realfield, currentMode
    print "\x1b[2J"
    print "\x1b[0;0H"
    renderedField.setCursor(posX, posY)
    print renderedField
    print "Current mode: "+currentMode+"     Money spent: %8.2f MM\xc2\xa5      Contaminants remaining: %4d\r\n" % (moneySpent, realfield.sumValues())

    if lastResult != "":
        print "Last result: %s\r\n" % lastResult,
    print "Your move: ",
    sys.stdout.flush()

def checkWinCondition():
    global moneySpent, realfield
    
    if cheatEnabled or realfield.sumValues() == 0:
        print "\x1b[2J"
        print "\x1b[0;0H"
        sys.stdout.flush()
        print "Congratulations! You have cleaned up the world, and only spent %8.2f MM\xc2\xa5" % moneySpent
        print victoryText
        sys.stdout.flush()
        sys.exit(0)

def init(e, v):
    print "\xff\xfd\x22",
    showScreen()
    sys.stdout.flush()

def handleSpecialKey(key):
    global tn
    c = tn.readChar()
    if c == "[":
        c = tn.readChar()
        try:
            handleKeyPress("keypress", ({
                "A": "u",
                "B": "d",
                "C": "r",
                "D": "l",
            }[c],))
        except KeyError:
            handleKeyPress("keypress", (c, ))
    else:
        handleKeyPress("keypress", (c, ))

def handleKeyPress(e, (key, )):
    global lastResult
    try: 
        lastResult = {
            "\x1b": handleSpecialKey,
            "u": moveCursorUp,
            "8": moveCursorUp,
            "d": moveCursorDown,
            "2": moveCursorDown,
            "l": moveCursorLeft,
            "4": moveCursorLeft,
            "r": moveCursorRight,
            "6": moveCursorRight,
            "f": fire,
            " ": fire,
            "5": fire,
            "s": changeMode,
            "c": changeMode,
            "q": exitGame,
            "w": setWinCondition,
        }[key](key)
    except KeyError:
        lastResult = unknownCommand(key)

    checkWinCondition()
    showScreen()

victoryText = open("/opt/playagame/victory.txt").read()
flagText = open("/opt/playagame/flag.txt").read().strip()
flagText = "\n".join(["#" * 60, "#" + " "*58 + "#", "# " + flagText.center(56) + " #", "#" + " "*58 + "#", "#" * 60])
victoryText = victoryText + flagText + "\n\n\n"

if not skipIntro:
    wg = WargamesMenu(0.1, 1.0)

    cont = wg.start()
    if not cont:
        sys.exit(0)

wg = HelpScreen(0.1, 1.0)

cont = wg.start()

if cont:
    tn = TelnetProtocol(sys.stdin)
    tn.registerHandler("init", init) 
    #tn.registerHandler("command", printCommand) 
    tn.registerHandler("keypress", handleKeyPress) 
    tn.registerHandler("closed", sys.exit) 
    tn.start()




