#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QTimer>
#include <deque>
#include <opencv2/opencv.hpp>

class Processor : public QObject {
    Q_OBJECT
public:
    explicit Processor(QObject* parent = nullptr);
    ~Processor() override;

    void setInputPath(const QString& path) { m_inputPath = path; }
    void setDelayFrames(int delay) { m_delayFrames = delay; }
    void setOutputName(const QString& name) { m_outputName = name; }

    bool openVideo();
    void setPosition(int frame);
    void play();
    void pause();
    void stop();
    bool isPreviewRunning() const { return m_previewRunning; }

    int totalFrames() const { return m_totalFrames; }

public slots:
    void saveVideo(const QString& outputPath);

signals:
    void frameProcessed(QImage original, QImage processed, int current, int total);
    void progressChanged(int percent);
    void fileSaved(const QString& path);
    void previewFinished();
    void errorOccurred(const QString& msg);

private slots:
    void onTick();

private:
    void fillBufferUpTo(int frame);
    QImage matToQImage(const cv::Mat& mat);

    QString m_inputPath;
    QString m_outputName;
    int m_delayFrames = 5;

    cv::VideoCapture m_cap;
    std::deque<cv::Mat> m_buffer;
    QTimer* m_timer = nullptr;
    int m_totalFrames = 0;
    double m_fps = 0.0;
    bool m_previewRunning = false;
    bool m_videoOpen = false;
};
