# abis/roxy — Roxy's own ABI definitions

Every header in this directory is the definition that
`sysdeps/roxy/include/abi-bits/<name>.h` links to. Roxy owns these definitions: a layout,
constant, or type width that Roxy wants different from Linux is changed here, never in
`abis/linux/`, which the other sysdeps share.

## Contents and provenance

These 50 headers were transferred verbatim from `abis/linux/` and are byte-identical to it as of
the transfer. They are Roxy-owned copies rather than links so that Roxy can change any of them
without affecting the other sysdeps that share `abis/linux/`.

Headers present in `abis/linux/` but unused by Roxy (epoll, inotify, ptrace, reboot, ...) are not
copied. Add one when the option that needs it is enabled.

## Divergence and drift checking

Only this directory may diverge. `abis/linux/` stays as upstream wrote it, because several other
sysdeps share it.

List the intentional divergences:

	diff -rq abis/linux abis/roxy        # README.md shows up as an expected extra

Run that after every upstream sync. Upstream changes to `abis/linux/` — including layout fixes —
no longer reach Roxy automatically, so both the existing differences and the incoming changes
have to be judged by hand, one header at a time.

A header that diverges carries a comment at the top of the file naming the reason for the
divergence and, where one exists, the kernel-side record it has to match.

An ABI-owned constant that a generic header also defines is made to win differently: the generic
definition becomes a guarded fallback (`#ifndef`), so a sysdep that owns the value defines it first
and every other sysdep keeps the fallback. `options/posix/include/termios.h` and
`options/glibc/include/sys/ioctl.h` carry the guards for the ioctl numbers Roxy defines in
`ioctls.h`; those two edits do not change behavior for any other sysdep, whose values match the
fallbacks.

## Kernel contract

A structure that crosses the syscall boundary is only half-defined here; its other half is a
`#[repr(C)]` record with size and offset assertions in `kernel/syscall/src/`, and the two change
together. Structures that mlibc translates field by field do not cross the boundary at all: for
example `struct stat` is filled from the kernel's `StatAbi` in `sysdeps/roxy/sysdeps.cpp`, so its
layout constrains userspace only.
