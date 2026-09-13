#ifndef _ABIBITS_SOCKET_H
#define _ABIBITS_SOCKET_H

#include <abi-bits/sa_family_t.h>
#include <abi-bits/socklen_t.h>
#include <abi-bits/sockaddr_storage.h>
#include <bits/size_t.h>
#include <bits/ssize_t.h>
#include <bits/posix/iovec.h>

#ifdef __cplusplus
extern "C" {
#endif

struct msghdr {
	void *msg_name;
	socklen_t msg_namelen;
	struct iovec *msg_iov;
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	int __pad0;
#endif
	int msg_iovlen;
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	int __pad0;
#endif
	void *msg_control;
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	int __pad1;
#endif
	socklen_t msg_controllen;
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	int __pad1;
#endif
	int msg_flags;
};

#if defined(_GNU_SOURCE)
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned int  msg_len;
};
#endif

struct cmsghdr {
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	int __pad;
#endif
	socklen_t cmsg_len;
#if __INTPTR_WIDTH__ == 64 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	int __pad;
#endif
	int cmsg_level;
	int cmsg_type;
};

#ifdef __cplusplus
}
#endif

/*
 * Roxy's socket constants.
 *
 * Each word is numbered from a base above that word's whole Linux range, so a value below the base
 * is another personality's numbering: the kernel reports such a caller as foreign instead of
 * reading it as a request of its own. The one exception is a `ROXY_*_UNSUPPORTED` marker, which
 * every member Roxy defines but cannot serve is given: naming one still compiles, and passing one
 * reaches the kernel as that marker, which it reports as unsupported rather than as a foreign
 * number. Collapsing them keeps ported sources compiling without pretending the option exists.
 *
 * The kernel side is `kernel/syscall/src/syscalls/socket/`.
 */

/* Linux's families stop at `PF_MAX` 46, so 0x80 is not one of them. `AF_INET` and `AF_INET6` keep
 * values of their own because an upstream `switch` names them as cases. */
#define ROXY_AF_BASE 0x100
#define ROXY_AF_UNSUPPORTED 0x80

/* Linux's type word uses bits 0-3, 11 (`SOCK_NONBLOCK`), and 19 (`SOCK_CLOEXEC`), so 1 << 12 is
 * free. The type occupies three bits from the base, with the two supported flags above them, and
 * `SOCK_DGRAM` takes a value of its own for the same reason `AF_INET` does. */
#define ROXY_SOCK_BASE (1 << 20)
#define ROXY_SOCK_TYPE_MASK ((ROXY_SOCK_BASE << 3) - ROXY_SOCK_BASE)
#define ROXY_SOCK_UNSUPPORTED (1 << 12)

/* Linux's `SO_*` stop at 83, so 0x80 is not one of them. */
#define ROXY_SO_BASE 0x100
#define ROXY_SO_UNSUPPORTED 0x80

/* Linux's levels are its `IPPROTO_*` values, whose maximum is 263. */
#define ROXY_SOL_BASE (1 << 10)
#define ROXY_SOL_UNSUPPORTED (1 << 9)

/* Linux's message flags fill bits 0-15 and 26 (`MSG_ZEROCOPY`), so 1 << 16 is free. */
#define ROXY_MSG_BASE (1 << 27)
#define ROXY_MSG_UNSUPPORTED (1 << 16)

/* Linux's `SCM_*` are 1 and 2, so 0x80 is not one of them. */
#define ROXY_SCM_UNSUPPORTED 0x80

/* Ancillary data passing is not implemented, so every `SCM_*` is the marker. */
#define SCM_RIGHTS ROXY_SCM_UNSUPPORTED

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#define SCM_CREDENTIALS ROXY_SCM_UNSUPPORTED
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#define ROXY_SHUT_BASE 0x100
#define SHUT_RD ROXY_SHUT_BASE
#define SHUT_WR (ROXY_SHUT_BASE << 1)
#define SHUT_RDWR (ROXY_SHUT_BASE << 2)

#ifndef SOCK_STREAM
#define SOCK_STREAM ROXY_SOCK_BASE
#define SOCK_DGRAM (ROXY_SOCK_BASE + 1)
#endif

