#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <atomic>
#include <opencv2/opencv.hpp>

class Processor : public QObject {
    Q_OBJECT
public:
    explicit Processor(QObject* parent = nullptr);
    ~Processor() override;

    void setInputPath(const QString& path) { m_inputPath = path; }
    void setDelayFrames(int delay) { m_delayFrames = delay; }
    void cancel() { m_cancel = true; }

public slots:
    void process();

signals:
    void frameProcessed(QImage original, QImage processed, int current, int total);
    void progressChanged(int percent);
    void finished();
    void errorOccurred(const QString& message);

private:
    static QImage matToQImage(const cv::Mat& mat);

    QString m_inputPath;
    int m_delayFrames = 5;
    std::atomic<bool> m_cancel{false};
};
