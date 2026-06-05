#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QTimer>
#include <opencv2/opencv.hpp>

class VideoPlayer : public QObject {
    Q_OBJECT
public:
    explicit VideoPlayer(QObject* parent = nullptr);
    ~VideoPlayer() override;

    bool open(const QString& path);
    void play();
    void pause();
    void togglePlayPause();
    void stop();
    void seekBySlider(int value, int max);
    void seekToFrame(int frame);

    bool isPlaying() const { return m_playing; }
    int totalFrames() const { return m_totalFrames; }
    double fps() const { return m_fps; }

signals:
    void frameReady(const QImage& image);
    void positionChanged(int currentFrame, int totalFrames);
    void playingChanged(bool playing);

private slots:
    void onTimerTick();

private:
    QImage readCurrentFrame();

    cv::VideoCapture m_cap;
    QTimer* m_timer;
    QString m_path;
    int m_totalFrames = 0;
    double m_fps = 0.0;
    bool m_playing = false;
};