#define SOCK_RAW ROXY_SOCK_UNSUPPORTED
#define SOCK_SEQPACKET ROXY_SOCK_UNSUPPORTED

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#define SOCK_RDM ROXY_SOCK_UNSUPPORTED
#define SOCK_DCCP ROXY_SOCK_UNSUPPORTED
#define SOCK_PACKET ROXY_SOCK_UNSUPPORTED
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC (ROXY_SOCK_BASE << 4)
#define SOCK_NONBLOCK (ROXY_SOCK_BASE << 5)
#endif

#define AF_UNSPEC 0
#define AF_UNIX ROXY_AF_BASE
#define AF_INET (ROXY_AF_BASE + 1)
#define AF_INET6 (ROXY_AF_BASE + 2)

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#define PF_UNSPEC 0
#define PF_LOCAL ROXY_AF_BASE
#define PF_UNIX         PF_LOCAL
#define PF_FILE         PF_LOCAL
#define PF_INET (ROXY_AF_BASE + 1)
#define PF_AX25 ROXY_AF_UNSUPPORTED
#define PF_IPX ROXY_AF_UNSUPPORTED
#define PF_APPLETALK ROXY_AF_UNSUPPORTED
#define PF_NETROM ROXY_AF_UNSUPPORTED
#define PF_BRIDGE ROXY_AF_UNSUPPORTED
#define PF_ATMPVC ROXY_AF_UNSUPPORTED
#define PF_X25 ROXY_AF_UNSUPPORTED
#define PF_INET6 (ROXY_AF_BASE + 2)
#define PF_ROSE ROXY_AF_UNSUPPORTED
#define PF_DECnet       12
#define PF_NETBEUI ROXY_AF_UNSUPPORTED
#define PF_SECURITY ROXY_AF_UNSUPPORTED
#define PF_KEY ROXY_AF_UNSUPPORTED
#define PF_NETLINK ROXY_AF_UNSUPPORTED
#define PF_ROUTE        PF_NETLINK
#define PF_PACKET ROXY_AF_UNSUPPORTED
#define PF_ASH ROXY_AF_UNSUPPORTED
#define PF_ECONET ROXY_AF_UNSUPPORTED
#define PF_ATMSVC ROXY_AF_UNSUPPORTED
#define PF_RDS ROXY_AF_UNSUPPORTED
#define PF_SNA ROXY_AF_UNSUPPORTED
#define PF_IRDA ROXY_AF_UNSUPPORTED
#define PF_PPPOX ROXY_AF_UNSUPPORTED
#define PF_WANPIPE ROXY_AF_UNSUPPORTED
#define PF_LLC ROXY_AF_UNSUPPORTED
#define PF_IB ROXY_AF_UNSUPPORTED
#define PF_MPLS ROXY_AF_UNSUPPORTED
#define PF_CAN ROXY_AF_UNSUPPORTED
#define PF_TIPC ROXY_AF_UNSUPPORTED
#define PF_BLUETOOTH ROXY_AF_UNSUPPORTED
#define PF_IUCV ROXY_AF_UNSUPPORTED
#define PF_RXRPC ROXY_AF_UNSUPPORTED
#define PF_ISDN ROXY_AF_UNSUPPORTED
#define PF_PHONET ROXY_AF_UNSUPPORTED
#define PF_IEEE802154 ROXY_AF_UNSUPPORTED
#define PF_CAIF ROXY_AF_UNSUPPORTED
#define PF_ALG ROXY_AF_UNSUPPORTED
#define PF_NFC ROXY_AF_UNSUPPORTED
#define PF_VSOCK ROXY_AF_UNSUPPORTED
#define PF_KCM ROXY_AF_UNSUPPORTED
#define PF_QIPCRTR ROXY_AF_UNSUPPORTED
#define PF_SMC ROXY_AF_UNSUPPORTED
#define PF_XDP ROXY_AF_UNSUPPORTED
#define PF_MAX ROXY_AF_UNSUPPORTED

