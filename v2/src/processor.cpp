#include "processor.h"
#include <deque>

Processor::Processor(QObject* parent) : QObject(parent) {}
Processor::~Processor() = default;

QImage Processor::matToQImage(const cv::Mat& mat) {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, rgb.cols * 3, QImage::Format_RGB888);
    return img.copy();
}

void Processor::process() {
    m_cancel = false;

    cv::VideoCapture cap(m_inputPath.toStdString());
    if (!cap.isOpened()) {
        emit errorOccurred("Erro ao abrir o video: " + m_inputPath);
        emit finished();
        return;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int total = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    std::string inputStr = m_inputPath.toStdString();
    size_t dot = inputStr.rfind('.');
    std::string base = (dot != std::string::npos) ? inputStr.substr(0, dot) : inputStr;
    std::string outPath = base + "_" + std::to_string(m_delayFrames) + "f.mp4";

    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    cv::VideoWriter out(outPath, fourcc, fps, cv::Size(width, height));

    std::deque<cv::Mat> buffer;
    cv::Mat frame, processed, inverted;
    int frameCount = 0;

    while (true) {
        if (m_cancel) break;

        cap >> frame;
        if (frame.empty()) break;

        processed = frame.clone();

        if (static_cast<int>(buffer.size()) == m_delayFrames) {
            cv::bitwise_not(buffer.front(), inverted);
            cv::addWeighted(frame, 0.5, inverted, 0.5, 0, processed);
            buffer.pop_front();
        }

        buffer.push_back(frame.clone());
        out.write(processed);

        ++frameCount;

        emit frameProcessed(matToQImage(frame), matToQImage(processed), frameCount, total);

        if (total > 0)
            emit progressChanged((frameCount * 100) / total);
    }

    cap.release();
    out.release();

    if (!m_cancel)
        emit progressChanged(100);

    emit finished();
}
