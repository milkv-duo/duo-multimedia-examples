#include "video_decoder.h"

#include "log.h"

VideoDecoder::~VideoDecoder() {
    if (_video_dec_ctx) {
        avcodec_free_context(&_video_dec_ctx);
    }
    if (_audio_dec_ctx) {
        avcodec_free_context(&_audio_dec_ctx);
    }
    if (_fmt_ctx) {
        avformat_close_input(&_fmt_ctx);
    }
}

int VideoDecoder::open_file(std::string input_file) {

    if (avformat_open_input(&_fmt_ctx, input_file.c_str(), NULL, NULL) < 0) {
        ERROR("Could not open source file '{0}' !", input_file);
        return -1;
    }

    if (avformat_find_stream_info(_fmt_ctx, NULL) < 0) {
        ERROR("Could not find stream information!");
        return -1;
    }

    if (open_codec_context(&_video_stream_idx, &_video_dec_ctx, _fmt_ctx,
                           AVMEDIA_TYPE_VIDEO) >= 0) {
        _video_stream = _fmt_ctx->streams[_video_stream_idx];
    }

    if (open_codec_context(&_audio_stream_idx, &_audio_dec_ctx, _fmt_ctx,
                           AVMEDIA_TYPE_AUDIO) >= 0) {
        _audio_stream = _fmt_ctx->streams[_audio_stream_idx];
    }

    if (!_audio_stream && !_video_stream) {
        ERROR("Could not find audio or video stream in the input, aborting");
        return -1;
    }

    return 0;
}

int VideoDecoder::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                                     AVFormatContext *fmt_ctx,
                                     enum AVMediaType type) {
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        ERROR("Could not find {0} stream in input file!",
              av_get_media_type_string(type));
        return ret;
    }

    stream_index = ret;
    st = fmt_ctx->streams[stream_index];

    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!dec) {
        ERROR("Failed to find {0} codec!", av_get_media_type_string(type));
        return AVERROR(EINVAL);
    }

    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx) {
        ERROR("Failed to allocate the {0} codec context!",
              av_get_media_type_string(type));
        return AVERROR(ENOMEM);
    }

    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
        ERROR("Failed to copy {0} codec parameters to decoder context!",
              av_get_media_type_string(type));
        return ret;
    }

    if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
        ERROR("Failed to open {0} codec!", av_get_media_type_string(type));
        return ret;
    }
    *stream_idx = stream_index;

    return 0;
}

int VideoDecoder::decode_packet(AVCodecContext *dec, const AVPacket *pkt) {
    int ret = 0;
    AVFrame *frame = av_frame_alloc();

    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        ERROR("Error submitting a packet for decoding: {0}", errbuf);
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            char errbuf[128];
            av_strerror(ret, errbuf, sizeof(errbuf));
            ERROR("Error during decoding {0}", errbuf);
            return ret;
        }

        if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
            _callback->onVideoData(frame);
        else
            _callback->onAudioData(frame);

        // av_frame_unref(frame);
    }

    av_frame_free(&frame);

    return ret;
}

void VideoDecoder::run() {
    AVPacket *pkt = av_packet_alloc();
    while (av_read_frame(_fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == _video_stream_idx)
            decode_packet(_video_dec_ctx, pkt);
        else if (pkt->stream_index == _audio_stream_idx)
            decode_packet(_audio_dec_ctx, pkt);
        av_packet_unref(pkt);
    }

    if (_video_dec_ctx)
        decode_packet(_video_dec_ctx, NULL);
    if (_audio_dec_ctx)
        decode_packet(_audio_dec_ctx, NULL);

    _callback->onStop();
    av_packet_free(&pkt);
}