#define AF_LOCAL        PF_LOCAL
#define AF_FILE         AF_LOCAL
#define AF_AX25         PF_AX25
#define AF_IPX          PF_IPX
#define AF_APPLETALK    PF_APPLETALK
#define AF_NETROM       PF_NETROM
#define AF_BRIDGE       PF_BRIDGE
#define AF_ATMPVC       PF_ATMPVC
#define AF_X25          PF_X25
#define AF_ROSE         PF_ROSE
#define AF_DECnet       PF_DECnet
#define AF_NETBEUI      PF_NETBEUI
#define AF_SECURITY     PF_SECURITY
#define AF_KEY          PF_KEY
#define AF_NETLINK      PF_NETLINK
#define AF_ROUTE        PF_ROUTE
#define AF_PACKET       PF_PACKET
#define AF_ASH          PF_ASH
#define AF_ECONET       PF_ECONET
#define AF_ATMSVC       PF_ATMSVC
#define AF_RDS          PF_RDS
#define AF_SNA          PF_SNA
#define AF_IRDA         PF_IRDA
#define AF_PPPOX        PF_PPPOX
#define AF_WANPIPE      PF_WANPIPE
#define AF_LLC          PF_LLC
#define AF_IB           PF_IB
#define AF_MPLS         PF_MPLS
#define AF_CAN          PF_CAN
#define AF_TIPC         PF_TIPC
#define AF_BLUETOOTH    PF_BLUETOOTH
#define AF_IUCV         PF_IUCV
#define AF_RXRPC        PF_RXRPC
#define AF_ISDN         PF_ISDN
#define AF_PHONET       PF_PHONET
#define AF_IEEE802154   PF_IEEE802154
#define AF_CAIF         PF_CAIF
#define AF_ALG          PF_ALG
#define AF_NFC          PF_NFC
#define AF_VSOCK        PF_VSOCK
#define AF_KCM          PF_KCM
#define AF_QIPCRTR      PF_QIPCRTR
#define AF_SMC          PF_SMC
#define AF_XDP          PF_XDP
#define AF_MAX          PF_MAX
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#define SO_DEBUG ROXY_SO_UNSUPPORTED
#define SO_REUSEADDR ROXY_SO_UNSUPPORTED
#define SO_TYPE ROXY_SO_BASE
#define SO_ERROR (ROXY_SO_BASE << 1)
#define SO_DONTROUTE ROXY_SO_UNSUPPORTED
#define SO_BROADCAST ROXY_SO_UNSUPPORTED
#define SO_SNDBUF ROXY_SO_UNSUPPORTED
#define SO_RCVBUF ROXY_SO_UNSUPPORTED
#define SO_KEEPALIVE ROXY_SO_UNSUPPORTED
#define SO_OOBINLINE ROXY_SO_UNSUPPORTED
#define SO_LINGER ROXY_SO_UNSUPPORTED
#define SO_RCVLOWAT ROXY_SO_UNSUPPORTED
#define SO_SNDLOWAT ROXY_SO_UNSUPPORTED
#define SO_ACCEPTCONN ROXY_SO_UNSUPPORTED
#define SO_PROTOCOL ROXY_SO_UNSUPPORTED
#define SO_DOMAIN ROXY_SO_UNSUPPORTED

#ifndef SO_RCVTIMEO
#if __LONG_MAX__ == 0x7fffffff
#define SO_RCVTIMEO ROXY_SO_UNSUPPORTED
#else
#define SO_RCVTIMEO ROXY_SO_UNSUPPORTED
#endif
#endif

#ifndef SO_SNDTIMEO
#if __LONG_MAX__ == 0x7fffffff
#define SO_SNDTIMEO ROXY_SO_UNSUPPORTED
#else
#define SO_SNDTIMEO ROXY_SO_UNSUPPORTED
#endif
#endif

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#ifndef SO_RCVTIMEO_OLD
#define SO_RCVTIMEO_OLD ROXY_SO_UNSUPPORTED
#endif

#ifndef SO_SNDTIMEO_OLD
#define SO_SNDTIMEO_OLD ROXY_SO_UNSUPPORTED
#endif

