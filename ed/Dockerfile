FROM alpine:latest
RUN adduser -s /bin/ed -D -g shellgames -h /home/shellgames shellgames
COPY README /home/shellgames
COPY key /home/shellgames
WORKDIR /home/shellgames
ENV SHELL=/bin/ed
ENTRYPOINT ulimit -u 2 && exec /bin/ed
