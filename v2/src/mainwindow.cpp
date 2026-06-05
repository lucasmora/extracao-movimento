#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_player = new VideoPlayer(this);
    m_previewProcessor = new Processor(this);
    setupUI();
    resize(1100, 720);
    setWindowTitle("Extrator de Movimento");
}

MainWindow::~MainWindow() {
    m_previewProcessor->pause();
    if (m_saveThread && m_saveThread->isRunning()) {
        m_saveThread->quit();
        m_saveThread->wait();
    }
}

QString MainWindow::formatTime(int frame, double fps) const {
    if (fps <= 0) return "00:00";
    int secs = static_cast<int>(frame / fps);
    return QString("%1:%2").arg(secs / 60, 2, 10, QChar('0'))
                           .arg(secs % 60, 2, 10, QChar('0'));
}

void MainWindow::setupUI() {
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Top controls
    auto* topRow = new QHBoxLayout();

    m_btnSelect = new QPushButton("Selecionar vídeo...");
    m_btnSelect->setMinimumWidth(160);

    auto* delayLayout = new QHBoxLayout();
    auto* delayLabel = new QLabel("Delay:");
    delayLabel->setStyleSheet("color: #ccc; font-size: 13px;");
    m_spinDelay = new QSpinBox();
    m_spinDelay->setRange(1, 60);
    m_spinDelay->setValue(5);
    m_spinDelay->setFixedWidth(60);
    auto* framesLabel = new QLabel("frames");
    framesLabel->setStyleSheet("color: #ccc; font-size: 13px;");
    delayLayout->addWidget(delayLabel);
    delayLayout->addWidget(m_spinDelay);
    delayLayout->addWidget(framesLabel);

    m_btnSave = new QPushButton("Salvar");
    m_btnSave->setEnabled(false);
    m_btnSave->setMinimumWidth(120);
    m_btnSave->setStyleSheet(
        "QPushButton { font-weight: bold; padding: 6px 20px; }"
    );

    topRow->addWidget(m_btnSelect);
    topRow->addLayout(delayLayout);
    topRow->addWidget(m_btnSave);
    topRow->addStretch();

    // Progress
    m_progress = new QProgressBar();
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setTextVisible(true);
    m_progress->setFixedHeight(24);

    m_statusLabel = new QLabel("Selecione um vídeo para começar");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Preview area
    auto* previewRow = new QHBoxLayout();

    auto makeView = [](const QString& title) -> QWidget* {
        auto* frame = new QFrame();
        frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        frame->setStyleSheet(
            "QFrame { background: #1e1e1e; border: 1px solid #444; border-radius: 6px; }");
        auto* lay = new QVBoxLayout(frame);
        lay->setSpacing(6);

        auto* lbl = new QLabel(title);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: #ccc; font-weight: bold; font-size: 13px; "
                           "background: transparent; border: none;");

        auto* view = new QLabel();
        view->setObjectName("view");
        view->setAlignment(Qt::AlignCenter);
        view->setMinimumSize(400, 280);
        view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        view->setText("(aguardando)");
        view->setStyleSheet("color: #666; font-size: 16px; "
                            "background: transparent; border: none;");

        lay->addWidget(lbl);
        lay->addWidget(view, 1);

        auto* container = new QWidget();
        auto* box = new QVBoxLayout(container);
        box->setContentsMargins(0, 0, 0, 0);
        box->addWidget(frame, 1);
        return container;
    };

    QWidget* origContainer = makeView("Original");
    m_originalView = origContainer->findChild<QLabel*>("view");

    QWidget* procContainer = makeView("Processado");
    m_processedView = procContainer->findChild<QLabel*>("view");

    // Player controls
    auto* ctrlFrame = new QFrame();
    ctrlFrame->setStyleSheet(
        "QFrame { background: #2a2a2a; border: 1px solid #444; border-radius: 4px; }");
    auto* ctrlBox = new QHBoxLayout(ctrlFrame);
    ctrlBox->setContentsMargins(8, 4, 8, 4);

    m_btnPlay = new QPushButton(QString::fromUtf8("\u25B6"));
    m_btnPlay->setFixedSize(36, 28);
    m_btnPlay->setEnabled(false);
    m_btnPlay->setStyleSheet("font-size: 16px;");

    m_btnStop = new QPushButton(QString::fromUtf8("\u25A0"));
    m_btnStop->setFixedSize(36, 28);
    m_btnStop->setEnabled(false);
    m_btnStop->setStyleSheet("font-size: 14px;");

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 1000);
    m_slider->setValue(0);
    m_slider->setEnabled(false);

    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setStyleSheet("color: #aaa; font-size: 12px;");

    ctrlBox->addWidget(m_btnPlay);
    ctrlBox->addWidget(m_btnStop);
    ctrlBox->addWidget(m_slider, 1);
    ctrlBox->addWidget(m_timeLabel);

    previewRow->addWidget(origContainer, 1);
    previewRow->addWidget(procContainer, 1);

    mainLayout->addLayout(topRow);
    mainLayout->addWidget(m_progress);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addLayout(previewRow, 1);
    mainLayout->addWidget(ctrlFrame);

    setCentralWidget(central);

    // Button connections
    connect(m_btnSelect, &QPushButton::clicked, this, &MainWindow::onSelectVideo);
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::onSave);

    // Player connections
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_slider, &QSlider::sliderPressed, this, &MainWindow::onSliderPressed);
    connect(m_slider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
    connect(m_slider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(m_player, &VideoPlayer::frameReady, this, &MainWindow::onPlayerFrame);
    connect(m_player, &VideoPlayer::positionChanged, this, &MainWindow::onPlayerPos);

    // Preview processor connections
    connect(m_previewProcessor, &Processor::frameProcessed, this, &MainWindow::onPreviewFrame);
    connect(m_previewProcessor, &Processor::progressChanged, this, &MainWindow::onProgressChanged);
    connect(m_previewProcessor, &Processor::previewFinished, this, &MainWindow::onPreviewFinished);

    // Delay spinbox connection
    connect(m_spinDelay, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int delay) {
                m_previewProcessor->setDelayFrames(delay);
                m_previewProcessor->setPosition(m_currentFrame);
            });
}

