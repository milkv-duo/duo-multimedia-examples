#pragma once
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>

int32_t find_pre_start_code(uint8_t *buf, int32_t bufLen);
std::string intToHex(int value);