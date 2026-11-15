#include "TerminalWidget.h"
#include "NetworkDevice.h"
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

namespace {
    QFont monoFont()
    {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        const QStringList preferred = { "Cascadia Mono", "JetBrains Mono", "Consolas" };
        const QStringList families = QFontDatabase().families();

        for (const QString& name : preferred) {
            if (families.contains(name)) {
                f.setFamily(name);
                break;
            }
        }
        f.setPointSize(10);
        return f;
    }

    //функция для выравнивания текста по левому краю используется для табличного вывода
    QString col(const QString& s, int w) {
        return s.leftJustified(w, ' ');
    }

} // namespace

TerminalWidget::TerminalWidget(NetworkDevice* device, QWidget* parent)
    : QWidget(parent)
    , m_device(device)
{
    setupUI(); //инициализацию интерфейса отдельным методом
}

// Настройка пользовательского интерфейса терминала
void TerminalWidget::setupUI()
{
    setObjectName("terminalRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    const QFont mono = monoFont();

    //Основной вертикальный компоновщик
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 10);
    mainLayout->setSpacing(0);

    //Заголовок терминала (Имя устройства и IP)
    QString title = m_device
        ? QString("%1  -  %2").arg(m_device->getName(), m_device->getIP())
        : QString("Терминал");
    QLabel* titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("terminalTitle");
    titleLabel->setFont(mono);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(6);

    // Разделительная линия под заголовком
    QFrame* topLine = new QFrame(this);
    topLine->setObjectName("terminalSep");
    topLine->setFixedHeight(1);
    mainLayout->addWidget(topLine);
    mainLayout->addSpacing(6);

    //Область вывода текста (QPlainTextEdit)
    m_outputView = new QPlainTextEdit(this);
    m_outputView->setObjectName("terminalOutput");
    m_outputView->setReadOnly(true); // Запрет прямого редактирования вывода
    m_outputView->setFont(mono);
    m_outputView->setFrameShape(QFrame::NoFrame);
    m_outputView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_outputView->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_outputView->setMaximumBlockCount(1000); // Ограничение истории вывода для экономии памяти
    m_outputView->document()->setDocumentMargin(0);

    // Установка фильтра событий на viewport, чтобы перехватывать клики и передавать фокус в поле ввода
    m_outputView->viewport()->installEventFilter(this);
    mainLayout->addWidget(m_outputView, 1); // 1 = stretch factor, занимает всё доступное пространство

    //Загрузка сохраненной истории терминала из устройства
    if (m_device) {
        QString saved = m_device->getTerminalHistory();
        // Защита от устаревших данных (если ранее использовался HTML-формат)
        if (saved.startsWith("<!DOCTYPE"))
            saved.clear();

        if (!saved.isEmpty()) {
            m_outputView->setPlainText(saved);
        }
        else {
            // Приветственное сообщение для нового сеанса
            print(QString("Сетевой узел %1 [%2]").arg(m_device->getName(), m_device->getIP()));
            print("Терминал готов к работе. Введите 'help' для списка команд.");
            print();
            saveHistory();
        }
    }

    mainLayout->addSpacing(6);

    // Нижняя разделительная линия
    QFrame* bottomLine = new QFrame(this);
    bottomLine->setObjectName("terminalSep");
    bottomLine->setFixedHeight(1);
    mainLayout->addWidget(bottomLine);
    mainLayout->addSpacing(6);

    //Область ввода команды (горизонтальный компоновщик)
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);

    // Метка приглашения ввода (prompt)
    m_promptLabel = new QLabel(promptString(), this);
    m_promptLabel->setObjectName("terminalPrompt");
    m_promptLabel->setFont(mono);
    inputLayout->addWidget(m_promptLabel);

    // Поле ввода команды
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setObjectName("terminalInput");
    m_inputEdit->setFont(mono);
    m_inputEdit->setFrame(false); // Убираем стандартную рамку для бесшовного вида
    m_inputEdit->setPlaceholderText("help, ping, ipconfig, arp -a ...");

    // Настройка цвета текста-подсказки (зеленый оттенок)
    QPalette inputPalette = m_inputEdit->palette();
    inputPalette.setColor(QPalette::PlaceholderText, QColor("#22C55E"));
    m_inputEdit->setPalette(inputPalette);

    // Установка фильтра событий для перехвата нажатий клавиш (история, автодополнение)
    m_inputEdit->installEventFilter(this);
    inputLayout->addWidget(m_inputEdit, 1);

    mainLayout->addLayout(inputLayout);

    // Делаем поле ввода основным получателем фокуса для всего виджета
    setFocusProxy(m_inputEdit);

    // Обработка нажатия Enter для выполнения команды
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TerminalWidget::handleCommand);
}

