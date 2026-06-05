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
    void onSave();

    void onPlayPause();
    void onStop();
    void onSliderPressed();
    void onSliderReleased();
    void onSliderMoved(int value);

    void onPlayerFrame(const QImage& image);
    void onPlayerPos(int current, int total);

    void onPreviewFrame(QImage original, QImage processed, int current, int total);
    void onProgressChanged(int percent);
    void onFileSaved(const QString& path);
    void onPreviewFinished();
    void onSaveError(const QString& msg);

private:
    void setupUI();
    QString formatTime(int frame, double fps) const;

    QPushButton*  m_btnSelect;
    QPushButton*  m_btnSave;
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
    QLabel*       m_hwIndicator;
    bool          m_seeking = false;

    Processor*    m_previewProcessor;
    QString       m_inputPath;
    bool          m_saving = false;

    QThread*      m_saveThread = nullptr;
    Processor*    m_saveProcessor = nullptr;
    int           m_currentFrame = 0;
    bool          m_previewFinished = false;
};
