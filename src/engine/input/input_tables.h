#pragma once

#include <stdbool.h>

bool key_code_from_name(const char* name, int* out_code);
const char* key_name_from_code(int code);
