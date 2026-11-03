#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include "NetworkDevice.h"

class CableItem;

class NodeItem : public QGraphicsItem //объединяет в себе графику и сетевые данные
{
public:
	explicit NodeItem(NetworkDevice* device, QGraphicsItem* parent = nullptr);
	~NodeItem() override;

	QRectF boundingRect() const override; //Возврощает прямоугольник габарита узалов
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; //Рисует фигурку, иконку и подписи на сцене
	void addCable(CableItem* cable); //Добавляет кабель к узлу
	void removeCable(CableItem* cable); //Удаляет кабель из узла

	NetworkDevice* getDevice() const { return m_device; }
	const QList<CableItem*>& getCables() const { return m_cables; }
protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override; //датчик движения в Qt
private:
	NetworkDevice* m_device;
	QList<CableItem*> m_cables;

};

