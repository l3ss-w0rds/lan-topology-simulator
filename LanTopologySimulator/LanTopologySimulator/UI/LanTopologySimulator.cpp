#include "LanTopologySimulator.h"
#include "NetworkDevice.h"
#include "NodeItem.h"
#include "CableItem.h"

LanTopologySimulator::LanTopologySimulator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_scene = new QGraphicsScene(this); //Создаем сцену для отобр объектов(белый лист)
	m_scene->installEventFilter(this); //Устанавливаем фильтр событий для сцены
	ui.canvasView->setScene(m_scene); //Устанавливаем сцену в виджет canvasView
}

LanTopologySimulator::~LanTopologySimulator()
{}

void LanTopologySimulator::on_btnPC_clicked()
{
	static int pcNumber = 0; //Счетчик ПК
	pcNumber++;

	QString name = QString("PC%1").arg(pcNumber); //Имя ПК
	QString ip = QString("192.168.0.%1").arg(10+pcNumber); //IP адрес ПК)

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::PC); //Создаем объект ПК

	NodeItem* node = new NodeItem(device); //Создаем графический объект для ПК
	m_scene->addItem(node); //Добавляем узел на сцену

	logDeviceCreated(device);

}
void LanTopologySimulator::on_btnServer_clicked()
{
	static int serverNumber = 0; //Счетчик серверов
	serverNumber++;

	QString name = QString("Server%1").arg(serverNumber);
	QString ip = QString("192.168.0.%1").arg(20 + serverNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Server);

	NodeItem* node = new NodeItem(device); //Создаем графический объект
	m_scene->addItem(node); //Добавляем узел на сцену
	logDeviceCreated(device);


}
void LanTopologySimulator::on_btnSwitch_clicked()
{
	static int switchNumber = 0; //Счетчик коммутаторов
	switchNumber++;

	QString name = QString("Switch%1").arg(switchNumber);
	QString ip = QString("192.168.0.%1").arg(30 + switchNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Switch);

	NodeItem* node = new NodeItem(device); //Создаем графический объект
	m_scene->addItem(node); //Добавляем узел на сцену
	logDeviceCreated(device);

}
void LanTopologySimulator::on_btnRouter_clicked()
{
	static int routerNumber = 0; //Счетчик маршрутизаторов
	routerNumber++;

	QString name = QString("Router%1").arg(routerNumber);
	QString ip = QString("192.168.0.%1").arg(40 + routerNumber);

	NetworkDevice* device = new NetworkDevice(name, ip, DeviceType::Router);

	NodeItem* node = new NodeItem(device); //Создаем графический объект
	m_scene->addItem(node); //Добавляем узел на сцену
	logDeviceCreated(device);

}


void LanTopologySimulator::logDeviceCreated(const NetworkDevice* device) {

	QString timeStr = QTime::currentTime().toString("hh:mm:ss");

	QString logLine = QString("[%1] [СЕТЬ] Создан %2 | IP: %3 | MAC: %4")
		.arg(timeStr)
		.arg(device->getName())
		.arg(device->getIP())
		.arg(device->getMac());

	ui.logTextEdit->appendPlainText(logLine);

}

void LanTopologySimulator::on_actionCable_triggered()
{
	if (m_firstCableNode) m_firstCableNode->setSelected(false);
	m_firstCableNode = nullptr; //Сбрасываем первый узел для кабеля
	ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Прокладка кабеля. Кликните по первому устройству"));

}

bool LanTopologySimulator::eventFilter(QObject* obj, QEvent* event) {
	if (obj == m_scene && event->type() == QEvent::GraphicsSceneMousePress) {
		if (ui.actionCable && ui.actionCable->isChecked()) {
			auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event); //событие превращаем в клик мыши
			QGraphicsItem *item = m_scene->itemAt(mouseEvent->scenePos(), QTransform()); //что лежит под мышкой, спрашив. у сцены
			while (item && !dynamic_cast<NodeItem*>(item)) {
				item = item->parentItem();
			}

			NodeItem *clickedNode = dynamic_cast<NodeItem*>(item); //проверка
			if (!clickedNode) return QMainWindow::eventFilter(obj, event); 
			handleCableConnection(clickedNode);
			return true; //блокирвоание перетаскивания
		}

	}
	return QWidget::eventFilter(obj, event);
}

void LanTopologySimulator::handleCableConnection(NodeItem* clickedNode){
	if (!clickedNode || !clickedNode->getDevice()) return;
	if (m_firstCableNode == nullptr) {
		m_firstCableNode = clickedNode;
		m_firstCableNode->setSelected(true); //син. рамка
		ui.logTextEdit->appendPlainText("Первый узел выбран:" + m_firstCableNode->getDevice()->getName() + "\n"
		+ " Теперь кликните по второму устройству.");
		return;
	}

	if (m_firstCableNode == clickedNode) {
		ui.logTextEdit->appendPlainText("[ОШИБКА] Нельзя соединить устройство само с собой.");
		return;
	}

	ui.logTextEdit->appendPlainText("[Кабель] Узлы выбраны: " +
	m_firstCableNode->getDevice()->getName() + "<----->" + clickedNode->getDevice()->getName());

	CableItem* cable = new CableItem(m_firstCableNode, clickedNode, CableType::TwistedPair); //Создаем объект кабеля
	m_scene->addItem(cable);

	m_firstCableNode->addCable(cable);
	clickedNode->addCable(cable);
	
	cable->trackNodes();

	ui.logTextEdit->appendPlainText("[Кабель] Соединение установлено");


	m_firstCableNode->setSelected(false);
	m_firstCableNode = nullptr;
}