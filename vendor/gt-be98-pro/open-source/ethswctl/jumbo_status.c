/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
int outputJumboStatus_GPHY_0(unsigned mask)
{
    return puts(mask & (1u << 0)
        ? "  GPHY_0 port accepts jumbo frames."
        : "  GPHY_0 port rejects jumbo frames.");
}

int outputJumboStatus_GPHY_1(unsigned mask)
{
    return puts(mask & (1u << 1)
        ? "  GPHY_1 port accepts jumbo frames."
        : "  GPHY_1 port rejects jumbo frames.");
}

int outputJumboStatus_GMII_1(unsigned mask)
{
    return puts(mask & (1u << 2)
        ? "  GMII_1 port accepts jumbo frames."
        : "  GMII_1 port rejects jumbo frames.");
}

int outputJumboStatus_GMII_2(unsigned mask)
{
    return puts(mask & (1u << 3)
        ? "  GMII_2 port accepts jumbo frames."
        : "  GMII_2 port rejects jumbo frames.");
}

int outputJumboStatus_GPON_SERDES(unsigned mask)
{
    return puts(mask & (1u << 4)
        ? "  GPON_SERDES port accepts jumbo frames."
        : "  GPON_SERDES port rejects jumbo frames.");
}

int outputJumboStatus_MOCA(unsigned mask)
{
    return puts(mask & (1u << 5)
        ? "  MOCA port accepts jumbo frames."
        : "  MOCA port rejects jumbo frames.");
}

int outputJumboStatus_USB(unsigned mask)
{
    return puts(mask & (1u << 6)
        ? "  USB port accepts jumbo frames."
        : "  USB port rejects jumbo frames.");
}

int outputJumboStatus_GPON(unsigned mask)
{
    return puts(mask & (1u << 7)
        ? "  GPON port accepts jumbo frames."
        : "  GPON port rejects jumbo frames.");
}

int outputJumboStatus_MIPS(unsigned mask)
{
    return puts(mask & (1u << 8)
        ? "  MIPS port accepts jumbo frames."
        : "  MIPS port rejects jumbo frames.");
}

/* Port selectors wrap at nine bits; 9 prints all ports in hardware order. */
int outputJumboStatus(unsigned port, unsigned mask)
{
    switch (port & 511u) {
    case 0: return outputJumboStatus_GPHY_0(mask);
    case 1: return outputJumboStatus_GPHY_1(mask);
    case 2: return outputJumboStatus_GMII_1(mask);
    case 3: return outputJumboStatus_GMII_2(mask);
    case 4: return outputJumboStatus_GPON_SERDES(mask);
    case 5: return outputJumboStatus_MOCA(mask);
    case 6: return outputJumboStatus_USB(mask);
    case 7: return outputJumboStatus_GPON(mask);
    case 8: return outputJumboStatus_MIPS(mask);
    case 9:
        printf("JUMBO_PORT_MASK:0x%08X\n", mask);
        outputJumboStatus_GPHY_0(mask);
        outputJumboStatus_GPHY_1(mask);
        outputJumboStatus_GMII_1(mask);
        outputJumboStatus_GMII_2(mask);
        outputJumboStatus_GPON_SERDES(mask);
        outputJumboStatus_MOCA(mask);
        outputJumboStatus_USB(mask);
        outputJumboStatus_GPON(mask);
        return outputJumboStatus_MIPS(mask);
    default:
        return (int)(port & 511u);
    }
}
