#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QHostAddress>
#include <QDateTime>
#include "../common/packet.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleSending();
    void sendData();
    void readResponse();
    void updateConnectionStatus();

private:
    void setupUI();
    void logMessage(const QString& message);
    bool validateInput();
    
    QUdpSocket *udpSocket;
    QTimer *sendTimer;
    QTimer *statusTimer;
    
    QLineEdit *serverIpEdit;
    QSpinBox *serverPortSpin;
    QPushButton *toggleButton;
    QProgressBar *statusBar;
    QLabel *statusLabel;
    
    QSpinBox *xSpin, *ySpin, *vSpin, *mSpin, *sSpin, *pSpin;
    QDoubleSpinBox *aSpin;
    QSpinBox *rSpin;
    QCheckBox *autoSendCheck;
    
    QTextEdit *logText;
    
    bool isSending;
    int sentCount;
    int successCount;
    int errorCount;
};

#endif