#define SO_NO_CHECK ROXY_SO_UNSUPPORTED
#define SO_PRIORITY ROXY_SO_UNSUPPORTED
#define SO_BSDCOMPAT ROXY_SO_UNSUPPORTED
#define SO_REUSEPORT ROXY_SO_UNSUPPORTED
#define SO_PASSCRED ROXY_SO_UNSUPPORTED
#define SO_PEERCRED ROXY_SO_UNSUPPORTED
#define SO_PEERSEC ROXY_SO_UNSUPPORTED
#define SO_SNDBUFFORCE ROXY_SO_UNSUPPORTED
#define SO_RCVBUFFORCE ROXY_SO_UNSUPPORTED

#ifndef SO_TIMESTAMP
#if __LONG_MAX__ == 0x7fffffff
#define SO_TIMESTAMP ROXY_SO_UNSUPPORTED
#define SO_TIMESTAMPNS ROXY_SO_UNSUPPORTED
#define SO_TIMESTAMPING ROXY_SO_UNSUPPORTED
#else
#define SO_TIMESTAMP ROXY_SO_UNSUPPORTED
#define SO_TIMESTAMPNS ROXY_SO_UNSUPPORTED
#define SO_TIMESTAMPING ROXY_SO_UNSUPPORTED
#endif
#endif

#define SO_SECURITY_AUTHENTICATION ROXY_SO_UNSUPPORTED
#define SO_SECURITY_ENCRYPTION_TRANSPORT ROXY_SO_UNSUPPORTED
#define SO_SECURITY_ENCRYPTION_NETWORK ROXY_SO_UNSUPPORTED

#define SO_BINDTODEVICE ROXY_SO_UNSUPPORTED

#define SO_ATTACH_FILTER ROXY_SO_UNSUPPORTED
#define SO_DETACH_FILTER ROXY_SO_UNSUPPORTED
#define SO_GET_FILTER           SO_ATTACH_FILTER

#define SO_PEERNAME ROXY_SO_UNSUPPORTED
#define SCM_TIMESTAMP           SO_TIMESTAMP
#define SO_PASSSEC ROXY_SO_UNSUPPORTED
#define SCM_TIMESTAMPNS         SO_TIMESTAMPNS
#define SO_MARK ROXY_SO_UNSUPPORTED
#define SCM_TIMESTAMPING        SO_TIMESTAMPING
#define SO_RXQ_OVFL ROXY_SO_UNSUPPORTED
#define SO_WIFI_STATUS ROXY_SO_UNSUPPORTED
#define SCM_WIFI_STATUS         SO_WIFI_STATUS
#define SO_PEEK_OFF ROXY_SO_UNSUPPORTED
#define SO_NOFCS ROXY_SO_UNSUPPORTED
#define SO_LOCK_FILTER ROXY_SO_UNSUPPORTED
#define SO_SELECT_ERR_QUEUE ROXY_SO_UNSUPPORTED
#define SO_BUSY_POLL ROXY_SO_UNSUPPORTED
#define SO_MAX_PACING_RATE ROXY_SO_UNSUPPORTED
#define SO_BPF_EXTENSIONS ROXY_SO_UNSUPPORTED
#define SO_INCOMING_CPU ROXY_SO_UNSUPPORTED
#define SO_ATTACH_BPF ROXY_SO_UNSUPPORTED
#define SO_DETACH_BPF           SO_DETACH_FILTER
#define SO_ATTACH_REUSEPORT_CBPF ROXY_SO_UNSUPPORTED
#define SO_ATTACH_REUSEPORT_EBPF ROXY_SO_UNSUPPORTED
#define SO_CNX_ADVICE ROXY_SO_UNSUPPORTED
#define SCM_TIMESTAMPING_OPT_STATS 54
#define SO_MEMINFO ROXY_SO_UNSUPPORTED
#define SO_INCOMING_NAPI_ID ROXY_SO_UNSUPPORTED
#define SO_COOKIE ROXY_SO_UNSUPPORTED
#define SCM_TIMESTAMPING_PKTINFO 58
#define SO_PEERGROUPS ROXY_SO_UNSUPPORTED
#define SO_ZEROCOPY ROXY_SO_UNSUPPORTED
#define SO_TXTIME ROXY_SO_UNSUPPORTED
#define SCM_TXTIME              SO_TXTIME
#define SO_BINDTOIFINDEX ROXY_SO_UNSUPPORTED
#define SO_DETACH_REUSEPORT_BPF ROXY_SO_UNSUPPORTED
#define SO_PASSRIGHTS ROXY_SO_UNSUPPORTED
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#define SOL_SOCKET ROXY_SOL_BASE

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#define SOL_IP ROXY_SOL_UNSUPPORTED
#define SOL_IPV6 ROXY_SOL_UNSUPPORTED
#define SOL_ICMPV6 ROXY_SOL_UNSUPPORTED

