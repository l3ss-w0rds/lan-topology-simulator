#include "NodeItem.h"


NodeItem::NodeItem(NetworkDevice* device, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_device(device)
{
    // Включаем физику мыши: таскать и выделять
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

NodeItem::~NodeItem()
{
    delete m_device;
}
QRectF NodeItem::boundingRect() const
{
	return QRectF(-35, -25, 70, 75);
}


void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    if (!m_device) return;
    //Сглаживание
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    //Выделение кликом - синий ореол
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(59, 130, 246), 2, Qt::DashLine));
        painter->setBrush(QColor(59, 130, 246, 30)); // полупрозрачный
        painter->drawRoundedRect(QRectF(-24, -19, 48, 38), 6, 6);
    }

	QString iconPath;
    switch (m_device->getType()) {
    case DeviceType::PC:
        iconPath = ":/LanTopologySimulator/node_icons/node_pc_device.svg";
        break;
	case DeviceType::Server:
		iconPath = ":/LanTopologySimulator/node_icons/node_server.svg";
        break;
    case DeviceType::Switch:
		iconPath = ":/LanTopologySimulator/node_icons/node_switch.svg";
        break;
	case DeviceType::Router:
		iconPath = ":/LanTopologySimulator/node_icons/node_router.svg";
        break;

    }


        // РИСУЕМ НАШУ SVG-ИКОНКУ ПО ЦЕНТРУ
    QPixmap iconPixmap = QIcon(iconPath).pixmap(40, 40);
    painter->drawPixmap(-20, -20, iconPixmap);

    // Рисуем текст под фигурой (Имя и IP)
    QFont font = painter->font();
    font.setPointSize(7);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(QColor(15, 23, 42)); // Темно-серый текст

    // Имя устройства (PC1, Switch1...)
    painter->drawText(QRectF(-35, 23, 70, 12), Qt::AlignCenter, m_device->getName());

    // IP-адрес (серым цветом чуть ниже)
    font.setBold(false);
    font.setPointSize(6);
    painter->setFont(font);
    painter->setPen(QColor(100, 116, 139));
    painter->drawText(QRectF(-35, 34, 70, 10), Qt::AlignCenter, m_device->getIP());
}