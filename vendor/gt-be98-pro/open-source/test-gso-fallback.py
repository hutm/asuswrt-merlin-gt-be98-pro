#!/usr/bin/env python3
"""Compile actual kernel dispatch functions with mock packet callbacks in Docker.

Tests dispatch and ownership only, not segmentation, real packets or kernel races.
"""
from pathlib import Path
import subprocess
import tempfile
import os

ROOT = Path(__file__).resolve().parents[3]
SOURCE = 'release/src-rt-5.04behnd.4916/kernel/bcmkernel/net/core/bcm_sw_gso.c'
source = subprocess.check_output(['git', '-C', str(ROOT), 'show', 'HEAD:' + SOURCE], text=True)


def function(name):
    start = source.index('int ' + name + '(')
    end = source.index('\nEXPORT_SYMBOL(' + name + ');', start)
    return source[start:end]


prefix = r'''
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#define NETDEV_TX_OK 0
#define NETDEV_TX_BUSY 16
struct sk_buff { int placeholder; };
struct net_device { int placeholder; };
typedef int (*HardStartXmitFuncP)(struct sk_buff *, struct net_device *);
typedef int (*gso_skb_prehandle_cb)(struct sk_buff *, struct net_device *);
typedef int (*sw_gso_enqueue_cb)(struct sk_buff *, struct net_device *,
                               gso_skb_prehandle_cb, HardStartXmitFuncP);
static sw_gso_enqueue_cb gso_thread_enq_cb;
static int segments, frees, precalls, pre_result;
static void dev_kfree_skb_any(struct sk_buff *p) { assert(p); ++frees; }
static int bcm_sw_gso(struct sk_buff *p, struct net_device *d, HardStartXmitFuncP f)
{ assert(p && d && f); ++segments; return 0; }
static int pre(struct sk_buff *p, struct net_device *d)
{ assert(p && d); ++precalls; return pre_result; }
static int xmit(struct sk_buff *p, struct net_device *d)
{ assert(p && d); return 0; }
static int unavailable(struct sk_buff *p, struct net_device *d,
                       gso_skb_prehandle_cb a, HardStartXmitFuncP b)
{ (void)p; (void)d; (void)a; (void)b; return NETDEV_TX_BUSY; }
'''
tests = r'''
int main(void) {
    struct sk_buff p = {0};
    struct net_device d = {0};
    /* No worker: direct GSO path is used, not silent packet loss. */
    assert(bcm_sw_gso_xmit(&p, &d, xmit) == 0);
    assert(segments == 1 && frees == 0);
    /* Busy worker must still fall back. */
    gso_thread_enq_cb = unavailable;
    assert(bcm_sw_gso_xmit(&p, &d, xmit) == 0);
    assert(segments == 2 && frees == 0);
    gso_thread_enq_cb = NULL;
    /* Successful pre-handler owns the packet: wrapper must not free again. */
    pre_result = NETDEV_TX_OK;
    assert(bcm_gso_enqueue(&p, &d, pre) == 0);
    assert(precalls == 1 && frees == 0);
    /* Rejected pre-handler packet is freed once. */
    pre_result = NETDEV_TX_BUSY;
    assert(bcm_gso_enqueue(&p, &d, pre) == NETDEV_TX_BUSY);
    assert(precalls == 2 && frees == 1);
    assert(bcm_sw_gso_skb_prehandle(&p, &d, NULL, xmit) == 0);
    assert(segments == 3 && frees == 1);
    assert(bcm_sw_gso_skb_prehandle(NULL, &d, pre, NULL) == NETDEV_TX_BUSY);
    assert(precalls == 2 && frees == 1);
    puts("actual source dispatch: absent/busy worker, callbacks and ownership passed");
    return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='gso-dispatch-') as directory:
    path = Path(directory)
    path.chmod(0o755)
    (path / 'test.c').write_text(prefix + '\n'.join(function(name) for name in (
        'bcm_sw_gso_xmit', 'bcm_sw_gso_skb_prehandle', 'bcm_gso_enqueue')) + tests)
    subprocess.run(['docker', 'run', '--rm', '--user', f'{os.getuid()}:{os.getgid()}',
                    '-v', f'{path}:/source:ro', 'amng-build', 'bash', '-c',
                    'gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined '
                    '/source/test.c -o /tmp/test && /tmp/test'], check=True)
