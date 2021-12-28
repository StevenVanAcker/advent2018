FROM ubuntu

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install python3 vim telnet net-tools

RUN mkdir -p /opt/pwmanager/server
COPY src/ /opt/pwmanager/server

WORKDIR /opt/pwmanager/server
USER nobody
CMD /opt/pwmanager/server/server.py
