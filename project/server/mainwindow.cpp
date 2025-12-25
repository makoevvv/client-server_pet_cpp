#include "mainwindow.h"

#include <QCoreApplication>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), totalPackets(0), validPackets(0), invalidPackets(0), isRunning(false) {
    setupUI();
    
    udpSocket = new QUdpSocket(this);
    updateTimer = new QTimer(this);
    
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::readDatagrams);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateClientList);
    
    const QStringList limitsPaths = {
        QCoreApplication::applicationDirPath() + "/limits.json",
        QCoreApplication::applicationDirPath() + "/../server/limits.json",
        QStringLiteral("limits.json")
    };

    bool limitsLoaded = false;
    for (const QString& path : limitsPaths) {
        if (limits.loadFromJson(path.toStdString())) {
            logMessage(QString("✓ Загружены ограничения из %1").arg(QDir::toNativeSeparators(path)));
            limitsLoaded = true;
            break;
        }
    }

    if (!limitsLoaded) {
        logMessage("⚠ Не удалось загрузить limits.json, используются значения по умолчанию");
    }
    
    logMessage("Сервер инициализирован. Нажмите 'Запустить сервер' для начала работы.");
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QGroupBox *controlGroup = new QGroupBox("Управление сервером", this);
    QGridLayout *controlLayout = new QGridLayout(controlGroup);
    
    controlLayout->addWidget(new QLabel("Порт:"), 0, 0);
    portSpin = new QSpinBox(this);
    portSpin->setRange(1, 65535);
    portSpin->setValue(8080);
    controlLayout->addWidget(portSpin, 0, 1);
    
    toggleButton = new QPushButton("Запустить сервер", this);
    controlLayout->addWidget(toggleButton, 1, 0, 1, 2);
    
    statusBar = new QProgressBar(this);
    statusBar->setTextVisible(false);
    statusBar->setMaximumHeight(10);
    controlLayout->addWidget(statusBar, 2, 0, 1, 2);
    
    statusLabel = new QLabel("Сервер остановлен", this);
    controlLayout->addWidget(statusLabel, 3, 0, 1, 2);
    
    statsLabel = new QLabel("Пакеты: 0 | Корректные: 0 | Некорректные: 0", this);
    controlLayout->addWidget(statsLabel, 4, 0, 1, 2);
    
    QPushButton *clearButton = new QPushButton("Очистить логи", this);
    controlLayout->addWidget(clearButton, 5, 0, 1, 2);
    
    QGroupBox *logGroup = new QGroupBox("Логи сервера", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    logLayout->addWidget(logText);
    
    leftLayout->addWidget(controlGroup);
    leftLayout->addWidget(logGroup);
    
    QGroupBox *clientsGroup = new QGroupBox("Подключенные клиенты", this);
    QVBoxLayout *clientsLayout = new QVBoxLayout(clientsGroup);
    
    clientsTable = new QTableWidget(this);
    clientsTable->setColumnCount(6);
    clientsTable->setHorizontalHeaderLabels({"IP:Порт", "Всего пакетов", "Успешно", "Ошибки", "% ошибок", "Последняя активность"});
    clientsTable->horizontalHeader()->setStretchLastSection(true);
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    clientsLayout->addWidget(clientsTable);
    
    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addWidget(clientsGroup, 1);
    
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::toggleServer);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearLogs);
}

void MainWindow::toggleServer() {
    if (!isRunning) {
        quint16 port = static_cast<quint16>(portSpin->value());
        
        if (udpSocket->bind(port)) {
            isRunning = true;
            toggleButton->setText("Остановить сервер");
            statusLabel->setText(QString("Сервер запущен на порту %1").arg(port));
            statusBar->setStyleSheet("QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #4CAF50; }");
            
            updateTimer->start(1000);
            
            logMessage(QString("✓ Сервер запущен на порту %1").arg(port));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось запустить сервер на указанном порту");
            logMessage("✗ Ошибка запуска сервера");
        }
    } else {
        isRunning = false;
        udpSocket->close();
        toggleButton->setText("Запустить сервер");
        statusLabel->setText("Сервер остановлен");
        statusBar->setStyleSheet("QProgressBar { background-color: #e0e0e0; } QProgressBar::chunk { background-color: #ff9800; }");
        
        updateTimer->stop();
        logMessage("Сервер остановлен");
    }
}

