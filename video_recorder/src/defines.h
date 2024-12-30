#pragma once

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}


struct VideoInfo {
    int width = 1920;
    int height = 1080;
    int frame = 15;
    int rate = 128; // kbps
};

class EncoderCallback {
  public:
    virtual void onVideoData(AVPacket *pkt) = 0;
};