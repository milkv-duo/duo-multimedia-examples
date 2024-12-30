#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#include <string>

class FBDisplay {
  public:
    FBDisplay();
    ~FBDisplay();

    int open_device(std::string device);
    int show_image(AVFrame *pFrame);

  private:
    int _width;
    int _height;

    int _fd = 0;
    unsigned char *_fbMMap = 0;
    long int _screensize = 0;
    int _bpp;
    AVPixelFormat _format;
    struct SwsContext *_sws_ctx = NULL;
};