// Коммутаторы (Switch) используют '#', хосты/маршрутизаторы используют '>'.
QString TerminalWidget::promptString() const
{
    const QString name = m_device ? m_device->getName() : QString("host");
    const bool isSwitch = m_device && m_device->getType() == DeviceType::Switch;
    return name + (isSwitch ? "#" : ">");
}

void TerminalWidget::print(const QString& text)
{
    m_outputView->appendPlainText(text);
    QScrollBar* bar = m_outputView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void TerminalWidget::saveHistory()
{
    if (m_device)
        m_device->setTerminalHistory(m_outputView->toPlainText());
}

// навигация по истории, автодополнение, очистка экрана и управление фокусом.
bool TerminalWidget::eventFilter(QObject* obj, QEvent* ev)
{
    // Обработка нажатий клавиш в поле ввода
    if (obj == m_inputEdit && ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        switch (ke->key()) {
        case Qt::Key_Up:
            // Перемещение назад по истории команд
            if (!m_history.isEmpty()) {
                m_historyPos = qMax(0, m_historyPos - 1);
                m_inputEdit->setText(m_history[m_historyPos]);
            }
            return true; // Событие обработано, не передаем дальше

        case Qt::Key_Down:
            // Перемещение вперед по истории команд
            if (m_historyPos < m_history.size() - 1) {
                m_inputEdit->setText(m_history[++m_historyPos]);
            }
            else {
                m_historyPos = m_history.size();
                m_inputEdit->clear();
            }
            return true;

        case Qt::Key_Tab: {
            // Автодополнение команд по префиксу
            static const QStringList cmds = { "help", "ipconfig", "ifconfig", "cls", "clear",
                                              "arp", "ping", "route", "show mac-address-table" };
            const QString t = m_inputEdit->text().trimmed().toLower();
            if (!t.isEmpty()) {
                QStringList matches;
                for (const QString& c : cmds) {
                    if (c.startsWith(t)) matches << c;
                }
                // Если найдено ровно одно совпадение, подставляем его
                if (matches.size() == 1) m_inputEdit->setText(matches.first() + " ");
            }
            return true;
        }
        case Qt::Key_Escape:
            // Очистка текущей строки ввода
            m_inputEdit->clear();
            return true;

        case Qt::Key_L:
            // Сочетание Ctrl+L для очистки экрана (как в настоящих терминалах)
            if (ke->modifiers() & Qt::ControlModifier) {
                cmdClear();
                saveHistory();
                return true;
            }
            break;

        default:
            break;
        }
    }

    // Обработка клика по области вывода: если текст не выделен, передаем фокус в поле ввода
    if (m_outputView && obj == m_outputView->viewport() && ev->type() == QEvent::MouseButtonRelease) {
        if (!m_outputView->textCursor().hasSelection())
            m_inputEdit->setFocus();
    }

    // Стандартная обработка событий базовым классом
    return QWidget::eventFilter(obj, ev);
}

// Обработчик нажатия Enter: извлекает команду, обновляет историю и запускает выполнение.
void TerminalWidget::handleCommand()
{
    const QString command = m_inputEdit->text().trimmed();
    m_inputEdit->clear();
    if (command.isEmpty()) return;

    // Добавляем команду в историю, избегая дублирования подряд идущих одинаковых команд
    if (m_history.isEmpty() || m_history.last() != command)
        m_history << command;

    // Сбрасываем позицию истории в конец (на "новую" пустую строку)
    m_historyPos = m_history.size();

    // Эхо-вывод введенной команды с приглашением
    print(promptString() + " " + command);

    executeCommand(command);
    saveHistory();
}

void TerminalWidget::executeCommand(const QString& rawCommand)
{
    // Разбиваем строку по пробелам, игнорируя множественные пробелы
    const QStringList tokens = rawCommand.simplified().split(' ', QString::SkipEmptyParts);
    if (tokens.isEmpty()) return;

    const QString cmd = tokens[0].toLower();

    if (cmd == "help")                          cmdHelp();
    else if (cmd == "cls" || cmd == "clear")    cmdClear();
    else if (cmd == "ipconfig" || cmd == "ifconfig") cmdIpconfig();
    else if (cmd == "show" || cmd == "mac-table")    cmdShowMacTable(tokens);
    else if (cmd == "arp")                           cmdArp(tokens);
    else if (cmd == "route")                         cmdRoute(tokens);
    else if (cmd == "ping")                          cmdPing(tokens);
    else {
        // Обработка неизвестной команды
        print(QString("'%1' не является внутренней или внешней командой.").arg(tokens[0]));
        print("Введите 'help' для просмотра списка доступных команд.");
        print("");
    }
}

void TerminalWidget::cmdHelp()
{
    print("Доступные команды:");
    print("  route <ip>             - выбор интерфейса и next hop (без отправки)");
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

void TerminalWidget::cmdClear(){
    m_outputView->clear();}

// Вывод сетевой конфигурации устройства (аналог ipconfig/ifconfig).
void TerminalWidget::cmdIpconfig()
{
    if (!m_device) return;
    print(QStringLiteral("Узел: %1").arg(m_device->getName()));

    const bool l3 = m_device->getType() != DeviceType::Switch;
    if (!l3) print(QStringLiteral("L2-коммутатор. IP управления не реализован."));

    const auto& ports = m_device->getPorts();
    for (const auto& port : ports) {
        print();
        print(QStringLiteral("Интерфейс %1:").arg(port.name));
        if (l3) {
            print("  MAC:   " + port.mac);
            print(QStringLiteral("  IPv4:  %1").arg(port.ipv4.isEmpty() ? QStringLiteral("не задан") : port.ipv4));
            print(QStringLiteral("  Маска: %1").arg(port.subnetMask.isEmpty() ? QStringLiteral("не задана") : port.subnetMask));
        }
        print(QStringLiteral("  Административно: %1").arg(port.adminUp ? QStringLiteral("включён") : QStringLiteral("выключен")));
        print(QStringLiteral("  Кабель: %1").arg(port.isOccupied ? QStringLiteral("подключён") : QStringLiteral("нет")));
    }

    if (l3) {
        const int index = m_device->getGatewayPortIndex();
        print();
        if (m_device->getDefaultGateway().isEmpty()) {
            print(QStringLiteral("Шлюз по умолчанию: не задан"));
        }
        else {
            print(QStringLiteral("Шлюз по умолчанию: %1 через %2").arg(
                m_device->getDefaultGateway(),
                index >= 0 && index < ports.size() ? ports[index].name : "?"
            ));
        }
    }
    print();
}

// Тестирование логики маршрутизации без реальной отправки пакетов.
void TerminalWidget::cmdRoute(const QStringList& tokens)
{
    if (!m_device) return;
    if (tokens.size() != 2) {
        print("Использование: route <IPv4>");
        return;
    }

    const auto decision = m_device->selectRoute(tokens[1]);
    if (decision.type == RouteDecisionType::NoRoute) {
        print(decision.error);
        return;
    }

    const QString name = m_device->getPorts()[decision.portIndex].name;
    if (decision.type == RouteDecisionType::Local) {
        print(QStringLiteral("Локальная доставка: %1").arg(name));
    }
    else {
        print(QStringLiteral("Интерфейс: %1").arg(name));
        print(QStringLiteral("Следующий узел: %1").arg(decision.nextHopIp));
        print(decision.type == RouteDecisionType::Direct
            ? QStringLiteral("Прямая доставка в подключённую сеть.")
            : QStringLiteral("Доставка через шлюз по умолчанию."));
    }
    print(QStringLiteral("Это выбор маршрута, а не проверка доступности."));
}

void TerminalWidget::cmdShowMacTable(const QStringList& tokens)
{
    if (!m_device) return;

    // Проверка корректности синтаксиса команды
    if (tokens[0].toLower() == "show" && !tokens.value(1).toLower().startsWith("mac")) {
        print("Использование: show mac-address-table");
        print("");
        return;
    }

    // Команда доступна только для L2-коммутаторов
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

    // ARP не используется на чистых L2-коммутаторах
    if (m_device->getType() == DeviceType::Switch) {
        print("Ошибка: коммутатор (L2 Switch) не использует протокол ARP.");
        print("Для просмотра таблицы коммутации используйте: show mac-address-table");
        print("");
        return;
    }

    ArpTable* arp = m_device->getArpTable();
    if (!arp) return;

    // Обработка флага очистки кэша
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

    // Формирование табличного вывода ARP-записей
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

    print(QStringLiteral("Внимание: legacy ping ещё использует BFS, а не новую маршрутизацию/ARP/ICMP."));
    QStringList logLines;

    // Выполнение пинга и получение лог-строк от устройства
    m_device->ping(tokens[1].trimmed(), logLines);
    for (const QString& line : logLines)
        print(line);
    print("");
}