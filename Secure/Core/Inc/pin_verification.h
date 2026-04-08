#include "libtropic_common.h"

int pin_verification_init(void);

lt_ret_t check_pin(const uint8_t *pin, const uint8_t pin_size);

uint8_t remaining_tries();