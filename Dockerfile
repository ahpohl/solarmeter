FROM alpine:3.14

RUN apk add git mosquitto-dev build-base

WORKDIR /app

ADD https://api.github.com/repos/ahpohl/libabbaurora/git/refs/heads/master libaurora-version.json
RUN git clone https://github.com/ahpohl/libabbaurora.git 
RUN cd /app/libabbaurora && sed -i 's/ln\s-sr\(.*\)/ln -s \1/g' Makefile && PREFIX=/usr make install
ADD https://api.github.com/repos/ahpohl/solarmeter/git/refs/heads/master solarmeter-version.json
RUN git clone https://github.com/ahpohl/solarmeter.git && cp /app/libabbaurora/include/*.h /app/solarmeter/include && cd /app/solarmeter && make install

ENTRYPOINT ["/usr/local/bin/solarmeter"]
