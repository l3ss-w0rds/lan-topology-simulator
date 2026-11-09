#pragma once
#include <QGraphicsItem>
#include <QPen>


class NodeItem;

enum class CableType{
TwistedPair, //Витая пара(Медь, 1Гб/с, черный)
OpticalFiber, //Оптоволокно (10Гб/с, оранжевый)
CoaxialCable, //Коаксиалка (темно-синий)
ConsoleCable //Консольный (голубой)
};
class CableItem : public QGraphicsLineItem {
	public:
		CableItem(NodeItem* startNode, NodeItem* endNode, CableType type, QGraphicsItem* parent = nullptr);
		~CableItem() override;

		void trackNodes(); //пересчитываем координаты линии между центрами узлов

		NodeItem* getStartNode() const { return m_startNode; }
		NodeItem* getEndNode() const { return m_endNode; }
		CableType getCableType() const { return m_type; }

private:
	QPainterPath shape() const override; //для выделения кабеля мышкой
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; //рисуем кабель
	NodeItem* m_startNode;
	NodeItem* m_endNode;
	CableType m_type;
};