#include "platform/screen.h"

#include "SDL.h"

#include "game/settings.h"
#include "graphics/screen.h"

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

static struct {
    int x;
    int y;
} window_pos;

void platform_screen_create(const char *title, int width, int height, int fullscreen)
{
    SDL_Log("Creating screen, fullscreen? %d\n", fullscreen);
    if (SDL.window) {
        SDL_DestroyWindow(SDL.window);
        SDL.window = 0;
    }
    if (SDL.renderer) {
        SDL_DestroyRenderer(SDL.renderer);
        SDL.renderer = 0;
    }
    
    Uint32 flags = SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL.window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, flags);
    
    SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_PRESENTVSYNC);
    
    platform_screen_resize(width, height, fullscreen);
}

void platform_screen_resize(int width, int height, int fullscreen)
{
    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
    }
    
    setting_set_display(fullscreen, width, height);
    SDL.texture = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        width, height);
    if (SDL.texture) {
        SDL_Log("Texture created (%d x %d)\n", width, height);
        screen_set_resolution(width, height);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture: %s\n", SDL_GetError());
    }
}

void platform_screen_set_fullscreen()
{
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    int orig_w, orig_h;
    SDL_GetWindowSize(SDL.window, &orig_w, &orig_h);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
    SDL_Log("User to fullscreen %d x %d\n", mode.w, mode.h);
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowDisplayMode(SDL.window, &mode);
    setting_set_display(1, mode.w, mode.h);
}

void platform_screen_set_windowed()
{
    int width, height;
    setting_window(&width, &height);
    SDL_Log("User to windowed %d x %d\n", width, height);
    SDL_SetWindowFullscreen(SDL.window, 0);
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    setting_set_display(0, width, height);
}

void platform_screen_set_window_size(int width, int height)
{
    if (setting_fullscreen()) {
        SDL_SetWindowFullscreen(SDL.window, 0);
    }
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    SDL_Log("User resize to %d x %d\n", width, height);
    setting_set_display(0, width, height);
}

void platform_screen_render(const void *canvas)
{
    SDL_UpdateTexture(SDL.texture, NULL, canvas, screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
    SDL_RenderPresent(SDL.renderer);
}