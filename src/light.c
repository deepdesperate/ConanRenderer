#include <stdint.h>
#include "light.h"

static light_t light = {
    .direction = { 0, 0, 1}
};

void init_light(vec3_t direction) {
    light.direction = direction;
}

vec3_t get_light_direction(void){
    return light.direction;
}

// Change color based on a percentage facor to represent light intensity

uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor) {
    if(percentage_factor < 0) percentage_factor = 0;
    if(percentage_factor > 1) percentage_factor = 1;

    // Separating argb and applying percentage factor
    uint32_t a = (original_color & 0xFF000000);
    uint32_t r = (original_color & 0x00FF0000) * percentage_factor ;
    uint32_t g = (original_color & 0x0000FF00) * percentage_factor ;
    uint32_t b = (original_color & 0x000000FF) * percentage_factor;

    // Rejoining color components
    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);

    return new_color;
}