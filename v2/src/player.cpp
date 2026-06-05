#include "player.h"

VideoPlayer::VideoPlayer(QObject* parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VideoPlayer::onTimerTick);
}

VideoPlayer::~VideoPlayer() {
    stop();
    m_cap.release();
}

bool VideoPlayer::open(const QString& path) {
    m_cap.release();
    m_path = path;
    if (!m_cap.open(path.toStdString(), cv::CAP_ANY, {
            cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_D3D11
        }))
        return false;

    m_fps = m_cap.get(cv::CAP_PROP_FPS);
    m_totalFrames = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_COUNT));
    m_playing = false;
    m_hwAccel = (static_cast<int>(m_cap.get(cv::CAP_PROP_HW_ACCELERATION))
                 == cv::VIDEO_ACCELERATION_D3D11);

    QImage first = readCurrentFrame();
    if (!first.isNull())
        emit frameReady(first);

    emit positionChanged(0, m_totalFrames);
    return true;
}

void VideoPlayer::play() {
    if (!m_cap.isOpened() || m_totalFrames <= 0) return;

    double interval = (m_fps > 0) ? 1000.0 / m_fps : 33.0;
    m_timer->start(static_cast<int>(interval));
    m_playing = true;
    emit playingChanged(true);
}

void VideoPlayer::pause() {
    m_timer->stop();
    m_playing = false;
    emit playingChanged(false);
}

void VideoPlayer::togglePlayPause() {
    if (m_playing) pause();
    else play();
}

void VideoPlayer::stop() {
    pause();
    seekToFrame(0);
}

void VideoPlayer::seekToFrame(int frame) {
    if (!m_cap.isOpened()) return;

    if (frame < 0) frame = 0;
    if (frame >= m_totalFrames) frame = m_totalFrames - 1;

    m_cap.set(cv::CAP_PROP_POS_FRAMES, frame);
    QImage img = readCurrentFrame();
    if (!img.isNull())
        emit frameReady(img);

    emit positionChanged(frame, m_totalFrames);
}

void VideoPlayer::seekBySlider(int value, int max) {
    if (!m_cap.isOpened() || max <= 0) return;
    int frame = (value * m_totalFrames) / max;
    seekToFrame(frame);
}

void VideoPlayer::onTimerTick() {
    if (!m_cap.isOpened()) {
        pause();
        return;
    }

    cv::Mat frame;
    if (!m_cap.read(frame)) {
        pause();
        seekToFrame(0);
        return;
    }

    int pos = static_cast<int>(m_cap.get(cv::CAP_PROP_POS_FRAMES));
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, rgb.cols * 3, QImage::Format_RGB888);

    emit frameReady(img.copy());
    emit positionChanged(pos, m_totalFrames);
}

QImage VideoPlayer::readCurrentFrame() {
    cv::Mat frame;
    if (!m_cap.read(frame)) return QImage();

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.cols * 3, QImage::Format_RGB888).copy();
}
