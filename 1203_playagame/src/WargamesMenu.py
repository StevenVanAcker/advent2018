#!/usr/bin/python

import sys, time
from TelnetProtocol import TelnetProtocol

class WargamesMenu:
    games = [   
                "TIC-TAC-TOE",
                "BLACK JACK",
                "GIN RUMMY",
                "HEARTS",
                "BRIDGE",
                "CHECKERS",
                "CHESS",
                "POKER",
                "FIGHTER COMBAT",
                "GUERRILLA ENGAGEMENT",
                "DESERT WARFARE",
                "AIR-TO-GROUND ACTIONS",
                "THEATERWIDE TACTICAL WARFARE",
                "THEATERWIDE BIOTOXIC AND CHEMICAL WARFARE",
        ]

    def __init__(self, td = 0.1, wd = 1.0):
        self.typeDelay = td
        self.waitDelay = wd
        self.textBuffer = ""
        self.telnetproto = None
        self.returnCode = False

    def putChar(self, x,y,c):
        sys.stdout.write("\x1b[%d;%dH%s" % (y,x,c))
        sys.stdout.flush()

    def typeText(self, startX, startY, text):
        for i in range(len(text)):
            self.putChar(i + startX, startY, text[i])
            time.sleep(self.typeDelay)


    def showScreen(self):
        # clear screen
        print "\x1b[2J",
        print "\x1b[34;1m",

        cursorX = len("SHALL WE PLAY A GAME? ") + 1
        cursorY = 4

        self.typeText(1,2,"GREETINGS PROFESSOR FALKEN.")
        time.sleep(self.waitDelay)

        self.typeText(1,4,"SHALL WE PLAY A GAME?")
        time.sleep(self.waitDelay)

        startX = 5
        startY = 6
        data = self.games

        maxloop = 2 * len(data) + max([len(x) for x in data])

        for i in range(maxloop):
            for l in range(len(data)):
                x = i - 2 * l
                y = l
                if x >= 0 and x < len(data[l]):
                    c = data[l][x]
                    self.putChar(x + startX, y + startY, c)
            time.sleep(self.typeDelay)

        time.sleep(self.waitDelay)
        self.typeText(5,startY + 1 + len(self.games),"GLOBAL THERMONUCLEAR WAR")
        time.sleep(self.waitDelay)
        self.typeText(5 + len("GLOBAL THERMONUCLEAR WAR") ,startY + 1 + len(self.games),", CLEANUP EDITION")

        self.putChar(cursorX, cursorY, "")
        print "\x1b[0m",

    def init(self, e, v):
        print "\xff\xfd\x22",
        self.showScreen()
        sys.stdout.flush()


    def handleKeyPress(self, e, (key,)):
        if key == "\n":
            print "\x1b[0J",
            print "\r\n",
            
            if self.textBuffer.upper() == "NO":
                print "\x1b[34;1m",
                self.typeText(1,5,"SAD TO HEAR IT, PROFESSOR FALKEN.")
                print "\x1b[0m",
                self.textBuffer = ""
                self.returnCode = False
                self.telnetproto.stop()
                return

            if self.textBuffer.upper() == "YES":
                print "\x1b[34;1m",
                self.typeText(1,5,"PLEASE INDICATE WHICH GAME, PROFESSOR FALKEN.")
                print "\x1b[0m",
                self.textBuffer = ""
                time.sleep(self.waitDelay)
                self.showScreen()
                return
                
            if self.textBuffer.upper() == "GLOBAL THERMONUCLEAR WAR, CLEANUP EDITION":
                print "\x1b[34;1m",
                self.typeText(1,5,"VERY WELL, PROFESSOR FALKEN.")
                print "\x1b[0m",
                self.textBuffer = ""
                self.returnCode = True
                self.telnetproto.stop()
                return

            if self.textBuffer.upper() == "" or not self.textBuffer.upper() in self.games:
                print "\x1b[34;1m",
                self.typeText(1,5,"I DO NOT KNOW THIS GAME.")
                print "\x1b[0m",
                self.textBuffer = ""
                time.sleep(self.waitDelay)
                self.showScreen()
                return

            print "\x1b[34;1m",
            self.typeText(1,5,"THIS GAME IS NOT CURRENTLY AVAILABLE.")
            print "\x1b[0m",
            self.textBuffer = ""
            time.sleep(self.waitDelay)
            self.showScreen()
            return

        else:
            self.textBuffer += key
    

    def start(self):
        self.telnetproto = TelnetProtocol(sys.stdin)
        self.telnetproto.registerHandler("init", "init", self) 
        self.telnetproto.registerHandler("keypress", "handleKeyPress", self) 
        self.telnetproto.registerHandler("closed", sys.exit) 
        self.telnetproto.start()
        return self.returnCode

if __name__ == '__main__':
    wg = WargamesMenu()
    if wg.start():
        print "Continuing..."
    else:
        print "Done"

