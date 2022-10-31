FROM alpine:latest
#CMD ["/bin/sh"]
RUN adduser -D -g shellgames -h /home/shellgames shellgames
COPY README /home/shellgames
COPY key /home/shellgames
WORKDIR /home/shellgames
ENTRYPOINT exec /bin/sh
