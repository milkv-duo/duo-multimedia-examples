#include "video_record.h"

std::string ffmpeg_error(int errcode) {
    char errbuf[128];

    if (errcode < 0) {
        av_strerror(errcode, errbuf, sizeof(errbuf));
        return std::string(errbuf);
    }

    return "No error";
}

int VideoRecord::add_video_stream() {

    _video_stream = avformat_new_stream(_fmt_ctx, NULL);
    if (!_video_stream) {
        ERROR("Could not allocate stream.");
        return -1;
    }

    _video_stream->id = _fmt_ctx->nb_streams - 1;
    _video_stream->time_base = (AVRational){1, _videoInfo.frame};

     // 设置编码器
    AVCodecParameters *codecParameters = _video_stream->codecpar;
    codecParameters->codec_id = AV_CODEC_ID_H264;
    codecParameters->codec_type = AVMEDIA_TYPE_VIDEO;
    codecParameters->width = _videoInfo.width;  // 设定视频宽度
    codecParameters->height = _videoInfo.height; // 设定视频高度
    codecParameters->format = AV_PIX_FMT_YUV420P;
    codecParameters->bit_rate = _videoInfo.rate * 1000;


    return 0;
}

VideoRecord::VideoRecord() {}

VideoRecord::~VideoRecord() {
    if (_encoder) {
        delete _encoder;
    }

    if (_fmt_ctx) {
        avio_closep(&_fmt_ctx->pb);
        avformat_free_context(_fmt_ctx);
    }
}

int32_t VideoRecord::initVideoEncoder() {
    if (_encoder == NULL) {
        _encoder = new CviVideoEncoder();
        int32_t ret = _encoder->init_cviSdk("264", &_videoInfo);

        if (ret != 0) {
            ERROR("init_cfg failed! ret = {0}", ret);
            return ret;
        }
    }
    _encoder->venc_start();
    return 0;
}

int32_t VideoRecord::initFfmpeg(RecordConf *pConf) {
    int ret = 0;
    std::string format = "h264";
    if (pConf->_format == "flv") {
        format = "flv";
    } else if (pConf->_format == "mp4") {
        format = "mp4";
    }

    _out_fmt = av_guess_format(format.c_str(), pConf->_out_file.c_str(), NULL);
    if (!_out_fmt) {
        ERROR("Could not deduce output format from file extension!");
        return -1;
    }

    avformat_alloc_output_context2(&_fmt_ctx, _out_fmt, NULL,
                                   pConf->_out_file.c_str());
    if (!_fmt_ctx) {
        ERROR("Failed to create output context!");
        return -1;
    }

    _record_time = pConf->_record_time * AV_TIME_BASE;

    _video_stream = new AVStream;

    if (_out_fmt->video_codec == AV_CODEC_ID_NONE) {
        ERROR("Failed to create video codec!");
        return -1;
    }

    ret = add_video_stream();
    if (ret != 0) {
        ERROR("Failed to add stream!");
        return -1;
    }


    if (!(_fmt_ctx->flags & AVFMT_NOFILE)) {
        ret =
            avio_open(&_fmt_ctx->pb, pConf->_out_file.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            ERROR("Could not open '{0}': {1}", pConf->_out_file,
                  ffmpeg_error(ret));
            return -1;
        }
    }

    // 写文件头
    ret = avformat_write_header(_fmt_ctx, NULL);
    if (ret < 0) {
        ERROR("Could not write header: {0}", ffmpeg_error(ret));
        return -1;
    }

    return 0;
}

void VideoRecord::onVideoData(AVPacket *pkt) {
    if (_first_pts_time == 0) {
        _first_pts_time = pkt->pts;
    }

    pkt->pts = pkt->dts = pkt->pts - _first_pts_time;
    pkt->duration = pkt->pts - _last_pts_time;
    pkt->stream_index = _video_stream->index;
    _last_pts_time = pkt->pts;

    if (pkt->pts > _record_time) {
        INFO("record enough time {0} seconds, stop record!",
             _record_time / AV_TIME_BASE);
        stop();
        return;
    }

    av_packet_rescale_ts(pkt, AV_TIME_BASE_Q, _video_stream->time_base);

    if (av_interleaved_write_frame(_fmt_ctx, pkt) < 0) {
        ERROR("Error while writing frame!");
        return;
    }
}

void VideoRecord::run() {
    int ret = 0;
    while (_running) {
        ret = _encoder->getStream(this);
        if (ret != 0) {
            WARN("get avpacket failed! ret = {0}", ret);
            Poco::Thread::sleep(100);
            continue;
        }
    }

    _video_stream->duration =
        av_rescale_q(_last_pts_time - _first_pts_time, AV_TIME_BASE_Q,
                     _video_stream->time_base);
    av_write_trailer(_fmt_ctx);
}
