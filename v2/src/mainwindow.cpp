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
    setupUI();
    resize(1100, 720);
    setWindowTitle("Extrator de Movimento");
}

MainWindow::~MainWindow() {
    if (m_workerThread && m_workerThread->isRunning()) {
        m_processor->cancel();
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

QString MainWindow::formatTime(int frame) const {
    int secs = static_cast<int>(frame / m_player->fps());
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

    m_btnSelect = new QPushButton("Selecionar Video...");
    m_btnSelect->setMinimumWidth(160);

    m_spinDelay = new QSpinBox();
    m_spinDelay->setRange(1, 60);
    m_spinDelay->setValue(5);
    m_spinDelay->setPrefix("Delay: ");
    m_spinDelay->setSuffix(" frames");
    m_spinDelay->setFixedWidth(150);

    m_btnProcess = new QPushButton("Processar");
    m_btnProcess->setEnabled(false);
    m_btnProcess->setMinimumWidth(120);
    m_btnProcess->setStyleSheet(
        "QPushButton { font-weight: bold; padding: 6px 20px; }"
    );

    topRow->addWidget(m_btnSelect);
    topRow->addWidget(m_spinDelay);
    topRow->addWidget(m_btnProcess);
    topRow->addStretch();

    // Progress
    m_progress = new QProgressBar();
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setTextVisible(true);
    m_progress->setFixedHeight(24);

    m_statusLabel = new QLabel("Selecione um video para comecar");
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

    // Insert controls into original container's layout
    auto* origLayout = qobject_cast<QVBoxLayout*>(origContainer->layout());
    origLayout->addWidget(ctrlFrame);

    previewRow->addWidget(origContainer, 1);
    previewRow->addWidget(procContainer, 1);

    // Assemble main
    mainLayout->addLayout(topRow);
    mainLayout->addWidget(m_progress);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addLayout(previewRow, 1);

    setCentralWidget(central);

    // Connections
    connect(m_btnSelect, &QPushButton::clicked, this, &MainWindow::onSelectVideo);
    connect(m_btnProcess, &QPushButton::clicked, this, &MainWindow::onProcessToggle);

    // Player connections
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_slider, &QSlider::sliderPressed, this, &MainWindow::onSliderPressed);
    connect(m_slider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
    connect(m_slider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(m_player, &VideoPlayer::frameReady, this, &MainWindow::onPlayerFrame);
    connect(m_player, &VideoPlayer::positionChanged, this, &MainWindow::onPlayerPos);

    // Worker thread
    m_workerThread = new QThread(this);
    m_processor = new Processor();
    m_processor->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_processor, &Processor::process);
    connect(m_processor, &Processor::finished, this, &MainWindow::onProcessingFinished);
    connect(m_processor, &Processor::finished, m_workerThread, &QThread::quit);
    connect(m_processor, &Processor::frameProcessed, this, &MainWindow::onFrameProcessed);
    connect(m_processor, &Processor::progressChanged, this, &MainWindow::onProgressChanged);
    connect(m_processor, &Processor::errorOccurred, this, &MainWindow::onError);
}

void MainWindow::onSelectVideo() {
    QString path = QFileDialog::getOpenFileName(
        this, "Selecionar Video", QString(),
        "Videos (*.mp4 *.avi *.mov *.mkv);;Todos (*.*)");
    if (path.isEmpty()) return;

    m_inputPath = path;
    m_btnSelect->setText(QFileInfo(path).fileName());
    m_btnProcess->setEnabled(true);
    m_statusLabel->setText("Video selecionado: " + QFileInfo(path).fileName());
    m_progress->setValue(0);
    m_processedView->setText("(processado)");
    m_processedView->setPixmap(QPixmap());

    m_player->open(path);
    m_btnPlay->setEnabled(true);
    m_btnStop->setEnabled(true);
    m_slider->setEnabled(true);
}

void MainWindow::onPlayPause() {
    m_player->togglePlayPause();
}

void MainWindow::onStop() {
    m_player->stop();
}

void MainWindow::onSliderPressed() {
    if (m_player->isPlaying()) {
        m_player->pause();
        m_seeking = true;
    }
}

void MainWindow::onSliderReleased() {
    m_seeking = false;
}

void MainWindow::onSliderMoved(int value) {
    m_player->seekBySlider(value, m_slider->maximum());
}

void MainWindow::onPlayerFrame(const QImage& image) {
    QSize sz = m_originalView->size();
    m_originalView->setPixmap(
        QPixmap::fromImage(image).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::onPlayerPos(int current, int total) {
    if (!m_seeking) {
        m_slider->blockSignals(true);
        m_slider->setValue((current * m_slider->maximum()) / qMax(total, 1));
        m_slider->blockSignals(false);
    }
    m_timeLabel->setText(formatTime(current) + " / " + formatTime(total));
}

void MainWindow::onProcessToggle() {
    if (m_processing) {
        m_processor->cancel();
        m_statusLabel->setText("Cancelando...");
        return;
    }

    if (m_inputPath.isEmpty()) return;

    m_processing = true;
    m_btnProcess->setText("Cancelar");
    m_btnSelect->setEnabled(false);
    m_spinDelay->setEnabled(false);
    m_progress->setValue(0);
    m_processedView->setText("(processando...)");

    m_processor->setInputPath(m_inputPath);
    m_processor->setDelayFrames(m_spinDelay->value());

    m_workerThread->start();
}

void MainWindow::onFrameProcessed(QImage original, QImage processed, int current, int total) {
    if (!original.isNull()) {
        QSize sz = m_originalView->size();
        m_originalView->setPixmap(
            QPixmap::fromImage(original).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (!processed.isNull()) {
        QSize sz = m_processedView->size();
        m_processedView->setPixmap(
            QPixmap::fromImage(processed).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_statusLabel->setText(QString("Processando: %1 / %2 frames").arg(current).arg(total));
    QApplication::processEvents();
}

void MainWindow::onProgressChanged(int percent) {
    m_progress->setValue(percent);
}

void MainWindow::onProcessingFinished() {
    m_processing = false;
    m_btnProcess->setText("Processar");
    m_btnSelect->setEnabled(true);
    m_spinDelay->setEnabled(true);
    m_statusLabel->setText("Processamento concluido!");
}

void MainWindow::onError(const QString& msg) {
    QMessageBox::warning(this, "Erro", msg);
    m_statusLabel->setText("Erro: " + msg);
}
