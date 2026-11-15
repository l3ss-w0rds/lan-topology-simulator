#include "TerminalWidget.h"
#include "NetworkDevice.h"
//#include "MacTable.h"
#include "ArpTable.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QKeyEvent>
#include <QEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QFontDatabase>
#include <QPalette>
#include <QColor>
// ====================== Вспомогательное ======================
namespace {

    QFont monoFont()
    {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        const QStringList preferred = { "Cascadia Mono", "JetBrains Mono", "Consolas" };
        const QStringList families = QFontDatabase().families();
        for (const QString& name : preferred)
            if (families.contains(name)) { f.setFamily(name); break; }
        f.setPointSize(10);
        return f;
    }

    QString col(const QString& s, int w) { return s.leftJustified(w, ' '); }

} // namespace

// ====================== Конструктор ======================
TerminalWidget::TerminalWidget(NetworkDevice* device, QWidget* parent)
    : QWidget(parent)
    , m_device(device)
{
    setupUI();

}

// ====================== UI ======================
void TerminalWidget::setupUI()
{
    // Без WA_StyledBackground фон и рамка у QWidget не рисуются
    setObjectName("terminalRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "#terminalRoot {"
        "   background-color: #0F0F0F;"
        "   border: 1px solid #2A2A2A;"
        "   border-radius: 6px;"
        "}"
        "#terminalRoot QLabel {"
        "   background-color: transparent;"
        "   font-family: 'Consolas';"
        "   font-size: 13px;"
        "}"
        "#terminalRoot QLabel#terminalTitle {"
        "   color: #22C55E;"
        "}"
        "#terminalRoot QLabel#terminalPrompt {"
        "   color: #D4D4D4;"
        "   font-weight: bold;"
        "}"
        "#terminalRoot QFrame#terminalSep {"
        "   background-color: #2A2A2A;"
        "   border: none;"
        "}"
        "#terminalRoot QPlainTextEdit#terminalOutput {"
        "   background-color: #0F0F0F;"
        "   color: #22C55E;"
        "   border: none;"
        "   border-radius: 0px;"
        "   padding: 0px;"
        "   font-family: 'Consolas';"
        "   font-size: 13px;"
        "   selection-background-color: #14532D;"
        "   selection-color: #22C55E;"
        "}"
        // :hover и :focus переопределяем, иначе подтянутся стили из общего qss
        "#terminalRoot QLineEdit#terminalInput,"
        "#terminalRoot QLineEdit#terminalInput:hover,"
        "#terminalRoot QLineEdit#terminalInput:focus {"
        "   background-color: #0F0F0F;"
        "   color: #22C55E;"
        "   border: none;"
        "   border-radius: 0px;"
        "   padding: 2px 0px;"
        "   font-family: 'Consolas';"
        "   font-size: 13px;"
        "   selection-background-color: #14532D;"
        "   selection-color: #22C55E;"
        "}"
        "#terminalRoot QScrollBar:vertical {"
        "   background: transparent; width: 8px; margin: 0; border: none;"
        "}"
        "#terminalRoot QScrollBar::handle:vertical {"
        "   background: #2E2E2E; border-radius: 4px; min-height: 24px;"
        "}"
        "#terminalRoot QScrollBar::handle:vertical:hover {"
        "   background: #444444;"
        "}"
        "#terminalRoot QScrollBar::add-line:vertical,"
        "#terminalRoot QScrollBar::sub-line:vertical {"
        "   height: 0; border: none;"
        "}"
        "#terminalRoot QScrollBar::add-page:vertical,"
        "#terminalRoot QScrollBar::sub-page:vertical {"
        "   background: none;"
        "}"
    );

    const QFont mono = monoFont();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 10);
    mainLayout->setSpacing(0);

    // ---------- 1. Заголовок: имя узла и IP ----------
    QString title = m_device
        ? QString("%1  -  %2").arg(m_device->getName(), m_device->getIP())
        : QString("Терминал");
    QLabel* titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("terminalTitle");
    titleLabel->setFont(mono);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(6);
    QFrame* topLine = new QFrame(this);
    topLine->setObjectName("terminalSep");
    topLine->setFixedHeight(1);
    mainLayout->addWidget(topLine);
    mainLayout->addSpacing(6);

    // ---------- 2. Окно вывода ----------
    m_outputView = new QPlainTextEdit(this);
    m_outputView->setObjectName("terminalOutput");
    m_outputView->setReadOnly(true);
    m_outputView->setFont(mono);
    m_outputView->setFrameShape(QFrame::NoFrame);
    m_outputView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_outputView->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_outputView->setMaximumBlockCount(1000);
    m_outputView->document()->setDocumentMargin(0);
    m_outputView->viewport()->installEventFilter(this); // клик -> фокус на ввод
    mainLayout->addWidget(m_outputView, 1);

    // Загрузка истории или приветствие
    if (m_device) {
        QString saved = m_device->getTerminalHistory();
        if (saved.startsWith("<!DOCTYPE"))   // остаток от прошлой HTML-версии
            saved.clear();

        if (!saved.isEmpty()) {
            m_outputView->setPlainText(saved);
        }
        else {
            print(QString("Сетевой узел %1 [%2]").arg(m_device->getName(), m_device->getIP()));
            print("Терминал готов к работе. Введите 'help' для списка команд.");
            print();
            saveHistory();
        }
    }

    // ---------- 3. Разделитель ----------
    mainLayout->addSpacing(6);
    QFrame* bottomLine = new QFrame(this);
    bottomLine->setObjectName("terminalSep");
    bottomLine->setFixedHeight(1);
    mainLayout->addWidget(bottomLine);
    mainLayout->addSpacing(6);

    // ---------- 4. Строка ввода ----------
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);

    m_promptLabel = new QLabel(promptString(), this);
    m_promptLabel->setObjectName("terminalPrompt");
    m_promptLabel->setFont(mono);
    inputLayout->addWidget(m_promptLabel);

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setObjectName("terminalInput");
    m_inputEdit->setFont(mono);
    m_inputEdit->setFrame(false);
    m_inputEdit->setPlaceholderText("help, ping, ipconfig, arp -a ...");
    QPalette inputPalette = m_inputEdit->palette();
    inputPalette.setColor(QPalette::PlaceholderText, QColor("#22C55E"));
    m_inputEdit->setPalette(inputPalette);
    m_inputEdit->installEventFilter(this);
    inputLayout->addWidget(m_inputEdit, 1);

    mainLayout->addLayout(inputLayout);

    setFocusProxy(m_inputEdit);
    setFocusProxy(m_inputEdit);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TerminalWidget::handleCommand);
}

