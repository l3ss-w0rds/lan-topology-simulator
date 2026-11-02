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
    setMinimumWidth(340);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Заголовок: кто с кем соединяется
    QLabel* headerLabel = new QLabel(QString("<b>Соединение:</b> %1 &lt;---&gt; %2")
        .arg(dev1->getName(), dev2->getName()), this);
    mainLayout->addWidget(headerLabel);
    mainLayout->addSpacing(10);

    QFormLayout* formLayout = new QFormLayout();

    // 1. Выпадающий список типов кабеля
    m_cableTypeCombo = new QComboBox(this);
    m_cableTypeCombo->addItem("Витая пара (Медь, 1G)", static_cast<int>(CableType::TwistedPair));
    m_cableTypeCombo->addItem("Оптоволокно (10G)", static_cast<int>(CableType::OpticalFiber));
    m_cableTypeCombo->addItem("Коаксиальный кабель", static_cast<int>(CableType::CoaxialCable));
    m_cableTypeCombo->addItem("Консольный кабель", static_cast<int>(CableType::ConsoleCable));
    formLayout->addRow("Тип кабеля:", m_cableTypeCombo);

    // 2. Порты первого устройства (только свободные!)
    m_port1Combo = new QComboBox(this);
    for (const auto& port : dev1->getFreePorts()) {
        m_port1Combo->addItem(port.name);
    }
    formLayout->addRow(QString("Порт %1:").arg(dev1->getName()), m_port1Combo);

    // 3. Порты второго устройства (только свободные!)
    m_port2Combo = new QComboBox(this);
    for (const auto& port : dev2->getFreePorts()) {
        m_port2Combo->addItem(port.name);
    }
    formLayout->addRow(QString("Порт %1:").arg(dev2->getName()), m_port2Combo);

    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(15);

    // Кнопки Подключить (Enter)/Отмена (Esc)
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    buttonBox->button(QDialogButtonBox::Ok)->setText("Подключить (Enter)");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    // Делаем кнопку Ok кнопкой по умолчанию по нажатию Enter
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

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