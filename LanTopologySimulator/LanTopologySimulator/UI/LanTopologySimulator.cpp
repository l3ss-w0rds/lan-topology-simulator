#include "LanTopologySimulator.h"
#include "NetworkDevice.h"
#include "NodeItem.h"
#include "CableItem.h"
#include "ConnectionDialog.h"

LanTopologySimulator::LanTopologySimulator(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	this->setStyleSheet(R"(
        /* Общий шрифт приложения */
        QWidget {
            font-family: 'Segoe UI', -apple-system, sans-serif;
            font-size: 13px;
        }

        /* Боковая панель "Склад и управление" */
        QGroupBox {
            font-weight: bold;
            font-size: 12px;
            color: #475569;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #FAFAFA;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }

        /* Кнопки добавления устройств слева (ПК, Сервер, Коммутатор, Маршрутизатор) */
        QPushButton {
            background-color: #FFFFFF;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 8px 14px;
            color: #1E293B;
            font-weight: 500;
            text-align: left;
            margin-bottom: 4px;
        }
        QPushButton:hover {
            background-color: #F1F5F9;
            border-color: #94A3B8;
            color: #0F172A;
        }
        QPushButton:pressed {
            background-color: #E2E8F0;
        }

        /* Верхняя панель инструментов (Toolbar) */
        QToolBar {
            background: #FFFFFF;
            border-bottom: 1px solid #E2E8F0;
            padding: 4px;
            spacing: 6px;
        }
        QToolButton {
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 6px;
            background-color: transparent;
        }
        QToolButton:hover {
            background-color: #F1F5F9;
            border: 1px solid #CBD5E1;
        }
        /* Когда инструмент (например, Кабель) нажат/активен */
        QToolButton:checked {
            background-color: #EFF6FF;
            border: 1px solid #3B82F6;
        }

        /* Окно журнала событий (Лог) */
        QPlainTextEdit {
            background-color: #F8FAFC;
            color: #334155;
            font-family: 'Consolas', 'Cascadia Code', monospace;
            font-size: 12px;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            padding: 8px;
        }
    )");
	//Делаем кнопки переключаемыми
	ui.actionCable->setCheckable(true);
	ui.actionDelete->setCheckable(true);
	if (ui.actionCursor) {
		ui.actionCursor->setCheckable(true);
		ui.actionCursor->setChecked(true); // Со старта активна стрелка
	}

	ui.canvasView->setDragMode(QGraphicsView::RubberBandDrag); // Включаем рамку выделения со старта!
	ui.canvasView->setRubberBandSelectionMode(Qt::IntersectsItemShape); // Выделять всё, чего коснулась рамка

	//Точечная сетка на холсте с созданием сцены
	const int gridSize = 30; // Шаг 30 px — просторно и удобно
	QPixmap gridPixmap(gridSize, gridSize);
	gridPixmap.fill(QColor("#F8FAFC")); // Мягкий фон

	QPainter gridPainter(&gridPixmap);
	gridPainter.setRenderHint(QPainter::Antialiasing); // Сглаживание кружочков
	gridPainter.setPen(Qt::NoPen);                     // Без контура
	gridPainter.setBrush(QColor("#94A3B8"));           // Заметный аккуратный серый цвет
	gridPainter.drawEllipse(gridSize / 2 - 1, gridSize / 2 - 1, 2, 2); // Кружок диаметром 2px!

	ui.canvasView->setBackgroundBrush(QBrush(gridPixmap));
	ui.canvasView->setRenderHint(QPainter::Antialiasing);


	m_scene = new QGraphicsScene(this);
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


void LanTopologySimulator::resetCableState() {
	// Выключаем режим кабеля, если он был включен
	if (m_firstCableNode) {
		m_firstCableNode->setSelected(false);
		m_firstCableNode = nullptr;
	}
}

void LanTopologySimulator::on_actionCable_triggered()
{
	resetCableState();
	if (!ui.actionCable->isChecked()) {
		if (ui.actionCursor) ui.actionCursor->setChecked(true);  // Возвращаем стрелку
		on_actionCursor_triggered();
		return;
	}
	if (ui.actionDelete) ui.actionDelete->setChecked(false);
	if (ui.actionCursor) ui.actionCursor->setChecked(false);

	ui.canvasView->setDragMode(QGraphicsView::NoDrag);

	if (ui.actionCable->isChecked()) {
		ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Прокладка кабеля. Кликните по первому устройству."));
	}
	else {
		ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Обычный указатель."));
	}

}

void LanTopologySimulator::on_actionDelete_triggered()
{
	resetCableState();
	if (!ui.actionDelete->isChecked()) {
		if (ui.actionCursor) ui.actionCursor->setChecked(true);  // Возвращаем стрелку
		on_actionCursor_triggered();
		return;
	}

	if (ui.actionCable) ui.actionCable->setChecked(false);
	if (ui.actionCursor) ui.actionCursor->setChecked(false);

	ui.canvasView->setDragMode(QGraphicsView::NoDrag);

	//если пользователь выделил объекты и нажал на корзину — удаляем их сразу
	if (!m_scene->selectedItems().isEmpty()) {
		deleteSelectedItems();
		ui.actionDelete->setChecked(false); // Отжимаем корзину обратно
		ui.actionCursor->setChecked(true);  // Возвращаем стрелку
		ui.canvasView->setDragMode(QGraphicsView::RubberBandDrag);
		return;
	}
	if (ui.actionDelete->isChecked()) {
		ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Удаление кликом включено. Кликайте по объектам для удаления."));
	}
	else {
		ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Обычный указатель."));
	}
}

void LanTopologySimulator::on_actionCursor_triggered() {
	resetCableState();
	if (ui.actionCursor) ui.actionCursor->setChecked(true);
	if (ui.actionCable) ui.actionCable->setChecked(false);
	if (ui.actionDelete) ui.actionDelete->setChecked(false);

	ui.canvasView->setDragMode(QGraphicsView::RubberBandDrag);
	ui.canvasView->setCursor(Qt::ArrowCursor);

	ui.logTextEdit->appendPlainText(QStringLiteral("[РЕЖИМ] Указатель. Можно выделять область и перемещать узлы."));
}

bool LanTopologySimulator::eventFilter(QObject* obj, QEvent* event) {
	//Ловим клавишу delete и backspace для удаления выделенных объектов
	if (obj == m_scene && event->type() == QEvent::KeyPress) {
		auto* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
			deleteSelectedItems();
			return true;
		}
	}
	//Ловим двойной клик мыши на сцене для открытия диалога свойств устройства
	if (obj == m_scene && event->type() == QEvent::GraphicsSceneMouseDoubleClick) {
		auto* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
		QGraphicsItem* item = m_scene->itemAt(mouseEvent->scenePos(), QTransform());

		// Ищем, есть ли под курсором NodeItem
		while (item && !dynamic_cast<NodeItem*>(item)) {
			item = item->parentItem();
		}

		if (auto* node = dynamic_cast<NodeItem*>(item)) {
			if (node->getDevice()) {
				// Открываем диалог модально (блокирует ввод до закрытия)
				DevicePropertiesDialog dialog(node->getDevice(), this);
				dialog.exec();
				return true; // Событие обработано!
			}
		}
	}

	//Ловим клики мыши на сцене для удаления объектов или прокладки кабеля
	if (obj == m_scene && event->type() == QEvent::GraphicsSceneMousePress) {
		auto* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
		QGraphicsItem* item = m_scene->itemAt(mouseEvent->scenePos(), QTransform());

		//Режим КОРЗИНЫ (удаление кликом)
		if (ui.actionDelete && ui.actionDelete->isChecked()) {
			if (!item) return true; // Кликнули в пустое место — ничего не делаем

			// Ищем узел или кабель под курсором
			while (item && !dynamic_cast<NodeItem*>(item) && !dynamic_cast<CableItem*>(item)) {
				item = item->parentItem();
			}
			// Если кликнули по кабелю — удаляем его мгновенно
			if (auto* cable = dynamic_cast<CableItem*>(item)) {
				delete cable;
				ui.logTextEdit->appendPlainText("[УДАЛЕНИЕ] Кабель удален кликом.");
				return true; // Блокируем дальнейшую обработку
			}
			// Если кликнули по узлу — удаляем узел со всеми его кабелями
			if (auto* node = dynamic_cast<NodeItem*>(item)) {
				if (node == m_firstCableNode) m_firstCableNode = nullptr;
				QString devName = node->getDevice() ? node->getDevice()->getName() : "Устройство";
				delete node;
				ui.logTextEdit->appendPlainText(QString("[УДАЛЕНИЕ] %1 удален кликом.").arg(devName));
				return true; // Блокируем дальнейшую обработку
			}
		}
		// Режим прокладки КАБЕЛЯ
		if (ui.actionCable && ui.actionCable->isChecked()) {
			//auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event); //событие превращаем в клик мыши
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

void LanTopologySimulator::keyPressEvent(QKeyEvent* event)
{
	// Если нажата клавиша Delete или Backspace — запускаем удаление
	if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
		deleteSelectedItems();
		event->accept();
		return;
	}

	QMainWindow::keyPressEvent(event);
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

	NetworkDevice* dev1 = m_firstCableNode->getDevice();
	NetworkDevice* dev2 = clickedNode->getDevice();

	if (!dev1->hasFreePorts()) {
		ui.logTextEdit->appendPlainText(QString("[ОШИБКА] У %1 закончились свободные порты!").arg(dev1->getName()));
		m_firstCableNode->setSelected(false);
		m_firstCableNode = nullptr;
		return;
	}
	if (!dev2->hasFreePorts()) {
		ui.logTextEdit->appendPlainText(QString("[ОШИБКА] У %1 закончились свободные порты!").arg(dev2->getName()));
		m_firstCableNode->setSelected(false);
		m_firstCableNode = nullptr;
		return;
	}

	//Открываем диалог подключения у мышки
	ConnectionDialog dialog(dev1, dev2, this);
	dialog.move(QCursor::pos()); 

	//Если пользователь нажал "Подключить" (или просто Enter)
	if (dialog.exec() == QDialog::Accepted) {
		CableType cableType = dialog.getSelectedCableType();
		QString port1 = dialog.getSelectedPort1();
		QString port2 = dialog.getSelectedPort2();

		//Занимаем порты в МОДЕЛИ
		dev1->connectPort(port1, dev2, port2);
		dev2->connectPort(port2, dev1, port1);

		//Создаем провод на ХОЛСТЕ
		CableItem* cable = new CableItem(m_firstCableNode, clickedNode, cableType);
		m_scene->addItem(cable);
		m_firstCableNode->addCable(cable);
		clickedNode->addCable(cable);
		cable->trackNodes();

		ui.logTextEdit->appendPlainText(QString("[СЕТЬ] Соединено: %1 [%2] &lt;---&gt; %3 [%4]")
			.arg(dev1->getName(), port1, dev2->getName(), port2));
	}
	else {
		ui.logTextEdit->appendPlainText("[ОТМЕНА] Подключение отменено пользователем.");
	}

	m_firstCableNode->setSelected(false);
	m_firstCableNode = nullptr;
}

void LanTopologySimulator::deleteSelectedItems() {
	//Забираем со сцены ВСЁ, что польз выделил мышкой
	QList<QGraphicsItem*> selected = m_scene->selectedItems();
	if (selected.isEmpty()) return;
	m_scene->clearSelection(); //снимаем выделение

	QList<CableItem*> cablesToDelete;
	QList<NodeItem*> nodesToDelete;

	//СНАЧАЛА удаляем отдельно выделенные кабели
	for (QGraphicsItem* item : selected) {
		if (auto* cable = dynamic_cast<CableItem*>(item)) {
			cablesToDelete.append(cable);
		}
		else if (auto* node = dynamic_cast<NodeItem*>(item)) {
			nodesToDelete.append(node);
		}
	}

	//Удаляем кабели, чтобы не оставалось висячих проводов на удаляемых узлах
	for (CableItem* cable : cablesToDelete) {
		delete cable; 
	}
	//Удаляем узлы
	for (NodeItem* node : nodesToDelete) {
		if (node == m_firstCableNode) {
			m_firstCableNode = nullptr;
		}

		QString devName = node->getDevice() ? node->getDevice()->getName() : "Устройство";
		delete node; 

		ui.logTextEdit->appendPlainText(QString("[УДАЛЕНИЕ] %1 удален.").arg(devName));
	}

	if (!cablesToDelete.isEmpty()) {
		ui.logTextEdit->appendPlainText(QString("[УДАЛЕНИЕ] Удалено кабелей: %1.").arg(cablesToDelete.size()));
	}
}