void MainWindow::onSelectVideo() {
    QString path = QFileDialog::getOpenFileName(
        this, "Selecionar vídeo", QString(),
        "Vídeos (*.mp4 *.avi *.mov *.mkv);;Todos (*.*)");
    if (path.isEmpty()) return;

    m_inputPath = path;
    QString fileName = QFileInfo(path).fileName();
    m_btnSelect->setText(fileName);
    m_btnSave->setEnabled(true);
    m_statusLabel->setText("Vídeo selecionado: " + fileName);
    m_progress->setValue(0);

    m_processedView->setText("(processado)");
    m_processedView->setPixmap(QPixmap());

    m_previewProcessor->setInputPath(path);
    m_previewProcessor->setDelayFrames(m_spinDelay->value());
    m_previewProcessor->openVideo();

    m_player->open(path);
    m_btnPlay->setEnabled(true);
    m_btnStop->setEnabled(true);
    m_slider->setEnabled(true);
}

void MainWindow::onPlayPause() {
    if (!m_player->isPlaying()) {
        m_previewProcessor->play();
        m_player->play();
        m_btnPlay->setText(QString::fromUtf8("\u23F8"));
    } else {
        m_previewProcessor->pause();
        m_player->pause();
        m_btnPlay->setText(QString::fromUtf8("\u25B6"));
    }
}

void MainWindow::onStop() {
    m_previewProcessor->stop();
    m_player->stop();
    m_btnPlay->setText(QString::fromUtf8("\u25B6"));
}

void MainWindow::onSliderPressed() {
    if (m_player->isPlaying()) {
        m_previewProcessor->pause();
        m_player->pause();
        m_btnPlay->setText(QString::fromUtf8("\u25B6"));
    }
    m_seeking = true;
}

