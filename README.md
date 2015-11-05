# LiXS: Lightweight XenStore

LiXS is a C++ implementation of Xen's xenstore server. It focus on performance
and efficiency while keeping compatibility with the upstream xenstore protocol.


# Motivation

The major goal of LiXS is to provide an efficient and extremely scalable
implementation of a xenstore daemon. It scales well to support up to thousands
of concurrently running domains while still consuming a low amount of
resources. It is therefore suitable to resource constrained devices like recent
single board computers.

LiXS was designed to be extensible and configurable. While initialy focusing on
creating an upstream xenstore replacement it is intended to be easily
extensible in two major aspects:

* Protocol: it should be fairly easy to design and integrate a new version of
the xenstore protocol into LiXS.

* Communication channels: it should be straight forward to support new
mechanisms to communicate with the xenstore like TCP sockets, shared memory,
etc.


# Install

Currently LiXS is only available from source so you need to build it first. The
process is fairly straightforward though.

## Dependencies

To build LiXS you need:

* A modern C++ compiler. Tested and works with `clang-3.6` and `gcc-4.8`, but
should be possible to use older versions as far as C++11 is supported.
* Xen and xenstore development headers.

LiXS doesn't really depend on any "non-standard" libraries, besides Xen libs of
course. You also need xenstore utilities from upstream to interact with it given LiXS
currently doesn't provide any replacement.

## Build from source

Building LiXS is fairly straight forward, you basically need:

`make`

There is a small quirk if you're building for Xen 4.4. On this Xen version the header
`xenctrl.h` uses the GNU C extensions "Incomplete enum Types" that is not supported on
C++. Therefore you need to patch this file before building. To do so:

`patch -p1 <path_to_xenctrl.h> < extra/xen-4.4-xenctrl.h.patch`

The patch basically changes the location of a typedef to appear after the respective enum
declaration.

## Instalation and configuration

LiXS is comprised of a single binary. To install:

`sudo make install`

This will basically install `app/lixs` under `/usr/local/sbin/lixs`. Finally you need to
configure Xen to use this xenstore. For that you must add the following lines to
`/etc/default/xencommons`:

```
XENSTORED=/usr/local/sbin/lixs
XENSTORED_ARGS="-D --xenbus --unix-socket --virq-dom-exc"
```

# Usage

Under **Instal** above you basically setup lixs as an upstream xenstore replacement. To
configure advance options please read `lixs --help`.
