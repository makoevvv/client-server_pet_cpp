#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QProgressBar>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <algorithm>
#include "clientinfo.h"
#include "../common/packet.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleServer();
    void readDatagrams();
    void updateClientList();
    void clearLogs();
    void updateStatistics();

private:
    void setupUI();
    void logMessage(const QString& message);
    void processPacket(const ClientPacket& packet, const QHostAddress& address, quint16 port);
    
    QUdpSocket *udpSocket;
    QTimer *updateTimer;
    ParameterLimits limits;
    
    QPushButton *toggleButton;
    QSpinBox *portSpin;
    QProgressBar *statusBar;
    QLabel *statusLabel;
    QLabel *statsLabel;
    
    QTableWidget *clientsTable;
    QTextEdit *logText;
    
    QMap<QString, ClientInfo> clients;
    int totalPackets;
    int validPackets;
    int invalidPackets;
    
    bool isRunning;
};

#endif