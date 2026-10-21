#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "NetworkDevice.h"


class NodeItem : public QGraphicsItem //объединяет в себе графику и сетевые данные
{
public:


	explicit NodeItem(NetworkDevice* device, QGraphicsItem* parent = nullptr);
	~NodeItem() override;

	QRectF boundingRect() const override; //Возврощает прямоугольник габарита узалов
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; //Рисует фигурку, иконку и подписи на сцене

	NetworkDevice* getDevice() const { return m_device; }


private:
	NetworkDevice* m_device;

};

