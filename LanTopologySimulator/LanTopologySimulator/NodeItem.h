#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "NetworkDevice.h"

class NodeItem : public QGraphicsItem //объединяет в себе графику и сетевые данные
{
public:
	explicit NodeItem(NetworkDevice *device, QGraphicsItem *parent = nullptr);
	~NodeItem() override;
	
	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	NetworkDevice* device() const { return m_device; }


private:
	void paintPC(QPainter* painter);
	NetworkDevice* m_device;

}

