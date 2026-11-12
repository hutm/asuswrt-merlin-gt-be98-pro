#!/usr/bin/env python3
"""Extract the public mesh declarations used by this model for compile checks."""
import pathlib
import re
import sys
PUBLIC_NAMES = ['add_led_ctrl_capability', 'amas_stop_acsd_config_init', 'cal_plc_cost', 'cal_space', 'checkMatchMloCondition', 'checkMloConnectionChange', 'checkMloWiFi', 'check_mlo_config', 'check_wlx_nband_type', 'create_amas_sys_folder', 'del_led_ctrl_capability', 'enable_ETH_U', 'gen_mlo_band', 'gen_uplinkport_describe', 'get_bandnum_by_nband', 'get_eth_count', 'get_mlo_fh_band', 'get_unit_by_wlc_bandindex', 'get_wifi_tx_maxpower', 'get_wl_count', 'get_wlc_bandindex_by_unit', 'isMLOConnectionSupported', 'isMloConnectionMode', 'is_compatible_network', 'is_connection_priority_is_wifi_first', 'is_mainFH_network', 'is_mlo_dwb_mssid', 'is_self_optmz_stage', 'trans_to_bhmode', 'trigger_opt', 'unset_selected_channel_info']
text = pathlib.Path(sys.argv[1]).read_text()
for name in PUBLIC_NAMES:
    declarations = list(dict.fromkeys(re.findall(r"extern\s+[^;\n]+\b" + re.escape(name) + r"\s*\([^;]*?\);", text)))
    if name == "trans_to_bhmode":
        declarations = [d for d in declarations if "amas_wifi_mlo_bhmode" in d]
    if len(declarations) != 1:
        raise SystemExit(f"Expected one model declaration for {name}, got {len(declarations)}")
    print(declarations[0])
