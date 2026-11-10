#include "ConnectionDialog.h"
#include <QVBoxLayout>
#include <QFormLayout> 
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
ConnectionDialog::ConnectionDialog(NetworkDevice* dev1, NetworkDevice* dev2, QWidget* parent)
    : QDialog(parent)
{
    setupUI(dev1, dev2);
}

void ConnectionDialog::setupUI(NetworkDevice* dev1, NetworkDevice* dev2)
{
    setWindowTitle("Параметры соединения");
    setModal(true);
    setMinimumWidth(380);

    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
            font-family: 'Segoe UI', sans-serif;
            font-size: 13px;
        }
        QLabel {
            color: #334155;
            font-size: 13px;
        }
        QComboBox {
            background-color: #F8FAFC;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 6px 12px;
            color: #0F172A;
        }
        QComboBox:hover {
            border-color: #3B82F6;
            background-color: #FFFFFF;
        }
        /* Всплывающее меню комбобокса */
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            selection-background-color: #EFF6FF;
            selection-color: #1D4ED8;
            outline: none;
            padding: 4px;
        }

        /* Синяя главная кнопка "Подключить" */
        QPushButton#primaryBtn {
            background-color: #2563EB;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 18px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton#primaryBtn:hover {
            background-color: #1D4ED8;
        }
        QPushButton#primaryBtn:pressed {
            background-color: #1E40AF;
        }

        /* Серая кнопка "Отмена" */
        QPushButton#cancelBtn {
            background-color: #F1F5F9;
            color: #475569;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 8px 18px;
            font-size: 13px;
        }
        QPushButton#cancelBtn:hover {
            background-color: #E2E8F0;
            color: #0F172A;
        }
    )");


    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20); // Аккуратные отступы от краев
    mainLayout->setSpacing(14);

    // Карточка заголовка с цветным бейджем
    QLabel* headerLabel = new QLabel(QString(
        "<div style='font-size: 14px; font-weight: 600; color: #0F172A;'>"
        "🔗 Соединение устройств</div>"
        "<div style='color: #64748B; font-size: 12px; margin-top: 2px;'>"
        "<b>%1</b> &lt;─────&gt; <b>%2</b></div>")
        .arg(dev1->getName(), dev2->getName()), this);
    mainLayout->addWidget(headerLabel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    //Выпадающий список типов кабеля
    m_cableTypeCombo = new QComboBox(this);
    m_cableTypeCombo->addItem(QStringLiteral("🔵 Витая пара (Медь, 1G)"), static_cast<int>(CableType::TwistedPair));
    m_cableTypeCombo->addItem(QStringLiteral("🟠 Оптоволокно (10G)"), static_cast<int>(CableType::OpticalFiber));
    m_cableTypeCombo->addItem(QStringLiteral("⚪ Коаксиальный кабель"), static_cast<int>(CableType::CoaxialCable));
    m_cableTypeCombo->addItem(QStringLiteral("🔷 Консольный кабель"), static_cast<int>(CableType::ConsoleCable));
    formLayout->addRow(QStringLiteral("Тип среды:"), m_cableTypeCombo);

    //Порты первого устройства
    m_port1Combo = new QComboBox(this);
    for (const auto& port : dev1->getFreePorts()) {
        m_port1Combo->addItem(port.name);
    }
    formLayout->addRow(QString("Порт %1:").arg(dev1->getName()), m_port1Combo);

    //Порты второго устройства
    m_port2Combo = new QComboBox(this);
    for (const auto& port : dev2->getFreePorts()) {
        m_port2Combo->addItem(port.name);
    }
    formLayout->addRow(QString("Порт %1:").arg(dev2->getName()), m_port2Combo);

    mainLayout->addLayout(formLayout);

    // Кнопки Подключить / Отмена
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    QPushButton* okBtn = buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn) {
        okBtn->setText(QStringLiteral("Подключить (Enter)"));
        okBtn->setDefault(true);
        // ЗАДАЕМ СИНИЙ СТИЛЬ НАПРЯМУЮ:
        okBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #2563EB;"
            "   color: #FFFFFF;"
            "   border: none;"
            "   border-radius: 6px;"
            "   padding: 8px 18px;"
            "   font-weight: 600;"
            "}"
            "QPushButton:hover { background-color: #1D4ED8; }"
            "QPushButton:pressed { background-color: #1E40AF; }"
        );
    }

    QPushButton* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    if (cancelBtn) {
        cancelBtn->setText(QStringLiteral("Отмена"));
        cancelBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #F1F5F9;"
            "   color: #475569;"
            "   border: 1px solid #CBD5E1;"
            "   border-radius: 6px;"
            "   padding: 8px 18px;"
            "}"
            "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }"
        );
    }

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

CableType ConnectionDialog::getSelectedCableType() const
{
    return static_cast<CableType>(m_cableTypeCombo->currentData().toInt());
}

QString ConnectionDialog::getSelectedPort1() const
{
    return m_port1Combo->currentText();
}

QString ConnectionDialog::getSelectedPort2() const
{
    return m_port2Combo->currentText();
}