FROM alpine:latest

RUN apk add git mosquitto-dev build-base

WORKDIR /app

ADD https://api.github.com/repos/ahpohl/libabbaurora/git/refs/heads/master libabbaurora-version.json
RUN git clone https://github.com/ahpohl/libabbaurora.git
RUN cd /app/libabbaurora && sed -i 's/ln\s-sr\(.*\)/ln -s \1/g' Makefile && make PREFIX=/usr install

ADD https://api.github.com/repos/ahpohl/solarmeter/git/refs/heads/master solarmeter-version.json
#RUN git clone https://github.com/ahpohl/solarmeter.git 
COPY . /app/solarmeter
RUN cd /app/solarmeter && make PREFIX=/usr install

ENTRYPOINT solarmeter --config /etc/solarmeter.conf
