# ESP32 IDF Ethernet library
* **Attention:** This library only support LAN8720.

## Wires
* Be careful with the ETH clock. It is the biggest source of problems.
* I use GPIO 16 as a clock source (connected to "nINT/RETCLCK"). In this case, dont forget to correctly configure the settings in the "ETH menuconfig".

| GPIO   | RMII Signal | Notes         |
| ------ | ----------- | ------------  |
| GPIO21 | TX_EN       | EMAC_TX_EN    |
| GPIO19 | TX0         | EMAC_TXD0     |
| GPIO22 | TX1         | EMAC_TXD1     |
| GPIO25 | RX0         | EMAC_RXD0     |
| GPIO26 | RX1         | EMAC_RXD1     |
| GPIO27 | CRS_DV      | EMAC_RX_DRV   |
| GPIO23 | MDC         | Output to PHY |
| GPIO18 | MDIO        | Bidirectional |


## Simple example to start Ethernet
```
ETH eth;
if (eth.init()) //Wait IP (DHCP) up to 20sec.
{
    ESP_LOGI("ESP32", "Connected with IP: %s", eth.get_ip());
}
else
{
    ESP_LOGE("ESP32", "Fail to connect");
}
```