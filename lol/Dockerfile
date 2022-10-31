FROM alpine:latest
RUN adduser -D -g shellgames -h /home/shellgames shellgames
RUN apk add git cmake build-base readline-dev
WORKDIR /home/shellgames
COPY key /home/shellgames/key
RUN git clone https://github.com/justinmeza/lci.git
RUN cd lci && git checkout future
RUN cd lci && cmake .
RUN cd lci && make
ENTRYPOINT lci/lci --help && lci/lci -
