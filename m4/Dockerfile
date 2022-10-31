FROM alpine:latest
RUN adduser -s /usr/bin/m4 -D -g shellgames -h /home/shellgames shellgames
RUN apk add m4
COPY key /home/shellgames
COPY m4sh /usr/bin/m4sh
RUN chmod 555 /usr/bin/m4sh
WORKDIR /home/shellgames
ENTRYPOINT exec /usr/bin/m4sh
