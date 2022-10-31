FROM alpine:latest
RUN apk add vim
RUN adduser -s /usr/bin/vish -D -g shellgames -h /home/shellgames shellgames
COPY key /home/shellgames
COPY vish /usr/bin/vish
RUN chmod 555 /usr/bin/vish
WORKDIR /home/shellgames
ENTRYPOINT exec /usr/bin/vish
