#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <QString>
#include <QHostAddress>
#include <QDateTime>

struct ClientInfo {
    QHostAddress address;
    quint16 port;
    QString id;
    int totalPackets;
    int errorCount;
    int successCount;
    qint64 lastSeen;
    
    ClientInfo(const QHostAddress& addr = QHostAddress(), quint16 p = 0) 
        : address(addr), port(p), totalPackets(0), errorCount(0), successCount(0), lastSeen(0) {
        id = QString("%1:%2").arg(addr.toString()).arg(p);
    }
    
    void updateStats(bool isValid) {
        totalPackets++;
        if (isValid) {
            successCount++;
        } else {
            errorCount++;
        }
        lastSeen = QDateTime::currentMSecsSinceEpoch();
    }
    
    double errorRate() const {
        return totalPackets > 0 ? (static_cast<double>(errorCount) / totalPackets) * 100.0 : 0.0;
    }
    
    bool operator<(const ClientInfo& other) const {
        return errorCount > other.errorCount;
    }
};

#endif