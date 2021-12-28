FROM ubuntu

RUN apt-get update && apt-get -y upgrade

RUN apt-get -y install python2.7 openbsd-inetd
RUN update-alternatives --install /usr/bin/python python /usr/bin/python2.7 1

RUN mkdir -p /opt/playagame
COPY src/*.py /opt/playagame/
COPY src/victory.txt /opt/playagame/victory.txt
COPY src/inetd.playagame /etc/inetd.conf


COPY flag /opt/playagame/flag.txt

WORKDIR /opt/playagame

CMD /usr/sbin/inetd -d


EXPOSE 1203
