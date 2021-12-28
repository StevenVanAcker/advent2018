FROM ubuntu

RUN apt-get update && apt-get -y upgrade

RUN apt-get -y install \
				build-essential pkg-config \
				protobuf-c-compiler libprotobuf-c-dev \
				openssl libssl-dev

# this is just for debugging
### RUN apt-get -y install vim manpages-dev manpages-posix-dev \
### 				man net-tools netcat telnet strace ltrace gdb \
### 				valgrind python curl

### COPY vimrc-DELETE-ME /etc/vim/vimrc.local

RUN mkdir -p /opt/udpsanta
COPY launcher.sh /opt/udpsanta/launcher.sh
COPY src/ /opt/udpsanta/src
COPY userdata/ /opt/udpsanta/userdata

RUN cd /opt/udpsanta/userdata && make clean 
RUN cd /opt/udpsanta/src && make clean all && cp server client udpsanta.proto /opt/udpsanta && cd / && rm -rf /opt/udpsanta/src

WORKDIR /opt/udpsanta

CMD /opt/udpsanta/launcher.sh
