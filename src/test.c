#include <stdint.h>
#include <stdio.h>

#include "../include/terminal-video-system.h"

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
    Viewport *vp = vp_alloc(RESOLUTION_LARGE, FPS_HIGH, RGB6X);

    int64_t k, s, n, p, cycles, frame;
    s = p = 0;
    cycles = vp->fps * 5;
    frame = 1000 / vp->fps;
    for (k = cycles - 1; k >= 0; --k) {
        draw(vp, k);
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
            vp->height, vp->width, vp->color_depth == BW ? "BW" : vp->color_depth == RGB ? "RGB" : vp->color_depth == RGBL ? "RGBL" :
            vp->color_depth == BW24X ? "BW24X" : vp->color_depth == RGB6X ? "RGB6X" : "RGB256X" , vp->fps, (int) (s / cycles),
            (int) (s / cycles / frame), (int) p, (int) (p / frame));

    vp_free(vp);
    return 0;
}