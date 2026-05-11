#include "lan8720.h"
#include <stdio.h>

extern ETH_HandleTypeDef heth;

// --- MDIO helpers (HAL wrappers) ---
static HAL_StatusTypeDef phy_read(uint16_t reg, uint32_t *value) {
    return HAL_ETH_ReadPHYRegister(&heth, LAN8720_PHY_ADDR, reg, value);
}

static HAL_StatusTypeDef phy_write(uint16_t reg, uint32_t value) {
    return HAL_ETH_WritePHYRegister(&heth, LAN8720_PHY_ADDR, reg, value);
}
// NOTE: HAL_ETH on F407 uses the PHY address configured in heth.Init.PhyAddress.
// That's set in MX_ETH_Init(); we'll override it to LAN8720_PHY_ADDR below.

// --- Public API ---

// Pulse PHY nRST: drive low ≥100us, release, wait 100ms.
int LAN8720_HardReset(void) {
    HAL_GPIO_WritePin(PHY_RST_GPIO_Port, PHY_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);   // plenty more than 100us minimum
    HAL_GPIO_WritePin(PHY_RST_GPIO_Port, PHY_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);  // datasheet: tRC = 100ms before MDIO access is reliable
    return 0;
}

int LAN8720_AliveCheck(void) {
    uint32_t id1 = 0, id2 = 0;
    HAL_StatusTypeDef s;

    s = phy_read(PHY_REG_ID1, &id1);
    if (s != HAL_OK) {
        printf("MDIO read ID1 failed (status=%d)\r\n", s);
        return -1;
    }
    s = phy_read(PHY_REG_ID2, &id2);
    if (s != HAL_OK) {
        printf("MDIO read ID2 failed (status=%d)\r\n", s);
        return -1;
    }

    printf("PHY ID1=0x%04lX  ID2=0x%04lX\r\n", id1, id2);

    if (id1 == 0xFFFF || id1 == 0x0000) {
        printf("  -> MDIO bus not working (id=0xFFFF or 0x0000)\r\n");
        printf("  -> Check 50MHz clock at PA1, check PHY power, check PHY address\r\n");
        return -1;
    }

    if (id1 == LAN8720_ID1 &&
        (id2 & LAN8720_ID2_MASK) == LAN8720_ID2_VALUE) {
        uint8_t rev = id2 & 0x000F;
        printf("  -> LAN8720A confirmed (silicon rev %u)\r\n", rev);
        return 0;
    }

    printf("  -> Unknown PHY. LAN8720A expected 0x0007/0xC0F*\r\n");
    return -1;
}

int LAN8720_WaitLink(uint32_t timeout_ms) {
    uint32_t bsr = 0;
    uint32_t start = HAL_GetTick();

    printf("Waiting for link... (plug in cable)\r\n");
    while (HAL_GetTick() - start < timeout_ms) {
        if (phy_read(PHY_REG_BSR, &bsr) != HAL_OK) continue;
        if (bsr & PHY_BSR_LINK_UP) {
            printf("Link UP after %lu ms\r\n", HAL_GetTick() - start);
            // Link is up but auto-neg may still be finishing
            uint32_t wait_start = HAL_GetTick();
            while (HAL_GetTick() - wait_start < 3000) {
                phy_read(PHY_REG_BSR, &bsr);
                if (bsr & PHY_BSR_ANEG_COMPLETE) {
                    printf("Auto-negotiation complete\r\n");
                    return 0;
                }
                HAL_Delay(50);
            }
            printf("Auto-neg did not complete (BSR=0x%04lX)\r\n", bsr);
            return -1;
        }
        HAL_Delay(100);
    }
    printf("Link DOWN timeout (BSR=0x%04lX)\r\n", bsr);
    return -1;
}

void LAN8720_PrintStatus(void) {
    uint32_t bcr = 0, bsr = 0, spscr = 0;
    phy_read(PHY_REG_BCR, &bcr);
    phy_read(PHY_REG_BSR, &bsr);
    phy_read(PHY_REG_SPSCR, &spscr);

    printf("BCR  = 0x%04lX  BSR = 0x%04lX  SPSCR = 0x%04lX\r\n",
           bcr, bsr, spscr);

    const char *speed_duplex = "unknown";
    switch (spscr & PHY_SPSCR_SPEED_MASK) {
        case PHY_SPSCR_10HD:   speed_duplex = "10BASE-T Half";  break;
        case PHY_SPSCR_100HD:  speed_duplex = "100BASE-TX Half"; break;
        case PHY_SPSCR_10FD:   speed_duplex = "10BASE-T Full";  break;
        case PHY_SPSCR_100FD:  speed_duplex = "100BASE-TX Full"; break;
    }
    printf("  Link: %s,  Speed/Duplex: %s\r\n",
           (bsr & PHY_BSR_LINK_UP) ? "UP" : "DOWN",
           speed_duplex);
}
