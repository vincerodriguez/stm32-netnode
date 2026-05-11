#ifndef LAN8720_H
#define LAN8720_H

#include "main.h"
#include <stdint.h>

#define LAN8720_PHY_ADDR        1   // strapped via PHYAD0=1 on your board

// Standard IEEE 802.3 MII registers
#define PHY_REG_BCR             0x00  // Basic Control
#define PHY_REG_BSR             0x01  // Basic Status
#define PHY_REG_ID1             0x02  // PHY Identifier 1
#define PHY_REG_ID2             0x03  // PHY Identifier 2
#define PHY_REG_ANAR            0x04  // Auto-Neg Advertisement
#define PHY_REG_ANLPAR          0x05  // Auto-Neg Link Partner Ability

// LAN8720A-specific
#define PHY_REG_SPSCR           0x1F  // Special Control/Status (speed+duplex)

// BCR bits
#define PHY_BCR_RESET           (1 << 15)
#define PHY_BCR_ANEG_ENABLE     (1 << 12)
#define PHY_BCR_ANEG_RESTART    (1 << 9)

// BSR bits
#define PHY_BSR_LINK_UP         (1 << 2)
#define PHY_BSR_ANEG_COMPLETE   (1 << 5)

// SPSCR bits (LAN8720-specific)
#define PHY_SPSCR_SPEED_MASK    (0x7 << 2)
#define PHY_SPSCR_10HD          (0x1 << 2)
#define PHY_SPSCR_100HD         (0x2 << 2)
#define PHY_SPSCR_10FD          (0x5 << 2)
#define PHY_SPSCR_100FD         (0x6 << 2)

// Expected LAN8720A ID
#define LAN8720_ID1             0x0007
#define LAN8720_ID2_MASK        0xFFF0   // mask out revision nibble
#define LAN8720_ID2_VALUE       0xC0F0

int  LAN8720_HardReset(void);
int  LAN8720_AliveCheck(void);
int  LAN8720_WaitLink(uint32_t timeout_ms);
void LAN8720_PrintStatus(void);

#endif