// ====================== Сервисные методы ======================
QString TerminalWidget::promptString() const
{
    const QString name = m_device ? m_device->getName() : QString("host");
    const bool isSwitch = m_device && m_device->getType() == DeviceType::Switch;
    return name + (isSwitch ? "#" : ">");
}

void TerminalWidget::print(const QString& text)
{
    m_outputView->appendPlainText(text);          // у QPlainTextEdit метод называется appendPlainText
    QScrollBar* bar = m_outputView->verticalScrollBar();
    bar->setValue(bar->maximum());                // всегда прокручиваем вниз
}

void TerminalWidget::saveHistory()
{
    if (m_device)
        m_device->setTerminalHistory(m_outputView->toPlainText());
}

bool TerminalWidget::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_inputEdit && ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        switch (ke->key()) {
        case Qt::Key_Up:
            if (!m_history.isEmpty()) {
                m_historyPos = qMax(0, m_historyPos - 1);
                m_inputEdit->setText(m_history[m_historyPos]);
            }
            return true;
        case Qt::Key_Down:
            if (m_historyPos < m_history.size() - 1) {
                m_inputEdit->setText(m_history[++m_historyPos]);
            }
            else {
                m_historyPos = m_history.size();
                m_inputEdit->clear();
            }
            return true;
        case Qt::Key_Tab: {
            static const QStringList cmds = { "help", "ipconfig", "ifconfig", "cls", "clear",
                                              "arp", "ping", "show mac-address-table" };
            const QString t = m_inputEdit->text().trimmed().toLower();
            if (!t.isEmpty()) {
                QStringList m;
                for (const QString& c : cmds)
                    if (c.startsWith(t)) m << c;
                if (m.size() == 1) m_inputEdit->setText(m.first() + " ");
            }
            return true;
        }
        case Qt::Key_Escape:
            m_inputEdit->clear();
            return true;
        case Qt::Key_L:
            if (ke->modifiers() & Qt::ControlModifier) { cmdClear(); saveHistory(); return true; }
            break;
        default:
            break;
        }
    }
    if (m_outputView && obj == m_outputView->viewport() && ev->type() == QEvent::MouseButtonRelease) {
        if (!m_outputView->textCursor().hasSelection())
            m_inputEdit->setFocus();
    }
    return QWidget::eventFilter(obj, ev);
}

