#include "CableItem.h"
#include "NodeItem.h"

bool CableItem::s_showPorts = true; //по умолчанию порты видны

CableItem::CableItem(NodeItem* startNode, NodeItem* endNode, CableType type, const QString startPort, const QString endPort, QGraphicsItem* parent)
	: QGraphicsLineItem(parent),
	m_startNode(startNode),
	m_endNode(endNode),
	m_type(type),
	m_startPort(startPort),
	m_endPort(endPort)
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
	//Освобождаем порты в МОДЕЛИ при удалении кабеля
	if (m_startNode && m_endNode && m_startNode->getDevice() && m_endNode->getDevice()) {
		m_startNode->getDevice()->disconnectedFrom(m_endNode->getDevice());
	}
	//Отвязываем провод от узлов в ГРАФИКЕ
	if (m_startNode) {
		m_startNode->removeCable(this);
	}
	if (m_endNode) {
		m_endNode->removeCable(this);
	}
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

	QLineF curLine = line();
	qreal len = curLine.length();
	if (len < 1.0) return;

	// Включаем сглаживание
	painter->setRenderHint(QPainter::Antialiasing, true);
//ОТРИСОВКА САМОЙ ЛИНИИ КАБЕЛЯ
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

//ОТРИСОВКА СМАРТ-КАПСУЛ ПОРТОВ
	if ((s_showPorts || isSelected()) && len > 95.0) {
		qreal offset = 40.0; // Отступ от центра устройства к краю иконки

		// Точка первого порта
		QPointF p1 = curLine.pointAt(offset / len);
		drawPortBadge(painter, p1, m_startPort);

		// Точка второго порта
		QPointF p2 = curLine.pointAt((len - offset) / len);
		drawPortBadge(painter, p2, m_endPort);
	}
}

QRectF CableItem::boundingRect() const
{
	qreal extra = 35.0;
	return QGraphicsLineItem::boundingRect().adjusted(-extra, -extra, extra, extra);
}

void CableItem::drawPortBadge(QPainter* painter, const QPointF& centerPos, const QString& portName) {
	if (portName.isEmpty()) return;

	painter->save();

	//Настройка шрифта
	QFont font("Segoe UI", 8, QFont::DemiBold);
	painter->setFont(font);
	QFontMetrics fm(font);

	int textWidth = fm.horizontalAdvance(portName);
	int textHeight = fm.height();

	// Размеры капсулы (с учетом зеленой точки и отступов)
	int badgeWidth = textWidth + 18;
	int badgeHeight = textHeight + 4;

	QRectF badgeRect(centerPos.x() - badgeWidth / 2.0,
		centerPos.y() - badgeHeight / 2.0,
		badgeWidth, badgeHeight);

	//Белая плашка с мягкой серой границей
	painter->setPen(QPen(QColor("#CBD5E1"), 1.0));
	painter->setBrush(QBrush(QColor("#FFFFFF")));
	painter->drawRoundedRect(badgeRect, 4.0, 4.0);

	//Зеленый светодиод линка
	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(QColor("#10B981"))); //Изумрудно-зеленый
	QPointF dotCenter(badgeRect.left() + 7, badgeRect.center().y());
	painter->drawEllipse(dotCenter, 2.5, 2.5);

	//Текст названия порта
	painter->setPen(QPen(QColor("#0F172A"))); // Глубокий графитовый цвет текста
	QRectF textRect(badgeRect.left() + 13, badgeRect.top(), textWidth + 2, badgeHeight);
	painter->drawText(textRect, Qt::AlignCenter, portName);

	painter->restore();
}
