/* SPDX-License-Identifier: GPL-2.0-only */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
extern int nvram_set(const char *, const char *);
extern int nvram_set_int(const char *, int);
extern char *psname(int, char *, int);
extern int validate_rc_service(const char *);
extern size_t strlcpy(char *, const char *, size_t);
extern int _eval(char *const [], const char *, int, pid_t *);
extern void logmessage_normal(const char *, const char *, ...);

/* This pointer is writable in the public ABI. */
char *skip_table = "restart_autowan";

int skippable_event(const char *event)
{
    return event && !strcmp(skip_table, event);
}

static void read_service(char *dst, const char *key)
{
    const char *value = nvram_get(key);
    strlcpy(dst, value ? value : "", 128);
}

int wait_rc_service(int wait, const char *event)
{
    char current[128], ack[128], process[16];
    int left = wait;
    int threshold = wait < 7 ? 7 : wait;
    int first = 1;

    psname(nvram_get_int("rc_service_pid"), process, sizeof(process));
    read_service(current, "rc_service");
    read_service(ack, "last_rc_service");
    while (*current) {
        /* Test before decrementing to avoid overflow for INT_MIN. */
        if (left <= 0)
            return 0;
        --left;
        if (strcmp(current, "restart_autowan")) {
            if (first)
                logmessage_normal("rc_service",
                    "Waiting for %s (ack %s, process %s)", current, ack, process);
            /* Preserve the existing diagnostic settings read. */
            (void)nvram_get_int("rc_service_pid");
        }
        if (!skippable_event(event) && wait - left >= threshold &&
            strncmp(current, ack, strlen(current))) {
            logmessage_normal("rc_service",
                "Clearing unacknowledged request %s (ack %s)", current, ack);
            nvram_set("rc_service", "");
            nvram_set("rc_service_pid", "");
            return 1;
        }
        sleep(1);
        read_service(current, "rc_service");
        read_service(ack, "last_rc_service");
        first = 0;
    }
    return 1;
}

static int notify(const char *event, int after, int wait)
{
    char process[16];

    if (!event || !*event || wait < 0 || !validate_rc_service(event))
        return -1;
    psname(getpid(), process, sizeof(process));
    if (strcmp(event, "restart_autowan"))
        logmessage_normal("rc_service", "%s %d requests %s",
                          process, (int)getpid(), event);
    if (strstr(event, "reboot")) {
        char reason[64];
        char *args[] = { "killall", "sh", NULL };
        snprintf(reason, sizeof(reason), "%s(%d)_call_rc", process, (int)getpid());
        nvram_set("sys_reboot_reason", reason);
        _eval(args, NULL, 0, NULL);
    }
    if (!wait_rc_service(wait, event)) {
        if (strcmp(event, "restart_autowan"))
            logmessage_normal("rc_service", "Skipping busy request %s", event);
        return 1;
    }
    nvram_set("rc_service", event);
    nvram_set_int("rc_service_pid", getpid());
    kill(1, SIGUSR1);
    if (after) {
        int left = wait;
        const char *current;
        /* Read ordering and post-decrement are part of the legacy return ABI. */
        while ((current = nvram_get("rc_service")) &&
               !strcmp(current, event) && left-- > 0)
            sleep(1);
        if (left == 0 && (current = nvram_get("rc_service")) &&
            !strcmp(current, event))
            return 2;
    }
    return 0;
}

int notify_rc(const char *event)
{
    return notify(event, 0, 15);
}

int notify_rc_after_wait(const char *event)
{
    return notify(event, 0, 30);
}

int notify_rc_after_period_wait(const char *event, int wait)
{
    return notify(event, 0, wait);
}

int notify_rc_and_wait(const char *event)
{
    return notify(event, 1, 10);
}

int notify_rc_and_wait_1min(const char *event)
{
    return notify(event, 1, 60);
}

int notify_rc_and_wait_2min(const char *event)
{
    return notify(event, 1, 120);
}

int notify_rc_and_period_wait(const char *event, int wait)
{
    return notify(event, 1, wait);
}
