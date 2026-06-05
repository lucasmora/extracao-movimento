#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Extrator de Movimento");
    app.setStyle(QStyleFactory::create("Fusion"));

    app.setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #0d1117;
            color: #e6edf3;
        }
        QPushButton {
            background-color: #21262d;
            color: #e6edf3;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 6px 16px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #30363d;
            border-color: #58a6ff;
        }
        QPushButton:pressed {
            background-color: #161b22;
        }
        QPushButton:disabled {
            background-color: #161b22;
            color: #484f58;
            border-color: #21262d;
        }
        QSpinBox {
            background-color: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 4px 6px;
            color: #e6edf3;
            font-size: 13px;
        }
        QSpinBox:focus {
            border-color: #58a6ff;
        }
        QSpinBox::up-button, QSpinBox::down-button {
            background-color: #21262d;
            border: none;
            width: 18px;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background-color: #30363d;
        }
        QProgressBar {
            background-color: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            text-align: center;
            color: #e6edf3;
            font-size: 12px;
        }
        QProgressBar::chunk {
            background-color: #58a6ff;
            border-radius: 5px;
        }
        QSlider::groove:horizontal {
            background: #21262d;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #58a6ff;
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
        QSlider::handle:horizontal:hover {
            background: #79c0ff;
        }
        QSlider::sub-page:horizontal {
            background: #58a6ff;
            border-radius: 3px;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
