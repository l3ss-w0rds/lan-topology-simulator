#include "DevicePropertiesDialog.h"
#include "ui_DevicePropertiesDialog.h"

DevicePropertiesDialog::DevicePropertiesDialog (NetworkDevice* device,QWidget *parent)
	: QDialog(parent), 
	ui(new Ui::DevicePropertiesDialog),
    m_device(device)
{
	ui->setupUi(this);
    ui->okButton->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; border-radius: 6px; padding: 7px 18px; font-weight: 600; border: none; }"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );

    ui->cancelButton->setStyleSheet(
        "QPushButton { background-color: #F1F5F9; color: #475569; border: 1px solid #CBD5E1; border-radius: 6px; padding: 7px 18px; }"
        "QPushButton:hover { background-color: #E2E8F0; }"
    );

    setStyleSheet(R"(
        /* Общий стиль диалога и типографика */
        QDialog {
            background-color: #FFFFFF;
            font-family: 'Segoe UI', -apple-system, sans-serif;
            font-size: 13px;
        }
        QLabel {
            color: #334155;
            font-size: 13px;
            font-weight: 500;
        }

        /* Вкладки */
        QTabWidget::pane {
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            background-color: #FFFFFF;
            top: -1px;
        }
        QTabBar::tab {
            background: #F8FAFC;
            color: #64748B;
            padding: 8px 18px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 4px;
            font-weight: 500;
        }
        QTabBar::tab:selected {
            background: #FFFFFF;
            color: #2563EB;
            font-weight: 600;
            border: 1px solid #E2E8F0;
            border-bottom: 1px solid #FFFFFF;
        }

        /* Поля ввода (Имя, IP) */
        QLineEdit {
            background-color: #F8FAFC;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 6px 10px;
            color: #0F172A;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 2px solid #2563EB;
            background-color: #FFFFFF;
        }

        /* Таблица интерфейсов */
        QTableWidget {
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            background-color: #FFFFFF;
            gridline-color: #F1F5F9;
            font-size: 12px;
            outline: none;
        }
        QHeaderView::section {
            background-color: #F8FAFC;
            color: #475569;
            font-weight: 600;
            font-size: 12px;
            padding: 7px 10px;
            border: none;
            border-bottom: 1px solid #E2E8F0;
        }

        /* Кнопки */
        QPushButton#okButton {
            background-color: #2563EB;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton#okButton:hover { background-color: #1D4ED8; }
        QPushButton#okButton:pressed { background-color: #1E40AF; }

        QPushButton#cancelButton {
            background-color: #F1F5F9;
            color: #475569;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 8px 18px;
            font-size: 13px;
        }
        QPushButton#cancelButton:hover { background-color: #E2E8F0; color: #0F172A; }
    )");

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

    ui->macLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #F1F5F9;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   padding: 5px 10px;"
        "   font-family: 'Consolas', monospace;"
        "   font-weight: bold;"
        "   color: #2563EB;"
        "}"
    );
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
	ui->portsTable->verticalHeader()->setDefaultSectionSize(32); 

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