#pragma once

#include "esp_netif_ip_addr.h"

enum class WifiEvent {
    CONNETING,
    CONNECTED,
    DISCONNECTED,
    FAILED
};

struct WifiGotIp {
    esp_ip4_addr_t ip;
};
