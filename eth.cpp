#include "eth.h"

int8_t ETH::_status = -1;
char ETH::_ip[] = {0};

/**
 * @brief ETH event handler
 */
void ETH::eth_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
    _status = event_id;

    switch (event_id)
    {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI("ETH", "Link UP [%02x:%02x:%02x:%02x:%02x:%02x]",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            memset(_ip, 0, sizeof(_ip));
            ESP_LOGW("ETH", "Link DOWN");
            break;
        case ETHERNET_EVENT_START:
            //ESP_LOGI("ETH", "Started");
            break;
        case ETHERNET_EVENT_STOP:
            //ESP_LOGW("ETH", "Stopped");
            break;
    }
}

/**
 * @brief IP event handler
 */
void ETH::ip_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const tcpip_adapter_ip_info_t *ip_info = &event->ip_info;
    snprintf(_ip, sizeof(_ip), "%s", ip4addr_ntoa(&ip_info->ip));

    //ESP_LOGI("ETH", "IP: %s", _ip);
}

/**
 * @brief Get ETH status.
 * 
 * This status refers to all events [typedef enum eth_event_t] , check it!
 */
int8_t ETH::status()
{
    return _status;
}

/**
 * @brief Get ETH IP Address.
 * 
 * @return [*char] IP Address.
 */
char *ETH::get_ip()
{
    return _ip;
}

/**
 * @brief Init Ethernet.
 * 
 * @param [wait=0]: Dont wait for IP (DHCP).
 * @param [wait=1]: Wait for IP (DHCP).
 * 
 * @return 0: Fail.
 * @return 1: Sucess.
 */
int8_t ETH::init(uint8_t wait=1)
{
    esp_err_t err;


    tcpip_adapter_init();

    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 1 [0x%02x]", err);
        return 0;
    }

    esp_event_loop_init(NULL, NULL);
    err = tcpip_adapter_set_default_eth_handlers();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 2 [0x%02x]", err);
        return 0;
    }

    err = esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 3 [0x%02x]", err);
        return 0;
    }

    err = esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &ip_event, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 4 [0x%02x]", err);
        return 0;
    }

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = -1;//5
    mac_config.smi_mdc_gpio_num = 23;
    mac_config.smi_mdio_gpio_num = 18;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
    
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    err = esp_eth_driver_install(&config, &eth_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 5 [0x%02x]", err);
        return 0;
    }

    err = esp_eth_start(eth_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Error 6 [0x%02x]", err);
        return 0;
    }

    if (wait)
    {
        for (int16_t i = 0; i < 2000; i++)
        {
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(10));

            if (strlen(get_ip()) > 6)
                {return 1;}
        }

        ESP_LOGW(tag, "Fail to get IP in 20sec.");
    }
    else
    {
        return 1;
    }
    
    return 0;
}