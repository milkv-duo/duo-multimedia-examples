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
#include "fb_display.h"

class AvFrameData : public Poco::Notification {
  public:
    AvFrameData(AVFrame *data) { _data = av_frame_clone(data); }
    ~AvFrameData() { av_frame_free(&_data); }
    AVFrame *data() const { return _data; }

  private:
    AVFrame *_data;
};

class MediaPlayer : public Poco::Runnable, public DecoderCallback {
  public:
    MediaPlayer();
    ~MediaPlayer();

    int open_device(std::string device);

    //Poco::Runnable
    //Do whatever the thread needs to do.
    void run();

    //DecodeCallback
    void onVideoData(AVFrame *frame) override;
    void onStop() { _running = false; }

  private:
    bool _running = true;
    FBDisplay _fbDisplay;
    Poco::NotificationQueue _video_queue;
};