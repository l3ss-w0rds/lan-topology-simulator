#include "LanTopologySimulator.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. Проверяем правильный путь с префиксом проекта:
    QFile styleFile(":/LanTopologySimulator/style.qss");

    // Если вдруг префикс другой — пробуем запасные пути:
    if (!styleFile.exists()) {
        styleFile.setFileName(":/style.qss");
    }
    if (!styleFile.exists()) {
        styleFile.setFileName("style.qss");
    }

    // 2. Открываем и применяем
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
    else {
        // Если файл ВСЁ РАВНО не найден — программа честно покажет всплывающее окно!
        QMessageBox::critical(nullptr, "Ошибка QSS",
            "Файл style.qss не найден в ресурсах!\nПроверьте путь в .qrc");
    }

    LanTopologySimulator window;
    window.show();
    return app.exec();
}
