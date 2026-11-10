#include "TerminalWidget.h"
#include "NetworkDevice.h"
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QDebug>

TerminalWidget::TerminalWidget(NetworkDevice* device, QWidget* parent)
    : QWidget(parent), m_device(device)
{
    setupUI();
}

void TerminalWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    //Черный экран терминала
    m_outputView = new QPlainTextEdit(this);
    m_outputView->setReadOnly(true);
    m_outputView->setStyleSheet(
        "QPlainTextEdit {"
        "   background-color: #0F172A;" // Темно-графитовый фон
        "   color: #22C55E;"            // Яркий терминальный зеленый цвет
        "   font-family: 'Consolas', 'Courier New', monospace;"
        "   font-size: 13px;"
        "   border: 1px solid #1E293B;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "}"
    );

    // Приветственное сообщение и восстановление прошлого сеанса
    if (m_device) {
        if (!m_device->getTerminalHistory().isEmpty()) {
            m_outputView->appendPlainText(m_device->getTerminalHistory());
        }
        else {
            m_outputView->appendPlainText(QString("--- Терминал узла %1 [%2] ---")
                .arg(m_device->getName(), m_device->getIP()));
            m_outputView->appendPlainText("Введите 'help' для списка доступных команд.\n");
        }
    }
    layout->addWidget(m_outputView);

    //Строка ввода команд
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText("Введите команду и нажмите Enter...");
    m_inputEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #1E293B;"
        "   color: #F8FAFC;"
        "   font-family: 'Consolas', 'Courier New', monospace;"
        "   font-size: 13px;"
        "   border: 1px solid #334155;"
        "   border-radius: 6px;"
        "   padding: 6px 10px;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #22C55E;" // Зеленый контур при фокусе
        "}"
    );
    layout->addWidget(m_inputEdit);

    // Нажатие клавиши Enter вызывает метод handleCommand()
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TerminalWidget::handleCommand);
}

void TerminalWidget::handleCommand()
{
    QString command = m_inputEdit->text().trimmed();
    if (command.isEmpty()) return;
    //Выводим саму команду
    m_outputView->appendPlainText("> " + command);
    m_inputEdit->clear();

    executeCommand(command);

    if (m_device) {
        m_device->setTerminalHistory(m_outputView->toPlainText());
    }
}


void TerminalWidget::executeCommand(const QString& rawCommand) {
	QStringList tokens = rawCommand.split(' ', QString::SkipEmptyParts);
	if (tokens.isEmpty()) return;


    QString cmd = tokens[0].toLower(); // Команда без учета регистра (CLS == cls)

    if (cmd == "help") {
        cmdHelp();
    }
    else if (cmd == "cls" || cmd == "clear") {
        cmdClear();
    }
    else if (cmd == "ipconfig" || cmd == "ifconfig") {
        cmdIpconfig();
    }
    else {
        // Ошибка неизвестной команды
        m_outputView->appendPlainText(QString("'%1' не является внутренней или внешней командой.")
            .arg(tokens[0]));
        m_outputView->appendPlainText("Введите 'help' для просмотра списка доступных команд.\n");
    }
}


void TerminalWidget::cmdHelp() {
	m_outputView->appendPlainText("Доступные команды сетевого симулятора:");
	m_outputView->appendPlainText("  help                  - вызов справки по командам");


	m_outputView->appendPlainText("  ipconfig/ifconfig   - вывод сетевой конфигурации интерфейса");
    m_outputView->appendPlainText("  cls/clear             - очистка терминала");
	m_outputView->appendPlainText("  ping                  - проверка доступности узла по IP");
}

void TerminalWidget::cmdIpconfig() {
    if (!m_device) return;

    m_outputView->appendPlainText(QString("Настройка протокола IP для узла \"%1\":\n")
        .arg(m_device->getName()));

    m_outputView->appendPlainText(QString("   Имя узла . . . . . . . . . . . . : %1").arg(m_device->getName()));
    m_outputView->appendPlainText(QString("   Физический адрес (MAC) . . . . . : %1").arg(m_device->getMac()));
    m_outputView->appendPlainText(QString("   IPv4-адрес . . . . . . . . . . . : %1").arg(m_device->getIP()));
    m_outputView->appendPlainText("   Маска подсети  . . . . . . . . . : 255.255.255.0");

    // Выводим статус физических портов
    m_outputView->appendPlainText("\nСостояние сетевых адаптеров:");
    for (const auto& port : m_device->getPorts()) {
        QString status = port.isOccupied ? "Подключен" : "Отключен";
        QString peer = (port.isOccupied && port.connectedDevice)
            ? QString(" -> %1 [%2]").arg(port.connectedDevice->getName(), port.connectedPortName)
            : "";
        m_outputView->appendPlainText(QString("   Адаптер %1: [%2]%3").arg(port.name, status, peer));
    }
    m_outputView->appendPlainText(""); // Пустая строка в конце
}

void TerminalWidget::cmdClear() {
    m_outputView->clear();
}

void TerminalWidget::cmdPing(const QStringList& args) {

}   