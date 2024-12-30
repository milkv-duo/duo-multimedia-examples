#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

#include "Poco/Runnable.h"
#include "Poco/Thread.h"

#include <string>

class DecoderCallback {
  public:
    virtual void onVideoData(AVFrame *frame) {};
    virtual void onAudioData(AVFrame *frame) {};
    virtual void onStop() {};
};

class VideoDecoder : public Poco::Runnable {
  public:
    VideoDecoder(DecoderCallback *callback) : _callback(callback) {}
    ~VideoDecoder();

    int open_file(std::string input_file);

    //Poco::Runnable
    //Do whatever the thread needs to do.
    void run();

  private:
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                           AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);

  private:
    DecoderCallback *_callback = NULL;

    AVFormatContext *_fmt_ctx = NULL;
    int _video_stream_idx = -1, _audio_stream_idx = -1;
    AVCodecContext *_video_dec_ctx = NULL, *_audio_dec_ctx = NULL;
    AVStream *_video_stream = NULL, *_audio_stream = NULL;
};
