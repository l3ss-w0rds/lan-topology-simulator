#pragma once
#include <QWidget>

class QPlainTextEdit;
class QLineEdit;
class NetworkDevice;

class TerminalWidget : public QWidget {
public:
    explicit TerminalWidget(NetworkDevice* device, QWidget* parent = nullptr);
    ~TerminalWidget() override = default;

private:
    void setupUI();        // Вёрстка и стили терминала
    void handleCommand();  // Слот: срабатывает при нажатии Enter в строке ввода

    void executeCommand(const QString& rawCommand); // Выполнение команды и вывод результата 
	void cmdHelp(); // Вывод справки по командам
	void cmdClear(); // Очистка терминала
	void cmdIpconfig(); // Вывод информации о сетевых интерфейсах
	void cmdPing(const QStringList& args); // Команда ping


private:
    NetworkDevice* m_device;
    QPlainTextEdit* m_outputView = nullptr; // Черный экран вывода
    QLineEdit* m_inputEdit = nullptr;       // Строка ввода команд


};