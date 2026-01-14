#pragma once

#include <stdint.h>

// Returns a stable bit for the given tag name, creating it if needed.
unsigned int phys_tag_bit(const char* name);

// Parses a '|' or comma separated list of tags into bit flags.
// Special tokens: "all" -> 0xFFFFFFFFu, "none" -> 0u.
unsigned int phys_parse_tag_list(const char* s);

// Resets the registry (useful for tests).
void phys_tag_reset_registry(void);
