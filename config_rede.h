#pragma once

#ifndef COMMON_MODULE_H
#define COMMON_MODULE_H

#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h> 
#include <string.h>

#define STR_LEN 11

struct aviator_msg {
    int32_t player_id;
    float value;
    char type[STR_LEN];
    float player_profit;
    float house_profit;
};

int configura_addr(const char *str_addr, const char *porta_str, struct sockaddr_storage *server_addr);

int inicializar_addr_server(const char *protocolo, const char* porta_str, struct sockaddr_storage *server_addr);

#endif