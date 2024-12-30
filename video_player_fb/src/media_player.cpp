#include "media_player.h"

MediaPlayer::MediaPlayer() {}

MediaPlayer::~MediaPlayer() {}

int MediaPlayer::open_device(std::string input_file) {
    return _fbDisplay.open_device(input_file);
}

void MediaPlayer::onVideoData(AVFrame *frame) {
    if (frame == NULL) {
        return;
    }

    _video_queue.enqueueNotification(new AvFrameData(frame));
}

void MediaPlayer::run() {
    Poco::AutoPtr<Poco::Notification> pNf;
    while (_running) {
        pNf = _video_queue.waitDequeueNotification();
        AvFrameData *frame = dynamic_cast<AvFrameData *>(pNf.get());
        if (frame) {
            _fbDisplay.show_image(frame->data());
        }
    }
}
