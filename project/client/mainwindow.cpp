#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isSending(false), sentCount(0), successCount(0), errorCount(0) {
    setupUI();
    
    udpSocket = new QUdpSocket(this);
    sendTimer = new QTimer(this);
    statusTimer = new QTimer(this);
    
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::sendData);
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::readResponse);
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateConnectionStatus);
    
    statusTimer->start(1000);
    
    logMessage("Клиент инициализирован. Введите параметры и нажмите 'Начать отправку'.");
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QGridLayout *mainLayout = new QGridLayout(centralWidget);
    
    QGroupBox *connectionGroup = new QGroupBox("Настройки соединения", this);
    QGridLayout *connectionLayout = new QGridLayout(connectionGroup);
    
    connectionLayout->addWidget(new QLabel("IP сервера:"), 0, 0);
    serverIpEdit = new QLineEdit("127.0.0.1", this);
    connectionLayout->addWidget(serverIpEdit, 0, 1);
    
    connectionLayout->addWidget(new QLabel("Порт сервера:"), 1, 0);
    serverPortSpin = new QSpinBox(this);
    serverPortSpin->setRange(1, 65535);
    serverPortSpin->setValue(8080);
    connectionLayout->addWidget(serverPortSpin, 1, 1);
    
    toggleButton = new QPushButton("Начать отправку", this);
    connectionLayout->addWidget(toggleButton, 2, 0, 1, 2);
    
    statusBar = new QProgressBar(this);
    statusBar->setTextVisible(false);
    statusBar->setMaximumHeight(10);
    connectionLayout->addWidget(statusBar, 3, 0, 1, 2);
    
    statusLabel = new QLabel("Не подключено", this);
    connectionLayout->addWidget(statusLabel, 4, 0, 1, 2);
    
    QGroupBox *paramsGroup = new QGroupBox("Параметры", this);
    QGridLayout *paramsLayout = new QGridLayout(paramsGroup);
    
    paramsLayout->addWidget(new QLabel("X:"), 0, 0);
    xSpin = new QSpinBox(this);
    xSpin->setRange(0, 63);
    xSpin->setValue(0);
    paramsLayout->addWidget(xSpin, 0, 1);
    
    paramsLayout->addWidget(new QLabel("Y:"), 1, 0);
    ySpin = new QSpinBox(this);
    ySpin->setRange(-32, 31);
    ySpin->setValue(0);
    paramsLayout->addWidget(ySpin, 1, 1);
    
    paramsLayout->addWidget(new QLabel("V:"), 2, 0);
    vSpin = new QSpinBox(this);
    vSpin->setRange(0, 255);
    vSpin->setValue(0);
    paramsLayout->addWidget(vSpin, 2, 1);
    
    paramsLayout->addWidget(new QLabel("M:"), 3, 0);
    mSpin = new QSpinBox(this);
    mSpin->setRange(0, 3);
    mSpin->setValue(0);
    paramsLayout->addWidget(mSpin, 3, 1);
    
    paramsLayout->addWidget(new QLabel("S:"), 4, 0);
    sSpin = new QSpinBox(this);
    sSpin->setRange(0, 3);
    sSpin->setValue(0);
    paramsLayout->addWidget(sSpin, 4, 1);
    
    paramsLayout->addWidget(new QLabel("A:"), 0, 2);
    aSpin = new QDoubleSpinBox(this);
    aSpin->setDecimals(1);
    aSpin->setSingleStep(0.1);
    aSpin->setRange(-12.7, 12.8);
    aSpin->setValue(0.0);
    paramsLayout->addWidget(aSpin, 0, 3);
    
    paramsLayout->addWidget(new QLabel("P:"), 1, 2);
    pSpin = new QSpinBox(this);
    pSpin->setRange(0, 130);
    pSpin->setValue(0);
    paramsLayout->addWidget(pSpin, 1, 3);
    
    paramsLayout->addWidget(new QLabel("R:"), 2, 2);
    rSpin = new QSpinBox(this);
    rSpin->setRange(0, 65535);
    rSpin->setValue(0);
    paramsLayout->addWidget(rSpin, 2, 3);
    
    autoSendCheck = new QCheckBox("Автоматическая отправка", this);
    autoSendCheck->setChecked(true);
    paramsLayout->addWidget(autoSendCheck, 5, 0, 1, 4);
    
    QGroupBox *logGroup = new QGroupBox("Логи", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    logLayout->addWidget(logText);
    
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->addWidget(new QLabel("Отправлено: 0"));
    statsLayout->addWidget(new QLabel("Успешно: 0"));
    statsLayout->addWidget(new QLabel("Ошибки: 0"));
    logLayout->addLayout(statsLayout);
    
    mainLayout->addWidget(connectionGroup, 0, 0);
    mainLayout->addWidget(paramsGroup, 1, 0);
    mainLayout->addWidget(logGroup, 2, 0);
    
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::toggleSending);
}

