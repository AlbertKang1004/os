#ifndef DEBUG_H
#define DEBUG_H

#include "../drivers/serial.h"
#include "utils.h"

#ifdef DEBUG
    #define LOG(msg)           serial_write(SERIAL_COM1_BASE, msg "\n")
    #define LOG_HEX(label, val) \
        serial_write(SERIAL_COM1_BASE, label ": "); \
        serial_write(SERIAL_COM1_BASE, print_hex(val)); \
        serial_write(SERIAL_COM1_BASE, "\n")
    #define LOG_STR(label, val) \
        serial_write(SERIAL_COM1_BASE, label ": "); \
        serial_write(SERIAL_COM1_BASE, val); \
        serial_write(SERIAL_COM1_BASE, "\n")
#else
    #define LOG(msg)
    #define LOG_HEX(label, val)
    #define LOG_STR(label, val)
#endif

#endif