// ====================== Команды ======================
void TerminalWidget::handleCommand()
{
    const QString command = m_inputEdit->text().trimmed();
    m_inputEdit->clear();
    if (command.isEmpty()) return;

    if (m_history.isEmpty() || m_history.last() != command)
        m_history << command;
    m_historyPos = m_history.size();

    print(promptString() + " " + command);

    executeCommand(command);
    saveHistory();
}

void TerminalWidget::executeCommand(const QString& rawCommand)
{
    const QStringList tokens = rawCommand.simplified().split(' ', QString::SkipEmptyParts);
    if (tokens.isEmpty()) return;

    const QString cmd = tokens[0].toLower();

    if (cmd == "help")                          cmdHelp();
    else if (cmd == "cls" || cmd == "clear")         cmdClear();
    else if (cmd == "ipconfig" || cmd == "ifconfig") cmdIpconfig();
    else if (cmd == "show" || cmd == "mac-table")    cmdShowMacTable(tokens);
    else if (cmd == "arp")                           cmdArp(tokens);
    else if (cmd == "ping")                          cmdPing(tokens);
    else {
        print(QString("'%1' не является внутренней или внешней командой.").arg(tokens[0]));
        print("Введите 'help' для просмотра списка доступных команд.");
        print("");
    }
}

void TerminalWidget::cmdHelp()
{
    print("Доступные команды:");
    print("  help                   - справка по командам");
    print("  ipconfig               - сетевая конфигурация и состояние портов");
    print("  ping <ip>              - проверка доступности узла");
    const bool isSwitch = m_device && m_device->getType() == DeviceType::Switch;
    if (isSwitch) {
        print("  show mac-address-table - таблица коммутации (CAM)");
    }
    else {
        print("  arp -a                 - таблица соответствия IP и MAC");
        print("  arp -d                 - очистка ARP-кэша");
    }
    print("  cls                    - очистка терминала");
    print("");
    print("Подсказка: стрелки вверх/вниз - история команд, Tab - автодополнение.");
    print("");
}

void TerminalWidget::cmdClear()
{
    m_outputView->clear();
}

void TerminalWidget::cmdIpconfig()
{
    if (!m_device) return;

    print(QString("Настройка протокола IP для узла \"%1\":\n").arg(m_device->getName()));
    print(QString("   Имя узла . . . . . . . . . . . . : %1").arg(m_device->getName()));
    print(QString("   Физический адрес (MAC) . . . . . : %1").arg(m_device->getMac()));
    print(QString("   IPv4-адрес . . . . . . . . . . . : %1").arg(m_device->getIP()));
    print("   Маска подсети  . . . . . . . . . : 255.255.255.0");

    print("\nСостояние сетевых адаптеров:");
    for (const auto& port : m_device->getPorts()) {
        const QString status = port.isOccupied ? "Подключен" : "Отключен";
        QString peer = (port.isOccupied && port.connectedDevice)
            ? QString(" -> %1 [%2]").arg(port.connectedDevice->getName(), port.connectedPortName)
            : QString();
        print(QString("   Адаптер %1: [%2]%3").arg(port.name, status, peer));
    }
    print("");
}

