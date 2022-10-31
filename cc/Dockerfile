FROM alpine:latest
RUN adduser -D -g shellgames -h /home/shellgames shellgames
RUN apk add gcc musl-dev libc-dev
RUN install -d /chroot/bin /chroot/lib
RUN install -c /lib/ld-musl-x86_64.so.1 /chroot/lib
RUN ln -s ld-musl-x86_64.so.1 /chroot/lib/libc.musl-x86_64.so.1
COPY keyfake /home/shellgames/key
WORKDIR /home/shellgames
ENTRYPOINT gcc --version && gcc -x c - && mv a.out /chroot/bin && echo 'ctf{sisi-6aa868c32befab51}' > /chroot/key && chroot /chroot /bin/a.out
