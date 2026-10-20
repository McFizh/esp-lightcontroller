#pragma once

#include "esp_err.h"

esp_err_t init_tcp_server(void);
void run_tcp_loop(void);