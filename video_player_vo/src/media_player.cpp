#include "media_player.h"

MediaPlayer::MediaPlayer() {}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::init_cviSdk() { _vdecVo.init_cviSdk(); }

void MediaPlayer::onVideoData(const AVPacket *pkt) {
    if (pkt == NULL) {
        return;
    }

    _video_queue.enqueueNotification(new AvPacketData(pkt));
}

void MediaPlayer::run() {
    Poco::Clock clock;
    Poco::AutoPtr<Poco::Notification> pNf;
    while (_running) {
        pNf = _video_queue.waitDequeueNotification();
        AvPacketData *pkt = dynamic_cast<AvPacketData *>(pNf.get());
        if (!pkt)
            continue;

        if (pkt->data()->pts == -1) {
            break;
        }

        _vdecVo.sendStreamToVenc(pkt->data());
        _vdecVo.run();
        usleep(pkt->data()->duration);
    }
    Poco::Int64 currentTicks = clock.elapsed();
    INFO("time duration: {0} seconds", currentTicks / (1000 * 1000));
}
