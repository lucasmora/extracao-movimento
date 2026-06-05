#include "processor.h"

Processor::Processor(QObject* parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Processor::onTick);
}

Processor::~Processor() {
    pause();
    m_cap.release();
}

bool Processor::openVideo() {
    m_cap.release();
    m_buffer.clear();
    m_previewRunning = false;

    if (!m_cap.open(m_inputPath.toStdString()))
        return false;

    m_fps = m_cap.get(cv::CAP_PROP_FPS);
    m_totalFrames = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_COUNT));
    m_videoOpen = true;

    setPosition(0);
    return true;
}

void Processor::fillBufferUpTo(int frame) {
    m_buffer.clear();
    int start = std::max(0, frame - m_delayFrames);
    m_cap.set(cv::CAP_PROP_POS_FRAMES, start);

    for (int i = start; i < frame; i++) {
        cv::Mat f;
        if (!m_cap.read(f)) break;
        m_buffer.push_back(f.clone());
    }
}

void Processor::setPosition(int frame) {
    if (!m_videoOpen) return;

    bool wasRunning = m_previewRunning;
    if (wasRunning) pause();

    fillBufferUpTo(frame);

    cv::Mat current;
    if (!m_cap.read(current)) return;

    cv::Mat processed = current.clone();
    if (static_cast<int>(m_buffer.size()) == m_delayFrames) {
        cv::Mat inverted;
        cv::bitwise_not(m_buffer.front(), inverted);
        cv::addWeighted(current, 0.5, inverted, 0.5, 0, processed);
        m_buffer.pop_front();
    }
    m_buffer.push_back(current.clone());

    emit frameProcessed(matToQImage(current), matToQImage(processed),
                        frame + 1, m_totalFrames);

    if (wasRunning) play();
}

void Processor::play() {
    if (!m_videoOpen) return;
    if (m_previewRunning) return;

    double interval = (m_fps > 0) ? 1000.0 / m_fps : 33.0;
    m_timer->start(static_cast<int>(interval));
    m_previewRunning = true;
}

void Processor::pause() {
    m_timer->stop();
    m_previewRunning = false;
}

void Processor::stop() {
    pause();
    setPosition(0);
}

void Processor::onTick() {
    if (!m_videoOpen) { pause(); return; }

    cv::Mat frame;
    if (!m_cap.read(frame)) {
        pause();
        emit previewFinished();
        return;
    }

    cv::Mat processed = frame.clone();
    if (static_cast<int>(m_buffer.size()) == m_delayFrames) {
        cv::Mat inverted;
        cv::bitwise_not(m_buffer.front(), inverted);
        cv::addWeighted(frame, 0.5, inverted, 0.5, 0, processed);
        m_buffer.pop_front();
    }
    m_buffer.push_back(frame.clone());

    int pos = static_cast<int>(m_cap.get(cv::CAP_PROP_POS_FRAMES));
    emit frameProcessed(matToQImage(frame), matToQImage(processed),
                        pos, m_totalFrames);

    if (m_totalFrames > 0)
        emit progressChanged((pos * 100) / m_totalFrames);
}

QImage Processor::matToQImage(const cv::Mat& mat) {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.cols * 3, QImage::Format_RGB888).copy();
}

void Processor::saveVideo(const QString& outputPath) {
    cv::VideoCapture cap(m_inputPath.toStdString());
    if (!cap.isOpened()) {
        emit errorOccurred("Erro ao abrir video para salvar");
        return;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int w = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int total = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    cv::VideoWriter out(outputPath.toStdString(), fourcc, fps, cv::Size(w, h));

    std::deque<cv::Mat> buf;
    cv::Mat frame, processed, inverted;
    int count = 0;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        processed = frame.clone();
        if (static_cast<int>(buf.size()) == m_delayFrames) {
            cv::bitwise_not(buf.front(), inverted);
            cv::addWeighted(frame, 0.5, inverted, 0.5, 0, processed);
            buf.pop_front();
        }
        buf.push_back(frame.clone());
        out.write(processed);

        ++count;
        if (total > 0)
            emit progressChanged((count * 100) / total);
    }

    cap.release();
    out.release();
    emit fileSaved(outputPath);
}
