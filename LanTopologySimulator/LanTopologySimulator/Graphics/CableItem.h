#pragma once
#include <QGraphicsItem>
#include <QPen>
#include <QFontMetrics>


class NodeItem;

enum class CableType{
TwistedPair, //Витая пара(Медь, 1Гб/с, черный)
OpticalFiber, //Оптоволокно (10Гб/с, оранжевый)
CoaxialCable, //Коаксиалка (темно-синий)
ConsoleCable //Консольный (голубой)
};
class CableItem : public QGraphicsLineItem {
	public:
		CableItem(NodeItem* startNode, NodeItem* endNode, CableType type,
			      const QString startPort = "", const QString endPort = "",
			      QGraphicsItem* parent = nullptr);
		~CableItem() override;

		void trackNodes(); //пересчитываем координаты линии между центрами узлов

		NodeItem* getStartNode() const { return m_startNode; }
		NodeItem* getEndNode() const { return m_endNode; }
		CableType getCableType() const { return m_type; }
		//Управление видимостью портов, созданы в единственн. экземпляре
		static bool s_showPorts; //флаг видимости портов
		static void setShowPorts(bool show) { s_showPorts = show; } //устанавливаем видимость портов
		static bool isShowPorts() { return s_showPorts; } //получаем видимость портов
		static void drawPortBadge(QPainter* painter, const QPointF& centerPos, const QString& portName); //отрисовка одной смарт-капсулы порта

		QPainterPath shape() const override; //для выделения кабеля мышкой
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; //рисуем кабель
		QRectF boundingRect() const override; //границы кабеля для сцены



private:
	NodeItem* m_startNode;
	NodeItem* m_endNode;
	CableType m_type;

	QString m_startPort;
	QString m_endPort;
};