#define SOL_RAW ROXY_SOL_UNSUPPORTED
#define SOL_DECNET ROXY_SOL_UNSUPPORTED
#define SOL_X25 ROXY_SOL_UNSUPPORTED
#define SOL_PACKET ROXY_SOL_UNSUPPORTED
#define SOL_ATM ROXY_SOL_UNSUPPORTED
#define SOL_AAL ROXY_SOL_UNSUPPORTED
#define SOL_IRDA ROXY_SOL_UNSUPPORTED
#define SOL_NETBEUI ROXY_SOL_UNSUPPORTED
#define SOL_LLC ROXY_SOL_UNSUPPORTED
#define SOL_DCCP ROXY_SOL_UNSUPPORTED
#define SOL_NETLINK ROXY_SOL_UNSUPPORTED
#define SOL_TIPC ROXY_SOL_UNSUPPORTED
#define SOL_RXRPC ROXY_SOL_UNSUPPORTED
#define SOL_PPPOL2TP ROXY_SOL_UNSUPPORTED
#define SOL_BLUETOOTH ROXY_SOL_UNSUPPORTED
#define SOL_PNPIPE ROXY_SOL_UNSUPPORTED
#define SOL_RDS ROXY_SOL_UNSUPPORTED
#define SOL_IUCV ROXY_SOL_UNSUPPORTED
#define SOL_CAIF ROXY_SOL_UNSUPPORTED
#define SOL_ALG ROXY_SOL_UNSUPPORTED
#define SOL_NFC ROXY_SOL_UNSUPPORTED
#define SOL_KCM ROXY_SOL_UNSUPPORTED
#define SOL_TLS ROXY_SOL_UNSUPPORTED
#define SOL_XDP ROXY_SOL_UNSUPPORTED
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#define SOMAXCONN       4096

#define MSG_OOB ROXY_MSG_UNSUPPORTED
#define MSG_PEEK ROXY_MSG_UNSUPPORTED
#define MSG_DONTROUTE ROXY_MSG_UNSUPPORTED
#define MSG_CTRUNC ROXY_MSG_UNSUPPORTED
#define MSG_TRUNC ROXY_MSG_UNSUPPORTED
#define MSG_EOR ROXY_MSG_UNSUPPORTED
#define MSG_WAITALL ROXY_MSG_UNSUPPORTED
#define MSG_NOSIGNAL (ROXY_MSG_BASE << 1)
#define MSG_CMSG_CLOEXEC ROXY_MSG_UNSUPPORTED

#if defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN
#define MSG_PROXY ROXY_MSG_UNSUPPORTED
#define MSG_DONTWAIT ROXY_MSG_BASE
#define MSG_FIN ROXY_MSG_UNSUPPORTED
#define MSG_SYN ROXY_MSG_UNSUPPORTED
#define MSG_CONFIRM ROXY_MSG_UNSUPPORTED
#define MSG_RST ROXY_MSG_UNSUPPORTED
#define MSG_ERRQUEUE ROXY_MSG_UNSUPPORTED
#define MSG_MORE ROXY_MSG_UNSUPPORTED
#define MSG_WAITFORONE ROXY_MSG_UNSUPPORTED
#define MSG_BATCH ROXY_MSG_UNSUPPORTED
#define MSG_ZEROCOPY ROXY_MSG_UNSUPPORTED
#define MSG_FASTOPEN ROXY_MSG_UNSUPPORTED
#endif /* defined(_DEFAULT_SOURCE) || __MLIBC_XOPEN */

#endif
