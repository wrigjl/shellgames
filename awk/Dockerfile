FROM alpine:latest
RUN apk add gawk
RUN adduser -D -g shellgames -h /home/shellgames shellgames
RUN install -d /chroot/bin /chroot/lib
RUN cp /usr/bin/gawk /chroot/bin
RUN install -c /lib/ld-musl-x86_64.so.1 /chroot/lib             
RUN ln -s ld-musl-x86_64.so.1 /chroot/lib/libc.musl-x86_64.so.1
COPY key /chroot
WORKDIR /home/shellgames
ENTRYPOINT gawk --version && exec chroot /chroot /bin/gawk -f -
