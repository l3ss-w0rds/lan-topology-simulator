#include "NodeItem.h"


NodeItem::NodeItem(NetworkDevice* device, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_device(device)
{
    // Включаем физику мыши: таскать и выделять
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

NodeItem::~NodeItem()
{
    delete m_device; // Удаляем модель при удалении с холста
}
QRectF NodeItem::boundingRect() const
{
    // Размеры объекта на сцене
    return QRectF(-35, -25, 70, 75);
}
void NoidItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    if (!m_device) return;

    if(option->state & QStyle::State_Selected) {
        //painter->setPen()
        painter->setPen(QPen(Qt::blue, 2));
    } else {
        painter->setPen(QPen(Qt::black, 1));
	}


    // Рисуем прямоугольник для устройства
    painter->setBrush(Qt::lightGray);
    painter->drawRect(-30, -20, 60, 40);
    // Рисуем имя устройства
    painter->setPen(Qt::black);
    painter->drawText(-30, -25, m_device->getName());
    // Рисуем IP-адрес устройства
    painter->drawText(-30, 35, m_device->getIP());
}