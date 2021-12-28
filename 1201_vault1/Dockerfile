FROM ubuntu

RUN apt-get update && apt-get -y upgrade

RUN apt-get -y install python2.7 openbsd-inetd
RUN update-alternatives --install /usr/bin/python python /usr/bin/python2.7 1

RUN mkdir -p /opt/vault1
COPY src/*.py /opt/vault1/
COPY src/victory.txt /opt/vault1/
COPY src/inetd.vault1 /etc/inetd.conf

WORKDIR /opt/vault1

CMD /usr/sbin/inetd -d

EXPOSE 1201
