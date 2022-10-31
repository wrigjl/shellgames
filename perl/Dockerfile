FROM alpine:latest
COPY key /home/shellgames/key
RUN apk add perl
RUN adduser -D -g shellgames -h /home/shellgames shellgames
WORKDIR /home/shellgames
ENTRYPOINT perl --version && ulimit -u 2 && perl