void MainWindow::onSliderReleased() {
    m_seeking = false;
}

void MainWindow::onSliderMoved(int value) {
    int totalP = m_player->totalFrames();
    int frame = (totalP > 0) ? (value * totalP) / m_slider->maximum() : 0;

    m_player->seekBySlider(value, m_slider->maximum());
    m_previewProcessor->setPosition(frame);
}

void MainWindow::onPlayerFrame(const QImage& image) {
    QSize sz = m_originalView->size();
    m_originalView->setPixmap(
        QPixmap::fromImage(image).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::onPlayerPos(int current, int total) {
    m_currentFrame = current;
    if (!m_seeking) {
        m_slider->blockSignals(true);
        m_slider->setValue((current * m_slider->maximum()) / qMax(total, 1));
        m_slider->blockSignals(false);
    }
    m_timeLabel->setText(formatTime(current, m_player->fps())
                         + " / " + formatTime(total, m_player->fps()));
}

void MainWindow::onPreviewFrame(QImage original, QImage processed, int current, int total) {
    Q_UNUSED(original)
    QSize sz = m_processedView->size();
    m_processedView->setPixmap(
        QPixmap::fromImage(processed).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_statusLabel->setText(QString("Processando: %1 / %2 frames").arg(current).arg(total));
}

void MainWindow::onProgressChanged(int percent) {
    m_progress->setValue(percent);
}

void MainWindow::onPreviewFinished() {
    m_statusLabel->setText("Processamento concluido!");
    m_btnPlay->setText(QString::fromUtf8("\u25B6"));
}

void MainWindow::onSave() {
    if (m_saving) return;

    QString base = QFileInfo(m_inputPath).completeBaseName();
    QString outPath = QFileInfo(m_inputPath).absolutePath()
                      + "/" + base + "_" + QString::number(m_spinDelay->value()) + "f.mp4";

    m_saving = true;
    m_btnSave->setEnabled(false);
    m_btnPlay->setEnabled(false);
    m_slider->setEnabled(false);
    m_statusLabel->setText("Salvando video...");

    m_saveThread = new QThread(this);
    m_saveProcessor = new Processor();
    m_saveProcessor->setInputPath(m_inputPath);
    m_saveProcessor->setDelayFrames(m_spinDelay->value());
    m_saveProcessor->moveToThread(m_saveThread);

    connect(m_saveThread, &QThread::started, m_saveProcessor, [this, outPath]() {
        m_saveProcessor->saveVideo(outPath);
    });
    connect(m_saveProcessor, &Processor::fileSaved, this, &MainWindow::onFileSaved);
    connect(m_saveProcessor, &Processor::progressChanged, this, &MainWindow::onProgressChanged);
    connect(m_saveProcessor, &Processor::errorOccurred, this, &MainWindow::onSaveError);
    connect(m_saveProcessor, &Processor::fileSaved, m_saveThread, &QThread::quit);
    connect(m_saveProcessor, &Processor::errorOccurred, m_saveThread, &QThread::quit);
    connect(m_saveThread, &QThread::finished, this, [this]() {
        m_saveThread->deleteLater();
        m_saveProcessor->deleteLater();
        m_saveThread = nullptr;
        m_saveProcessor = nullptr;
        m_saving = false;

        m_btnSave->setEnabled(true);
        m_btnPlay->setEnabled(true);
        m_slider->setEnabled(true);
        m_progress->setValue(100);
    });

    m_saveThread->start();
}

void MainWindow::onFileSaved(const QString& path) {
    QMessageBox::information(this, "Sucesso",
        "Video salvo em:\n" + path);
    m_statusLabel->setText("Vídeo salvo em: " + path);
}

void MainWindow::onSaveError(const QString& msg) {
    QMessageBox::warning(this, "Erro ao salvar", msg);
    m_statusLabel->setText("Erro ao salvar: " + msg);
}
