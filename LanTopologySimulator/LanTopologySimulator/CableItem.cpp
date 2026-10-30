#include "CableItem.h"
#include "NodeItem.h"

CableItem::CableItem(NodeItem* startNode, NodeItem* endNode, CableType type, QGraphicsItem* parent)
	: QGraphicsLineItem(parent),
	m_startNode(startNode),
	m_endNode(endNode),
	m_type(type)
{
	QPen pen;
	pen.setWidth(3);
	pen.setCapStyle(Qt::RoundCap);

	switch (m_type) {
	case CableType::TwistedPair:
		pen.setColor(QColor("#2563EB"));
		break;
	case CableType::OpticalFiber:
		pen.setColor(QColor("#F59E0B"));
		break;
	case CableType::CoaxialCable:
		pen.setColor(QColor("#1E293B"));
		break;
	case CableType::ConsoleCable:
		pen.setColor(QColor("#06B6D4"));
		break;
	}
	setFlags(QGraphicsItem::ItemIsSelectable);
	setZValue(-1);
}
CableItem::~CableItem() {
}
void CableItem::trackNodes() {
	if (!m_startNode || !m_endNode) return;
	prepareGeometryChange(); //обновление индекса сцены

	QPointF p1 = m_startNode->sceneBoundingRect().center();
	QPointF p2 = m_endNode->sceneBoundingRect().center();
	setLine(QLineF(p1, p2)); //Проводим линию между центрами узлов
}
QPainterPath CableItem::shape() const{
	QPainterPath path;
	path.moveTo(line().p1());
	path.lineTo(line().p2());

	QPainterPathStroker stroker;
	stroker.setWidth(12); //ширина выделения мышкой	
	return stroker.createStroke(path);
}

void CableItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (line().length() < 1.0) return;

	// Включаем сглаживание
	painter->setRenderHint(QPainter::Antialiasing, true);

	QPen pen;
	pen.setWidth(3);
	pen.setCapStyle(Qt::RoundCap); // Скругленные аккуратные концы

	if (isSelected()) {
		pen.setColor(Qt::red); // Цвет выделения
		pen.setStyle(Qt::DashLine); // Пунктирная линия при выделении
	}
	else {
		pen.setColor(m_type == CableType::TwistedPair ? QColor("#2563EB") :
			m_type == CableType::OpticalFiber ? QColor("#F59E0B") :
			m_type == CableType::CoaxialCable ? QColor("#1E293B") :
			QColor("#06B6D4")); // Цвет кабеля в зависимости от типа
	}
	painter->setPen(pen);
	painter->drawLine(line());
}

