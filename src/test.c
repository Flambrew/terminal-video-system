#include <stdint.h>

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
    Viewport *vp = vp_alloc(RESOLUTION_SMALL, FPS_HIGH, RGB);

    int64_t k;
    for (k = vp->fps * 5; k >= 0; --k) {
        draw(vp, k);
        vp_update_buffer(vp);
        vp_frame_await(vp);
        vp_print_buffer(vp, k == 0);
    }

    vp_free(vp);
    return 0;
}