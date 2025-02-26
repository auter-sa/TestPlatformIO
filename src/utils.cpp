#include <Arduino.h>
#include "utils.h"

int Milliseconds2Ticks(int ms) {
    return ms / portTICK_PERIOD_MS;
}
