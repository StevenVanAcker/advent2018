#!/usr/bin/python

import sys, time
from TelnetProtocol import TelnetProtocol

class HelpScreen:
    def __init__(self, td = 0.1, wd = 1.0):
        self.typeDelay = td
        self.waitDelay = wd
        self.textBuffer = ""
        self.telnetproto = None
        self.returnCode = False

    def showScreen(self):
        # clear screen
        print "\x1b[2J",
        print "\x1b[34;1m",

        print '''
In 2025, the world has been devastated by a nuclear attack and is a nuclear
wasteland. Luckily, the Chinese government has foreseen this disaster and
launched a satellite in orbit that is able to counter the nuclear waste.

Research has shown that nuclear waste is primarily composed of inverted tachyon
and positive lepton radiation, which cancel each other out.

The Chinese satellite uses inverted tachyons (-) and positive leptons (+) that
can each be fired in 9 different patterns over an area of 3 by 3 kilometers.

Your job is to clean up the nuclear waste by positioning the satellite,
selecting a firing mode (- or +) and a firing pattern (1 through 9), and then
firing.  The result will be instantaneous, the remaining contaminants per
square kilometer will be indicated on the screen.

Please note that firing the satellite costs money, which the post-apocalyptic
world is in short supply of: don't waste money! Good luck!

Keys:
        arrow keys: position satellite
        s or c: select firing mode and pattern
        f: fire satellite
        q: quit and let the world go to waste


Press enter to continue.
'''

    def init(self, e, v):
        print "\xff\xfd\x22",
        self.showScreen()
        sys.stdout.flush()


    def handleKeyPress(self, e, (key,)):
        self.textBuffer = ""
        self.returnCode = True
        self.telnetproto.stop()
    

    def start(self):
        self.telnetproto = TelnetProtocol(sys.stdin)
        self.telnetproto.registerHandler("init", "init", self) 
        self.telnetproto.registerHandler("keypress", "handleKeyPress", self) 
        self.telnetproto.registerHandler("closed", sys.exit) 
        self.telnetproto.start()
        return self.returnCode

if __name__ == '__main__':
    wg = HelpScreen()
    if wg.start():
        print "Continuing..."
    else:
        print "Done"

