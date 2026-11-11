import sys
from pathlib import Path
p=Path(sys.argv[1]) / Path('release/src-rt-5.04behnd.4916/bcmdrivers/opensource/char/bcm_knvram/impl1')
h=(p/'include/wlcsm_linux.h').read_text()
c=(p/'src/wlcsm_nvram.c').read_text()
parts=['#include <string.h>\n', h[h.index('typedef struct t_wlcsm_name_valuepair {'):h.index('typedef struct t_wlcsm_mngr_varhdr {')], h[h.index('#define VALUEPAIR_NAME(v)'):h.index('typedef struct wlcsm_msg_hdr {')], c[c.index('static void _valuepair_set('):c.index('static t_WLCSM_NVRAM_TUPLE *_wlcsm_nvram_tuple_search(')]]
Path(sys.argv[2]).write_text(''.join(parts))
