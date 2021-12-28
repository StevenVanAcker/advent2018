#!/usr/bin/python

import sys

TELNET_IAC  = chr(255)

TELNET_WILL = chr(251)
TELNET_WONT = chr(252)
TELNET_DO   = chr(253)
TELNET_DONT = chr(254)

TELNET_ENDSUBNEGOTIATE   = chr(240)
TELNET_NOP               = chr(241)
TELNET_DATAMARK          = chr(242)
TELNET_BREAK             = chr(243)
TELNET_INTERRUPT         = chr(244)
TELNET_ABORT             = chr(245)
TELNET_YOUTHERE          = chr(246)
TELNET_ERASECHAR         = chr(247)
TELNET_ERASELINE         = chr(248)
TELNET_GOAHEAD           = chr(249)
TELNET_SUBNEGOTIATE      = chr(250)
 
def log(m):
    pass
    #f = open("/tmp/telnet.log", "a")
    #f.write(m)
    #f.close()

class TelnetProtocol(object):
    def __init__(self, fd):
        self.fd = fd
        self.handlers = {}
        self.keepRunning = True

    def nameFromChar(self, cmd):
        try:
            return {
                TELNET_IAC: "IAC",
                TELNET_WILL: "WILL",
                TELNET_WONT: "WONT",
                TELNET_DO: "DO",
                TELNET_DONT: "DONT",
                TELNET_ENDSUBNEGOTIATE: "ENDSUBNEGOTIATE",
                TELNET_NOP: "NOP",
                TELNET_DATAMARK: "DATAMARK",
                TELNET_BREAK: "BREAK",
                TELNET_INTERRUPT: "INTERRUPT",
                TELNET_ABORT: "ABORT",
                TELNET_YOUTHERE: "YOUTHERE",
                TELNET_ERASECHAR: "ERASECHAR",
                TELNET_ERASELINE: "ERASELINE",
                TELNET_GOAHEAD: "GOAHEAD",
                TELNET_SUBNEGOTIATE: "SUBNEGOTIATE",
            }[cmd]
        except KeyError:
            return "DATA"

    def readChar(self):
        char = self.fd.read(1)
        if char == "":
            self.callHandler("closed")
            return None
        return char

    def registerHandler(self, event, handler, obj = None):
        if not event in self.handlers:
            self.handlers[event] = []

        self.handlers[event].append((handler, obj))
        log("I now have %d handlers for %s\n" % (len(self.handlers[event]), event))

    def callHandler(self, event, *args):
        if not event in self.handlers:
            return
        for (handler, obj) in self.handlers[event]:
            if obj != None:
                handler = getattr(obj, handler)
                handler(event, args)
            else:
                handler(event, args)

    def handleCommand(self):
        cmd = self.readChar()
        data = []
        #log("Received IAC + %s\n" % (self.nameFromChar(cmd)))
        if cmd == TELNET_SUBNEGOTIATE:
            #log("Reading until end of subneg\n")
            opt = ""
            while opt != TELNET_ENDSUBNEGOTIATE:
                opt = self.readChar()
                if opt != TELNET_ENDSUBNEGOTIATE:
                    data.append(opt)
            #log("Done with subneg\n")
        else:
            opt = self.readChar()
            data.append(opt)
            #log("... Received IAC + %s + byte %d\n" % (self.nameFromChar(cmd), ord(opt)))
        self.callHandler("command", cmd, data)

    def stop(self):
        log("Stop called!!!\n")
        self.keepRunning = False

    def start(self):
        self.callHandler("init")
        while self.keepRunning:
            char = self.readChar()
            if char == TELNET_IAC:
                self.handleCommand()
            else:
                if char == "\r":
                    self.readChar() #skip \0
                    self.callHandler("keypress", "\n")
                else:
                    self.callHandler("keypress", char)

        self.callHandler("stopped")
        










if __name__ == '__main__':
    def printCommand(e,(cmd, data)):
        global tn
        log("Got command(%s)\n" % tn.nameFromChar(cmd))
    def printChar(e,x):
        log("Got char(%s)\n" % ",".join(["%d" % ord(a) for a in x]))
    def connClosed(e, v):
        log("Connection closed\n")
        sys.exit(0)
    def init(e, v):
        log("Program started\n")

    print "\xff\xfd\x22",
    sys.stdout.flush()

    tn = TelnetProtocol(sys.stdin)
    tn.registerHandler("init", init) 
    tn.registerHandler("command", printCommand) 
    tn.registerHandler("keypress", printChar) 
    tn.registerHandler("closed", connClosed) 
    tn.start()