void MainWindow::toggleSending() {
    if (!isSending) {
        if (!validateInput()) {
            return;
        }
        
        isSending = true;
        toggleButton->setText("Остановить отправку");
        statusLabel->setText("Отправка данных...");
        statusBar->setStyleSheet("QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #4CAF50; }");
        
        if (autoSendCheck->isChecked()) {
            sendTimer->start(1000);
        } else {
            sendData();
        }
        
        logMessage("Начата отправка данных на сервер");
    } else {
        isSending = false;
        toggleButton->setText("Начать отправку");
        statusLabel->setText("Отправка остановлена");
        statusBar->setStyleSheet("QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #ff9800; }");
        
        sendTimer->stop();
        logMessage("Отправка данных остановлена");
    }
}

void MainWindow::sendData() {
    if (!validateInput()) {
        toggleSending();
        return;
    }
    
    int X = xSpin->value();
    int Y = ySpin->value();
    int V = vSpin->value();
    int M = mSpin->value();
    int S = sSpin->value();
    double A = aSpin->value();
    int P = pSpin->value();
    uint16_t R = static_cast<uint16_t>(rSpin->value());
    
    ClientPacket packet = createPacket(X, Y, V, M, S, A, P, R);
    
    QHostAddress serverAddr(serverIpEdit->text());
    quint16 serverPort = static_cast<quint16>(serverPortSpin->value());
    
    qint64 sent = udpSocket->writeDatagram(reinterpret_cast<const char*>(&packet), sizeof(packet), serverAddr, serverPort);
    
    if (sent == sizeof(packet)) {
        sentCount++;
        logMessage(QString("Отправлен пакет #%1: X=%2, Y=%3, V=%4, M=%5, S=%6, A=%7, P=%8, R=%9")
                  .arg(sentCount).arg(X).arg(Y).arg(V).arg(M).arg(S).arg(A).arg(P).arg(R));
    } else {
        errorCount++;
        logMessage("Ошибка отправки пакета");
    }
    
    updateConnectionStatus();
}

void MainWindow::readResponse() {
    while (udpSocket->hasPendingDatagrams()) {
        ServerResponse response;
        QHostAddress senderAddr;
        quint16 senderPort;
        
        qint64 size = udpSocket->readDatagram(reinterpret_cast<char*>(&response), sizeof(response), &senderAddr, &senderPort);
        
        if (size == sizeof(response)) {
            if (response.messageId == 0) {
                if (response.isValid == 1) {
                    successCount++;
                    logMessage(QString("✓ Получен ответ: данные корректны"));
                } else {
                    errorCount++;
                    logMessage(QString("✗ Получен ответ: данные некорректны"));
                }
            } else {
                errorCount++;
                logMessage("Неизвестный идентификатор сообщения");
            }
        }
        
        updateConnectionStatus();
    }
}

void MainWindow::updateConnectionStatus() {
    QString statusText;
    QString styleSheet;
    
    if (isSending) {
        statusText = QString("Отправка данных... | Отправлено: %1 | Успешно: %2 | Ошибки: %3")
                    .arg(sentCount).arg(successCount).arg(errorCount);
        styleSheet = "QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #4CAF50; }";
    } else if (udpSocket->state() == QUdpSocket::BoundState) {
        statusText = "Готов к отправке";
        styleSheet = "QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #2196F3; }";
    } else {
        statusText = "Не подключено";
        styleSheet = "QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #ff9800; }";
    }
    
    statusLabel->setText(statusText);
    statusBar->setStyleSheet(styleSheet);
}

bool MainWindow::validateInput() {
    QHostAddress addr(serverIpEdit->text());
    if (addr.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Неверный IP адрес сервера");
        return false;
    }

    if (serverPortSpin->value() <= 0 || serverPortSpin->value() > 65535) {
        QMessageBox::warning(this, "Ошибка", "Неверный порт сервера");
        return false;
    }
    
    return true;
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->append(QString("[%1] %2").arg(timestamp, message));

    if (logText->document()->lineCount() > 1000) {
        QTextCursor cursor(logText->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 100);
        cursor.removeSelectedText();
    }
}