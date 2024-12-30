#pragma once

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Timer.h"

#include "log.h"
#include "video_decoder.h"
#include "cvi_vdec_vo.h"

class AvPacketData : public Poco::Notification {
  public:
    AvPacketData(const AVPacket *data) { _data = av_packet_clone(data); }
    ~AvPacketData() { av_packet_free(&_data); }
    AVPacket *data() const { return _data; }

  private:
    AVPacket *_data;
};

class MediaPlayer : public Poco::Runnable, public DecoderCallback {
  public:
    MediaPlayer();
    ~MediaPlayer();


    void init_cviSdk();
    //Poco::Runnable
    //Do whatever the thread needs to do.
    void run();

    //DecodeCallback
    void onVideoData(const AVPacket *pkt) override;
    void onStop() { _running = false; }

  private:
    bool _running = true;
    CviVdecVO _vdecVo;
    Poco::NotificationQueue _video_queue;
};