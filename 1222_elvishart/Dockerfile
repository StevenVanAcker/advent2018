FROM ubuntu

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install openbsd-inetd gcc-multilib

RUN mkdir -p /opt
COPY inetd.conf /etc/inetd.conf
COPY src/chal /opt/chal
RUN echo 'this is not the flag' > /opt/flag

WORKDIR /opt
CMD /usr/sbin/inetd -d
EXPOSE 1222
