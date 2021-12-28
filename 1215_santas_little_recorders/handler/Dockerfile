FROM ubuntu

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install openbsd-inetd python3 python3-pil python3-eventlet python3-requests

COPY handler.py /opt/
COPY record_description /opt/
COPY inetd.conf /etc/inetd.conf
CMD /usr/sbin/inetd -d

EXPOSE 5555
