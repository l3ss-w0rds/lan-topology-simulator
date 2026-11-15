#pragma once
#include <QWidget>
#include <QStringList>

class QPlainTextEdit;
class QLineEdit;
class QLabel;
class NetworkDevice;

class TerminalWidget : public QWidget {
    Q_OBJECT
public:
    explicit TerminalWidget(NetworkDevice* device, QWidget* parent = nullptr);
    ~TerminalWidget() override = default;

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override; //управление фокусом, навигация истории, автодополнение

private slots:
    void handleCommand(); //Обработчик нажатия на Энтер: извлекает, обновляет, запускает выполнение

private:
    void setupUI(); 
    void executeCommand(const QString& rawCommand); //Разбивает строку на токены и направляет к нужному обработчику
    void print(const QString& text = QString()); //Выводим текст в область вывода и сколлбар вниз
    void saveHistory(); //Созранение истории для восстановления открытия
    QString promptString() const; //строка приглашения ввода в зависимости от типа устройств

    void cmdHelp(); //вывод справки по доступным по командам
    void cmdClear();
    void cmdIpconfig(); //вывод сетевой конфигурации для всех устройств
    void cmdPing(const QStringList& tokens); //Эмуляция команды пинг
    void cmdRoute(const QStringList& tokens);
    void cmdShowMacTable(const QStringList& tokens); //Вывод табл. мак адресов для коммутатора
    void cmdArp(const QStringList& tokens); //Просмотр или очистка АРП таблиц для устройств 3го уровня

private:
    NetworkDevice* m_device = nullptr;
    QPlainTextEdit* m_outputView = nullptr;
    QLineEdit* m_inputEdit = nullptr;
    QLabel* m_promptLabel = nullptr;
    QStringList     m_history;
    int             m_historyPos = 0;
};