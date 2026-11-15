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
    }

    QPushButton* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    if (cancelBtn) {
        cancelBtn->setText(QStringLiteral("Отмена"));
    }

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}
