#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "include/display.h"

// -----=====<<<<< TESTING >>>>>=====----- //

static void draw(Viewport *vp, uint64_t k) {
    uint64_t i, j;
    for (i = 0; i < vp->height; ++i) {
        for (j = 0; j < vp->width; ++j) {
            vp->bitmap[i * vp->width + j] = ((i+j+k)%256 << 16) + ((i+(vp->width-j)+k)%256 << 8) + ((vp->height-i)+j+k)%256; 
        }
    }
}

int main() {
    Viewport *vp = vp_alloc(RES_LARGE, FPS_HIGH, MODE_RGB256X);

    int64_t k, s, n, p, frames, framelen;
    s = p = 0;
    frames = 342;//vp->fps * 5;
    framelen = 1000 / vp->fps;
    for (k = frames - 1; k >= 0; --k) {
        //draw(vp, k);
        char *path = "tcat/00000.bmp";
        path[7] += (frames - k) / 100; 
        path[8] += (frames - k) / 10 % 10; 
        path[9] += (frames - k) % 10; 
        vp_load_bmp(vp, path);
        vp_update_buffer(vp);
        n = vp_frame_await(vp);
        vp_print_buffer(vp, k == 0);
        if (k != vp->fps * 5 - 1) {
            if (n > p) {
                p = n;
            }
            s += n;
        }
    }

    printf("[Display] Resolution: %dx%dpx, Color: %s, Framerate: %dfps; [Latency] Average: %dms=%dfr, Peak: %dms=%dfr\n",
            vp->height, vp->width, vp->color_depth == MODE_BW ? "BW" : vp->color_depth == MODE_RGB ? "RGB" :
            vp->color_depth == MODE_RGBL ? "RGBL" : vp->color_depth == MODE_BW24X ? "BW24X" : vp->color_depth == MODE_RGB6X ?
            "RGB6X" : "RGB256X" , vp->fps, (int) (s / frames), (int) (s / frames / framelen), (int) p, (int) (p / framelen));
    vp_free(vp);
    return 0;
}