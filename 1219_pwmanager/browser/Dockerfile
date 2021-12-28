FROM ubuntu

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install python3 vim curl libfontconfig parallel

RUN mkdir -p /opt/tmp && cd /opt/tmp && curl -L --output phantomjs.tar.bz2 https://bitbucket.org/ariya/phantomjs/downloads/phantomjs-2.1.1-linux-x86_64.tar.bz2 && tar -xf phantomjs.tar.bz2 && cp */bin/phantomjs /usr/bin/ && cd / && rm -rf /opt/tmp

RUN mkdir -p /opt/pwmanager/browser
COPY src/ /opt/pwmanager/browser

WORKDIR /opt/pwmanager/browser
USER nobody

CMD /opt/pwmanager/browser/launch_browsers.sh /opt/pwmanager/browser/ads/ 10