void MainWindow::readDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        ClientPacket packet;
        QHostAddress senderAddr;
        quint16 senderPort;
        
        qint64 size = udpSocket->readDatagram(reinterpret_cast<char*>(&packet), sizeof(packet), &senderAddr, &senderPort);
        
        if (size == sizeof(packet)) {
            processPacket(packet, senderAddr, senderPort);
        }
    }
}

void MainWindow::processPacket(const ClientPacket& packet, const QHostAddress& address, quint16 port) {
    QString clientId = QString("%1:%2").arg(address.toString()).arg(port);
    
    if (!clients.contains(clientId)) {
        clients[clientId] = ClientInfo(address, port);
        logMessage(QString("Новый клиент: %1").arg(clientId));
    }
    
    int X, Y, V, M, S, P;
    double A;
    uint16_t R;
    unpackPacket(packet, X, Y, V, M, S, A, P, R);
    
    totalPackets++;
    
    bool isValid = validateParameters(limits, X, Y, V, M, S, A, P);
    
    if (isValid) {
        validPackets++;
    } else {
        invalidPackets++;
    }
    
    clients[clientId].updateStats(isValid);
    
    QString status = isValid ? "корректны" : "некорректны";
    logMessage(QString("Пакет от %1: X=%2, Y=%3, V=%4, M=%5, S=%6, A=%7, P=%8 - %9")
              .arg(clientId).arg(X).arg(Y).arg(V).arg(M).arg(S).arg(A, 0, 'f', 1).arg(P).arg(status));
    
    ServerResponse response;
    response.messageId = 0;
    response.isValid = isValid ? 1 : 0;
    
    udpSocket->writeDatagram(reinterpret_cast<const char*>(&response), sizeof(response), address, port);
    
    updateStatistics();
}

void MainWindow::updateClientList() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QList<QString> toRemove;
    
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (currentTime - it->lastSeen > 30000) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString& clientId : toRemove) {
        logMessage(QString("Клиент отключен: %1").arg(clientId));
        clients.remove(clientId);
    }
    
    clientsTable->setRowCount(clients.size());
    
    QList<ClientInfo> sortedClients = clients.values();
    std::sort(sortedClients.begin(), sortedClients.end());
    
    int row = 0;
    for (const ClientInfo& client : sortedClients) {
        clientsTable->setItem(row, 0, new QTableWidgetItem(client.id));
        clientsTable->setItem(row, 1, new QTableWidgetItem(QString::number(client.totalPackets)));
        clientsTable->setItem(row, 2, new QTableWidgetItem(QString::number(client.successCount)));
        clientsTable->setItem(row, 3, new QTableWidgetItem(QString::number(client.errorCount)));
        clientsTable->setItem(row, 4, new QTableWidgetItem(QString::number(client.errorRate(), 'f', 1) + "%"));
        
        qint64 secondsAgo = (QDateTime::currentMSecsSinceEpoch() - client.lastSeen) / 1000;
        clientsTable->setItem(row, 5, new QTableWidgetItem(QString("%1 сек. назад").arg(secondsAgo)));
        
        if (client.errorRate() > 50) {
            for (int col = 0; col < clientsTable->columnCount(); ++col) {
                clientsTable->item(row, col)->setBackground(QColor(255, 200, 200));
            }
        } else if (client.errorRate() > 20) {
            for (int col = 0; col < clientsTable->columnCount(); ++col) {
                clientsTable->item(row, col)->setBackground(QColor(255, 255, 200));
            }
        }
        
        row++;
    }
}

void MainWindow::updateStatistics() {
    statsLabel->setText(QString("Пакеты: %1 | Корректные: %2 | Некорректные: %3")
                       .arg(totalPackets).arg(validPackets).arg(invalidPackets));
}

void MainWindow::clearLogs() {
    logText->clear();
    logMessage("Логи очищены");
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
    
    QTextCursor cursor(logText->textCursor());
    cursor.movePosition(QTextCursor::End);
    logText->setTextCursor(cursor);
}