void TerminalWidget::cmdShowMacTable(const QStringList& tokens)
{
    if (!m_device) return;

    if (tokens[0].toLower() == "show" && !tokens.value(1).toLower().startsWith("mac")) {
        print("Использование: show mac-address-table");
        print("");
        return;
    }

    if (m_device->getType() != DeviceType::Switch) {
        print("Ошибка: таблица коммутации доступна только на коммутаторах (Switch).");
        print("Для просмотра ARP-таблицы на ПК используйте команду: arp -a");
        print("");
        return;
    }

    MacTable* table = m_device->getMacTable();
    if (!table || table->getSize() == 0) {
        print("Таблица MAC-адресов пуста (коммутатор еще не получал кадры).");
        print("");
        return;
    }

    print("Таблица MAC-адресов коммутатора (CAM Table)");
    print(col("Vlan", 8) + col("Mac Address", 20) + col("Type", 12) + "Port");
    print(col("----", 8) + col("-----------------", 20) + col("--------", 12) + "----------------");

    for (const auto& entry : table->getEntries()) {
        QString typeStr;
        switch (entry.type) {
        case TypeEntry::Dynamic: typeStr = "DYNAMIC";   break;
        case TypeEntry::Static:  typeStr = "STATIC";    break;
        default:                 typeStr = "BLACKHOLE"; break;
        }
        const QString portStr = (entry.portIndex >= 0 && entry.portIndex < m_device->getPorts().size())
            ? m_device->getPorts()[entry.portIndex].name : QString("Drop");

        print(col(QString("%1").arg(entry.VlanID), 8)
            + col(entry.macAddress, 20)
            + col(typeStr, 12)
            + portStr);
    }
    print(QString("\nВсего записей: %1").arg(table->getSize()));
    print("");
}

void TerminalWidget::cmdArp(const QStringList& tokens)
{
    if (!m_device) return;

    if (m_device->getType() == DeviceType::Switch) {
        print("Ошибка: коммутатор (L2 Switch) не использует протокол ARP.");
        print("Для просмотра таблицы коммутации используйте: show mac-address-table");
        print("");
        return;
    }

    ArpTable* arp = m_device->getArpTable();
    if (!arp) return;

    if (tokens.size() > 1 && tokens[1].toLower() == "-d") {
        arp->clear();
        print("ARP-кэш успешно очищен.");
        print("");
        return;
    }

    print(QString("Интерфейс: %1 --- 0x1").arg(m_device->getIP()));

    if (arp->size() == 0) {
        print("  ARP-таблица пуста (записи появятся после первого обмена пакетами ping).");
        print("");
        return;
    }

    print("  " + col("Интернет-адрес", 22) + col("Физический адрес", 21) + "Тип");
    print("  " + col("--------------------", 22) + col("-------------------", 21) + "------------");

    for (const auto& entry : arp->getEntries()) {
        const QString typeStr = (entry.type == ArpEntryType::Dynamic) ? "динамический" : "статический";
        print("  " + col(entry.ip, 22) + col(entry.mac, 21) + typeStr);
    }
    print("");
}

void TerminalWidget::cmdPing(const QStringList& tokens)
{
    if (!m_device) return;

    if (tokens.size() < 2) {
        print("Использование: ping <IP-адрес>");
        print("Пример: ping 192.168.0.21");
        print("");
        return;
    }

    QStringList logLines;
    m_device->ping(tokens[1].trimmed(), logLines);
    for (const QString& line : logLines)
        print(line);
    print("");
}