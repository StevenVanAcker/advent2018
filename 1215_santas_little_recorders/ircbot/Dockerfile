FROM ubuntu:latest

RUN apt-get update && apt-get upgrade -y
RUN apt-get --yes install python-twisted supervisor

COPY GenericIRCBot.py /opt
COPY ircbot.py /opt
COPY supervisor.conf /etc/supervisor/conf.d/ircbot.conf


CMD /usr/bin/supervisord -nc /etc/supervisor/supervisord.conf 
