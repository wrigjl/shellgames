# Hackers Challenge 2022 Shellgames

The idea of this challenge is that you're given an ssh key
and a hostname (e.g. `shellgames-1.youcanhack.me`). When you
ssh in, you're in a shell and have to get the contents of a
file called key.

I had several questions about how the VMs were configured and
I thought I would capture those thoughts here.

## Basic Configuration

Each vm was configured to have a user named `shellgames` with
a shell called: `/bin/dosh`. You can find the source for `dosh`
here [dosh.cc](dosh.cc).

## dosh

The primary job of `dosh` is to log what you type and what the shell
returns to a temporary file. Otherwise, it justs acts as a bump in the
wire between your ssh session and your actual shell: `shell2docker`.

`dosh` also pushes the results to s3 and limits the total amount of
time and data you can use (2 minutes and 2MB, respectively). These values
are hard coded, but easily edited.

## shell2docker

`shell2docker` is run by `dosh` (above). The `shellgames` user has to
have `sudo` access to run docker, and in most cases I used docker's
`--cap_drop=all`, but for some containers I needed the ability to
call `chroot` and such. You'll find `shell2docker` in each directory.

## Docker containers

The real meat of this challenge is the docker image. Imagine this
pipeline: You ssh into my machine, sshd calls `dosh` which calls
`shell2docker` which, ultimately does `docker run --rm -it shell`. So,
you're in a docker container with (hopefully) nowhere to go. docker's
`--net=none` flag is used to prevent network access (other than to
localhost).

There are containers here for:

 - sh
 - m4
 - vim
 - cc
 - lolcode
 - perl
 - awk
 - ed
