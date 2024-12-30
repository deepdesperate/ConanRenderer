#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Use SDL to query what is the fullscreen max. width and height
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;


    // Create a SDL window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );

    if (!window){
        fprintf(stderr, "Error Creating SDL Window.\n");
        return false;
    }

    //
    // TODO: Create a SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void draw_grid(void) {
    // TODO: Draw a background grid that fills the entire window.
    // Lines should be rendered at every row/col multiple of 10.

    for( int i = 0; i < window_height; i += 10){
        for (int j = 0; j < window_width; j += 10){
            if (i % 10 == 0 || j % 10 == 0){
                color_buffer[(window_width * i) + j] = 0xFF333333;
            }
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if( x >= 0 && x < window_width && y >= 0 && y < window_height){
        color_buffer[(window_width * y) + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color){
    for( int i = x; i < x + width; i++){
        for(int j = y; j < y + height; j++){
            draw_pixel(i, j, color);
            // color_buffer[j * window_width + i] = color;
        }
    }
}

void render_color_buffer(void){
    // Puts color_buffer to the color_buffer_texture
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)(window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++){
        for( int x=0; x < window_width; x++){
            color_buffer[(window_width * y) + x ] = color;
        }
    } 
}

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); 
    SDL_Quit();
}
