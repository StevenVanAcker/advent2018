#!/usr/bin/python

from twisted.internet import reactor, protocol
from GenericIRCBot import GenericIRCBot, GenericIRCBotFactory
from urllib import unquote
import socket

HANDLERPORT = 5555
HANDLERHOST = "handler"

class IRCBot(GenericIRCBot):
    def __init__(self):
	self.commandData = {
	    "!record": { 
	    	"fn": self.handle_RECORD, 
		"argc": 1, 
		"tillEnd": True,
		"help": ""
	    },
	}

	self.commands = {
	    # only in direct user message, first word is the command
	    "private": ["!record"],
	    # only in channels, first word must be the command
	    "public": [],
	    # only in channels, first word is the name of this bot followed by a colon, second word is the command
	    "directed": [],
	}

    def handle_RECORD(self, msgtype, user, recip, cmd, msg): #{{{
        if msg.startswith("https://"):
            self.sendMessage(msgtype, user, recip, "Sorry, no https allowed because it causes the python requests lib to go into infinite recursion :(")
            return

        if not ( msg.startswith("http://") and msg.endswith(".jpg")):
            self.sendMessage(msgtype, user, recip, "This does not look like the URL of a JPEG image")
            return

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HANDLERHOST, HANDLERPORT))
        s.sendall(msg + "\n")
        data = s.recv(1024).strip()
        s.close()
	self.sendMessage(msgtype, user, recip, data)
#}}}

if __name__ == '__main__':
    # create factory protocol and application
    f = GenericIRCBotFactory(IRCBot, ["#santas_little_recorders"], "SantaRecorder", "Santa's Little Recorder", "the north pole")

    # connect factory to this host and port
    reactor.connectTCP("roxanne.overthewire.org", 6667, f)

    # run bot
    reactor.run()


