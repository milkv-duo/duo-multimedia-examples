#include "fb_display.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "log.h"

FBDisplay::FBDisplay() {}
FBDisplay::~FBDisplay() {
    if (_sws_ctx) {
        sws_freeContext(_sws_ctx);
    }
    if (_fbMMap) {
        memset(_fbMMap, 0x00, _screensize); // 清屏
        munmap(_fbMMap, _screensize);
    }
    if (_fd) {
        close(_fd);
    }
}

int FBDisplay::open_device(std::string device) {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    _fd = ::open(device.c_str(), O_RDWR);
    if (_fd < 0) {
        ERROR("Could not open device \'{0}\'", device);
        return -1;
    }
    if (ioctl(_fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        ERROR("Could not use ioctl!");
        return -1;
    }

    if (ioctl(_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        ERROR("Could not use ioctl!");
        return -1;
    }

    _width = vinfo.xres;
    _height = vinfo.yres;
    _bpp = vinfo.bits_per_pixel;
    INFO("_bpp = {0}, _width = {1}, _height = {2}, _vwidth = {3}, _vheight = "
         "{4}",
         _bpp, _width, _height, vinfo.xres_virtual, vinfo.yres_virtual);

    if (_bpp == 16 || _bpp == 18) {
        _format = AV_PIX_FMT_RGB565;
    } else if (_bpp == 24) {
        _format = AV_PIX_FMT_RGB24;
    } else if (_bpp == 32) {
        _format = AV_PIX_FMT_RGBA;
    } else {
        ERROR("Unsported format! vinfo.bits_per_pixel = {0}", _bpp);
        return -1;
    }

    _screensize = _width * _height * vinfo.bits_per_pixel / 8;

    _fbMMap = (unsigned char *)mmap(0, _screensize, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, _fd, 0);
    if (_fbMMap == MAP_FAILED) {
        ERROR("Could not mmap!");
        return -1;
    }

    return 0;
}
int FBDisplay::show_image(AVFrame *frame) {
    AVPixelFormat pix_fmt = (AVPixelFormat)frame->format;

    if (_sws_ctx == NULL) {
        _sws_ctx =
            sws_getContext(frame->width, frame->height, pix_fmt, _width,
                           _height, _format, SWS_BILINEAR, NULL, NULL, NULL);
    }

    AVFrame *rgbFrame = av_frame_alloc();
    rgbFrame->format = _format;
    rgbFrame->width = _width;
    rgbFrame->height = _height;
    av_frame_get_buffer(rgbFrame, 0);

    sws_scale(_sws_ctx, frame->data, frame->linesize, 0, frame->height,
              rgbFrame->data, rgbFrame->linesize);

    memcpy(_fbMMap, rgbFrame->data[0], _screensize);
    av_frame_free(&rgbFrame);
    return 0;
}