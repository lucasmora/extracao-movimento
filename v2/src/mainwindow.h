#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QSlider>
#include <QThread>
#include "processor.h"
#include "player.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSelectVideo();
    void onProcessToggle();
    void onFrameProcessed(QImage original, QImage processed, int current, int total);
    void onProgressChanged(int percent);
    void onProcessingFinished();
    void onError(const QString& msg);

    void onPlayPause();
    void onStop();
    void onSliderPressed();
    void onSliderReleased();
    void onSliderMoved(int value);
    void onPlayerFrame(const QImage& image);
    void onPlayerPos(int current, int total);

private:
    void setupUI();
    QString formatTime(int frame) const;

    QPushButton*  m_btnSelect;
    QPushButton*  m_btnProcess;
    QSpinBox*     m_spinDelay;
    QProgressBar* m_progress;
    QLabel*       m_statusLabel;
    QLabel*       m_originalView;
    QLabel*       m_processedView;

    VideoPlayer*  m_player;
    QPushButton*  m_btnPlay;
    QPushButton*  m_btnStop;
    QSlider*      m_slider;
    QLabel*       m_timeLabel;
    bool          m_seeking = false;

    QThread*   m_workerThread = nullptr;
    Processor* m_processor    = nullptr;
    QString    m_inputPath;
    bool       m_processing   = false;
};
