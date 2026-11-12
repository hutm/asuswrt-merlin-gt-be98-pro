/* SPDX-License-Identifier: GPL-2.0-only */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include "aae_ipc.h"

/* ARM32 wire layout: data[2048], length[4], wait[1], padding[3], socket[4]. */
enum { PAYLOAD_SIZE = 2048, PACKET_SIZE = 2060, WAIT_OFFSET = 2052 };
_Static_assert(AAE_MAX_IPC_PACKET_SIZE == PAYLOAD_SIZE, "IPC payload layout changed");
#ifndef OPEN_IPC_HOST_TEST
_Static_assert(sizeof(size_t) == 4, "This wire protocol targets ARM32 userspace");
_Static_assert(sizeof(struct aaeIpcArgStruct) == PACKET_SIZE, "IPC packet layout changed");
_Static_assert(offsetof(struct aaeIpcArgStruct, waitResp) == WAIT_OFFSET,
               "IPC response flag offset changed");
#endif

static int64_t now_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
        return -1;
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int ready(int fd, short events, int64_t deadline)
{
    for (;;) {
        int64_t now = now_ms();
        if (now < 0)
            return 0;
        int64_t left = deadline - now;
        if (left <= 0) {
            errno = ETIMEDOUT;
            return 0;
        }
        struct pollfd p = { fd, events, 0 };
        int result = poll(&p, 1, left > INT_MAX ? INT_MAX : (int)left);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            return 0;
        }
        if (!result)
            continue;
        if (p.revents & POLLNVAL) {
            errno = EBADF;
            return 0;
        }
        if (p.revents & (events | POLLERR | POLLHUP))
            return 1;
    }
}

static int transaction(char *path, char *data, int length,
                       char *out, int capacity, int timeout, int response)
{
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    unsigned char packet[PACKET_SIZE] = { 0 }, received[PACKET_SIZE];
    int fd = -1, success = 0;
    int64_t start;

    if (!path || !*path || length < 0 || length > PAYLOAD_SIZE ||
        (!data && length) || (response && (!out || capacity <= 0 || timeout <= 0))) {
        errno = EINVAL;
        return 0;
    }
    size_t path_length = strnlen(path, sizeof(addr.sun_path));
    if (path_length == sizeof(addr.sun_path)) {
        errno = ENAMETOOLONG;
        return 0;
    }
    memcpy(addr.sun_path, path, path_length + 1);
    if (length)
        memcpy(packet, data, (size_t)length);
    packet[WAIT_OFFSET] = !!response;
    start = now_ms();
    if (start < 0)
        return 0;
    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0)
        return 0;
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        goto done;
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (errno != EINPROGRESS || !ready(fd, POLLOUT, start + 2000))
            goto done;
    }
    int status = 0;
    socklen_t status_length = sizeof(status);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &status_length) < 0)
        goto done;
    if (status) {
        errno = status;
        goto done;
    }
    start = now_ms();
    if (start < 0)
        goto done;
    size_t sent = 0;
    while (sent < sizeof(packet)) {
        ssize_t n = send(fd, packet + sent, sizeof(packet) - sent, MSG_NOSIGNAL);
        if (n > 0) {
            sent += (size_t)n;
            continue;
        }
        if (!n) {
            errno = EPIPE;
            goto done;
        }
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            goto done;
        if (!ready(fd, POLLOUT, start + 2000))
            goto done;
    }
    if (!response) {
        success = 1;
        goto done;
    }
    start = now_ms();
    if (start < 0)
        goto done;
    for (;;) {
        if (!ready(fd, POLLIN, start + (int64_t)timeout * 1000))
            goto done;
        ssize_t n = read(fd, received, sizeof(received));
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            goto done;
        }
        /* Preserve the API's first-read response and NUL-terminated truncation. */
        size_t copied = (size_t)n < (size_t)capacity ? (size_t)n : (size_t)capacity - 1;
        memset(out, 0, (size_t)capacity);
        memcpy(out, received, copied);
        success = 1;
        break;
    }
done:
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
    }
    return success;
}

int aae_sendIpcMsg(char *path, char *data, int length)
{
    return transaction(path, data, length, NULL, 0, 0, 0);
}

int aae_sendIpcMsgAndWaitResp(char *path, char *data, int length,
                            char *out, int capacity, int timeout)
{
    int success = transaction(path, data, length, out, capacity, timeout, 1);
    if (!success && out && capacity > 0)
        out[0] = '\0';
    return success;
}
