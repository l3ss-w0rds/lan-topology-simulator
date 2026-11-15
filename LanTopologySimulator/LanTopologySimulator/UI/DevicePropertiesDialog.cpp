#include "DevicePropertiesDialog.h"
#include "ui_DevicePropertiesDialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

// Конструктор диалога свойств сетевого устройства
DevicePropertiesDialog::DevicePropertiesDialog(NetworkDevice* device, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::DevicePropertiesDialog),
    m_device(device)
{
    ui->setupUi(this);
    ui->configurationHint->setWordWrap(true);
    ui->footerHint->setWordWrap(true);
    ui->adminUpCheck->setText(QStringLiteral("Включён административно"));
    ui->ipEdit->setPlaceholderText("192.168.1.10");
    ui->maskEdit->setPlaceholderText("255.255.255.0");
    ui->gatewayEdit->setPlaceholderText(QStringLiteral("Пусто — шлюз не задан"));
    ui->okButton->setDefault(true);
    ui->macLabel->setTextInteractionFlags(Qt::TextSelectableByMouse); // Разрешаем выделение MAC-адреса мышью

    //Подключение базовых кнопок управления диалогом
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->okButton, &QPushButton::clicked, this, &DevicePropertiesDialog::saveConfiguration);

    //Проверка валидности указателя на устройство
    if (!m_device) {
        ui->okButton->setEnabled(false); // Блокируем сохранение, если устройство не передано
        return;
    }

    setWindowTitle(QStringLiteral("Свойства: %1").arg(m_device->getName()));

    //Инициализация "черновика" (draft) конфигурации портов. Делаем окпии поротов, чтобы изменения не применились мгновенно
    // а вступали в силу только после нажатия кнопки "ОК".
    m_draft = m_device->getPorts();

    ui->nameEdit->setText(m_device->getName());

    // Заполняем выпадающие списки именами портов
    for (int i = 0; i < m_draft.size(); ++i) {
        ui->interfaceCombo->addItem(m_draft[i].name, i);
        ui->gatewayInterfaceCombo->addItem(m_draft[i].name, i);
    }

    //Загрузка настроек шлюза по умолчанию
    ui->gatewayEdit->setText(m_device->getDefaultGateway());
    const int gatewayIndex = ui->gatewayInterfaceCombo->findData(m_device->getGatewayPortIndex());
    ui->gatewayInterfaceCombo->setCurrentIndex(gatewayIndex < 0 ? 0 : gatewayIndex);

    //Настройка сигналов и слотов для синхронизации UI и черновика (m_draft)

    // При смене интерфейса в комбобоксе: сохраняем изменения предыдущего, 
    // загружаем данные нового и обновляем общую таблицу портов.
    connect(ui->interfaceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) {
            storeSelectedInterface();
            loadSelectedInterface(index);
            loadPortsTable();
        });

    // При завершении ввода IP/маски сохраняем данные в черновик и обновляем таблицу
    connect(ui->ipEdit, &QLineEdit::editingFinished, this, [this]() {
        storeSelectedInterface();
        loadPortsTable();
        });

    connect(ui->maskEdit, &QLineEdit::editingFinished, this, [this]() {
        storeSelectedInterface();
        loadPortsTable();
        });

    // Инициализация полей данными первого порта
    loadSelectedInterface(0);

    //Разделение логики для L2 (коммутатор) и L3 (маршрутизатор/хост) устройств
    const bool l3 = m_device->getType() != DeviceType::Switch;
    ui->interfaceGroup->setVisible(l3);
    ui->gatewayGroup->setVisible(l3);

    if (!l3) {
        ui->configurationHint->setText(QStringLiteral(
            "L2-коммутатор пересылает Ethernet-кадры. IP управления и шлюз пока не реализованы."));
    }

    // Первичное заполнение таблицы портов
    loadPortsTable();

    //Динамическое добавление виджета терминала на соответствующую вкладку
    auto* layout = new QVBoxLayout(ui->terminalTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new TerminalWidget(m_device, ui->terminalTab));
}

DevicePropertiesDialog::~DevicePropertiesDialog() {
    delete ui;
}

void DevicePropertiesDialog::storeSelectedInterface()
{
    if (m_selectedPort < 0 || m_selectedPort >= m_draft.size()) return;

    auto& port = m_draft[m_selectedPort];
    port.ipv4 = ui->ipEdit->text().trimmed();
    port.subnetMask = ui->maskEdit->text().trimmed();
    port.adminUp = ui->adminUpCheck->isChecked();
}

void DevicePropertiesDialog::loadPortsTable() {
    ui->portsTable->setColumnCount(4);
    ui->portsTable->setHorizontalHeaderLabels({
        QStringLiteral("Порт"),
        "IPv4 / маска",
        QStringLiteral("Кабель"),
        QStringLiteral("Подключение")
        });

    ui->portsTable->setRowCount(m_draft.size());
    ui->portsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет прямого редактирования таблицы
    ui->portsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->portsTable->verticalHeader()->hide();

    // Настройка авторазмера колонок: последняя колонка растягивается на всё оставшееся место
    ui->portsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->portsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // Заполнение строк таблицы данными о портах
    for (int i = 0; i < m_draft.size(); ++i) {
        const auto& port = m_draft[i];

        // Формируем строку с информацией о подключенном устройстве (если есть)
        QString peer = QStringLiteral("—");
        if (port.isOccupied && port.connectedDevice)
            peer = port.connectedDevice->getName() + " [" + port.connectedPortName + "]";

        // Формируем массив значений для текущей строки
        const QStringList values = {
            port.name,
            port.ipv4.isEmpty() ? QStringLiteral("—") : port.ipv4 + " / " + port.subnetMask,
            port.isOccupied ? QStringLiteral("Подключён") : QStringLiteral("Нет"),
            peer
        };

        // Создание и добавление ячеек в таблицу
        for (int col = 0; col < values.size(); ++col)
            ui->portsTable->setItem(i, col, new QTableWidgetItem(values[col]));
    }
}

void DevicePropertiesDialog::loadSelectedInterface(int index)
{
    if (index < 0 || index >= m_draft.size()) return;

    m_selectedPort = index;
    const auto& port = m_draft[index];

    ui->ipEdit->setText(port.ipv4);
    ui->maskEdit->setText(port.subnetMask);
    ui->macLabel->setText(port.mac);
    ui->adminUpCheck->setChecked(port.adminUp);
}

// Сначала сохраняет текущий выбранный интерфейс в черновик,
// затем вызывает метод applyConfiguration. В случае ошибки выводит предупреждение.
void DevicePropertiesDialog::saveConfiguration()
{
    if (!m_device) return;

    // Финальное сохранение данных последнего редактируемого порта в черновик
    storeSelectedInterface();

    QString error;
    // Попытка применить конфигурацию к устройству
    if (!m_device->applyConfiguration(ui->nameEdit->text(), m_draft,
        ui->gatewayEdit->text(), ui->gatewayInterfaceCombo->currentData().toInt(), error)) {

        // Обработка ошибки: выводим текст в статусную строку и показываем MessageBox
        ui->saveStatus->setText(error);
        QMessageBox::warning(this, QStringLiteral("Конфигурация не сохранена"), error);
        return;
    }

    // Если всё успешно, закрываем диалог с кодом_ACCEPTED_
    accept();
}