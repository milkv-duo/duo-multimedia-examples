#include "video_decoder.h"

#include "log.h"

std::string ffmpeg_error(int errcode) {
    char errbuf[128];

    if (errcode < 0) {
        av_strerror(errcode, errbuf, sizeof(errbuf));
        return std::string(errbuf);
    }

    return "No error";
}

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

    const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
    if (!bsf) {
        ERROR("h264_mp4toannexb bitstream filter not found!");
        return -1;
    }

    if (av_bsf_alloc(bsf, &_bsf_ctx) < 0) {
        ERROR("Could not allocate h264_mp4toannexb bitstream filter!");
        return -1;
    }

    if (avcodec_parameters_copy(
            _bsf_ctx->par_in, _fmt_ctx->streams[_video_stream_idx]->codecpar) <
        0) {
        ERROR("Failed to copy codec parameters!");
        return -1;
    }


    if (av_bsf_init(_bsf_ctx) < 0) {
        ERROR("Could not initialize h264_mp4toannexb bitstream filter!");
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

        if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
            // todo
        } else {
            // todo
        }

        // av_frame_unref(frame);
    }

    av_frame_free(&frame);

    return ret;
}

void VideoDecoder::h264_mp4toannexb(AVPacket *pkt) {

    av_bsf_send_packet(_bsf_ctx, pkt);

    while (true) {
        AVPacket out_pkt;
        pkt = &out_pkt;
        int ret = av_bsf_receive_packet(_bsf_ctx, pkt);
        if (ret == 0) {
            av_packet_rescale_ts(pkt, _video_stream->time_base, AV_TIME_BASE_Q);
            if (pkt->duration <= 0 && _last_pts_time != 0) {
                pkt->duration = pkt->pts - _last_pts_time;
            }
            _last_pts_time = pkt->pts;
            _callback->onVideoData(pkt);

        } else {
            break;
        }
    }
}

void VideoDecoder::run() {
    AVPacket *pkt = av_packet_alloc();
    int ret = 0;
    while ((ret = av_read_frame(_fmt_ctx, pkt)) >= 0) {
        if (pkt->stream_index == _video_stream_idx) {
            h264_mp4toannexb(pkt);
        } else if (pkt->stream_index == _audio_stream_idx) {
        }
        av_packet_unref(pkt);
    }

    INFO("Video demuxing is complete: {0}", ffmpeg_error(ret));
    AVPacket out_pkt;

    out_pkt.pts = -1;
    _callback->onVideoData(&out_pkt);

    av_packet_free(&pkt);
}
