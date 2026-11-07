#include "DevicePropertiesDialog.h"
#include "ui_DevicePropertiesDialog.h"

DevicePropertiesDialog::DevicePropertiesDialog (NetworkDevice* device,QWidget *parent)
	: QDialog(parent), 
	ui(new Ui::DevicePropertiesDialog),
    m_device(device)
{
	ui->setupUi(this);

	if (m_device) {
		setWindowTitle(QString("Свойства устройства: %1").arg(m_device->getName()));
		loadData();
		loadPortsTable();
	}

	connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	connect(ui->okButton, &QPushButton::clicked, this, [this]() {
		if (m_device) {
			// Сохраняем введенные пользователем имя и IP обратно в модель
			m_device->setName(ui->nameEdit->text().trimmed());
			m_device->setIP(ui->ipEdit->text().trimmed());
		}
		accept(); // Закрываем окно с сохранением
		});
}


DevicePropertiesDialog::~DevicePropertiesDialog() {
	delete ui;
}

void DevicePropertiesDialog::loadData() {
	if (!m_device) return;

	ui->nameEdit->setText(m_device->getName());
	ui->ipEdit->setText(m_device->getIP());
	ui->macLabel->setText(QString("<b style='font-family: Consolas; color: #2563EB;'>%1</b>")
		.arg(m_device->getMac()));
}

void DevicePropertiesDialog::loadPortsTable() {
	if (!m_device) return;
	//Настраиваем 3 колонки таблицы портов
	ui->portsTable->setColumnCount(3);
	ui->portsTable->setHorizontalHeaderLabels({ "Интерфейс", "Линк", "Подключение" });
	ui->portsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui->portsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui->portsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui->portsTable->verticalHeader()->setVisible(false);
	ui->portsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//Забираем порты из модели устройства
	const auto& ports = m_device->getPorts();
	ui->portsTable->setRowCount(static_cast<int>(ports.size()));

	for (int i = 0; i < ports.size(); ++i) {
		const auto& p = ports[i];

		// Колонка 0: Имя порта (eth0, Fa0/1 и т.д.)
		ui->portsTable->setItem(i, 0, new QTableWidgetItem(p.name));

		// Колонка 1: Статус линка
		QString statusText = p.isOccupied ? "🟢 UP" : "⚪ DOWN";
		ui->portsTable->setItem(i, 1, new QTableWidgetItem(statusText));

		// Колонка 2: Куда подключен кабель
		QString connText = "Свободен";
		if (p.isOccupied && p.connectedDevice) {
			connText = QString("%1 [%2]").arg(p.connectedDevice->getName(), p.connectedPortName);
		}
		ui->portsTable->setItem(i, 2, new QTableWidgetItem(connText));
	}
}