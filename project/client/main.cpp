#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.setWindowTitle("UDP Client");
    window.resize(600, 500);
    window.show();
    
    return app.exec();
}