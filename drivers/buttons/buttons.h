#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

void buttons_init(void);
bool buttons_read(int button_id);

#endif