#include "NetworkDevice.h"


NetworkDevice::NetworkDevice(const QString& name, const QString& ip, DeviceType type)
	: m_name(name), 
	m_ip(ip), 
	m_mac(generateRandomMac()), 
	m_type(type), 
	m_isOnline(true)
{
	initPorts();
	if (m_type == DeviceType::Switch) { //Создаем таблицу для свитча
		m_macTable = new MacTable(300);
	}
	else {//Для ПК, Сервера и Роутера, создаем Arp-таблица
		m_arpTable = new ArpTable(120); 
	}
}
NetworkDevice::~NetworkDevice() { 
	delete m_macTable;
	delete m_arpTable;
}

QString NetworkDevice::generateRandomMac() {
	std::random_device rd; //Источник энтропии
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 255);

	uint8_t mac[6];
	for (int i = 0; i < 6; ++i) {
		mac[i] = static_cast<uint8_t>(dis(gen));
	}
	mac[0] &= 0xFC; // Устанавливаем первый бит в 0 (1байт четный), а второй не 2,6,10....
	//mac[0] |= 0x02; // Устанавливаем второй бит в 1


	QString macStr = QString("%1:%2:%3:%4:%5:%6")
		.arg(mac[0], 2, 16, QChar('0'))
		.arg(mac[1], 2, 16, QChar('0'))
		.arg(mac[2], 2, 16, QChar('0'))
		.arg(mac[3], 2, 16, QChar('0'))
		.arg(mac[4], 2, 16, QChar('0'))
		.arg(mac[5], 2, 16, QChar('0'))
		.toUpper();
	qDebug() << "Generated MAC: " << macStr;
	return macStr;
}

void NetworkDevice::initPorts(){
	m_ports.clear();
	switch (m_type) {
		case DeviceType::PC:
			m_ports.append({ "eth0", false, nullptr, "" });
		break;
		case DeviceType::Server:
			m_ports.append({ "eth0", false, nullptr, "" });
			m_ports.append({ "eth1", false, nullptr, "" });
		break;
		case DeviceType::Switch:
			for (int i = 0; i < 8; ++i) {
				m_ports.append({ QString("eth%1").arg(i), false, nullptr, "" });
			}
		break;
		case DeviceType::Router:
			for (int i = 0; i < 4; ++i) {
				m_ports.append({ QString("eth%1").arg(i), false, nullptr, "" });
			}
		break;
	}

}

QVector<NetworkPort> NetworkDevice::getFreePorts() const {
	QVector<NetworkPort> freePorts;
	for (const auto& port : m_ports) {
		if (!port.isOccupied) {
			freePorts.append(port);
		}
	}
	return freePorts;
}

bool NetworkDevice::hasFreePorts() const {
	for (const auto& port : m_ports) {
		if (!port.isOccupied) {
			return true;
		}
	}
	return false;
}

bool NetworkDevice::connectPort(const QString& myPortName, NetworkDevice* otherDevice, const QString& otherPortName) {
	for (auto& port : m_ports) {
		if (port.name == myPortName && !port.isOccupied) {
			port.isOccupied = true;
			port.connectedDevice = otherDevice;
			port.connectedPortName = otherPortName;
			return true;
		}
	}
	return false;
}

void NetworkDevice::disconnectPort(const QString& myPortName) {
	for (auto& port : m_ports) {
		if (port.name == myPortName && port.isOccupied) {
			port.isOccupied = false;
			port.connectedDevice = nullptr;
			port.connectedPortName = "";
			return;
		}
	}
}

void NetworkDevice::disconnectedFrom(NetworkDevice* other) {
	if (!other) return;
	//Освобождаем порт на нашем устрве, который вел к other
	for (auto& port : m_ports) {
		if (port.connectedDevice == other) {
			port.isOccupied = false;
			port.connectedDevice = nullptr;
			port.connectedPortName = "";
		}
	}
	//Освобождаем порт на other, который вел к нам
	for (auto& port : other->m_ports) {
		if (port.connectedDevice == this) {
			port.isOccupied = false;
			port.connectedDevice = nullptr;
			port.connectedPortName = "";
		}
	}

}

int NetworkDevice::getPortIndexConnectedTo(NetworkDevice* targetDevice) const
{
	for (int i = 0; i < m_ports.size(); ++i) {
		if (m_ports[i].isOccupied && m_ports[i].connectedDevice == targetDevice) {
			return i;
		}
	}
	return -1;
}


bool NetworkDevice::ping(const QString& targetIp, QStringList& outLog)
{
	QString cleanTarget = targetIp.trimmed();
	outLog.clear();

	//Проверка пинга самого себя (Loopback)
	if (cleanTarget == m_ip) {
		outLog << QStringLiteral("Обмен пакетами с ") + cleanTarget + QStringLiteral(" по 32 байт:");
		for (int i = 0; i < 4; ++i) {
			outLog << QStringLiteral("Ответ от ") + cleanTarget + QStringLiteral(": число байт=32 время<1мс TTL=128");
		}
		outLog << QStringLiteral("\nСтатистика Ping для ") + cleanTarget + QStringLiteral(":");
		outLog << QStringLiteral("    Пакетов: отправлено = 4, получено = 4, потеряно = 0 (0% потерь)");
		return true;
	}

	outLog << QStringLiteral("Обмен пакетами с ") + cleanTarget + QStringLiteral(" по 32 байт:");

	//Поиск пути (BFS) - оставляем, чтобы понять, куда вообще идти
	QQueue<NetworkDevice*> queue;
	QSet<NetworkDevice*> visited;
	QMap<NetworkDevice*, NetworkDevice*> parentMap; // Упростили: просто кто от кого пришел

	queue.enqueue(this);
	visited.insert(this);
	NetworkDevice* targetDevice = nullptr;

	while (!queue.isEmpty()) {
		NetworkDevice* current = queue.dequeue();

		if (current->getIP() == cleanTarget && current != this) {
			targetDevice = current;
			break;
		}

		// Конечные устройства не транзитят трафик (кроме случая, когда они цель)
		if (current != this && (current->getType() == DeviceType::PC || current->getType() == DeviceType::Server)) {
			continue;
		}

		for (int i = 0; i < current->m_ports.size(); ++i) {
			const auto& port = current->m_ports[i];
			if (port.isOccupied && port.connectedDevice) {
				NetworkDevice* neighbor = port.connectedDevice;
				if (!visited.contains(neighbor)) {
					visited.insert(neighbor);
					parentMap[neighbor] = current;
					queue.enqueue(neighbor);
				}
			}
		}
	}

	//Если путь не найден
	if (!targetDevice) {
		for (int i = 0; i < 4; ++i) outLog << QStringLiteral("Превышен интервал ожидания для запроса.");
		outLog << QStringLiteral("\nСтатистика Ping для ") + cleanTarget + QStringLiteral(":");
		outLog << QStringLiteral("    Пакетов: отправлено = 4, получено = 0, потеряно = 4 (100% потерь)");
		return false;
	}

//ПРАВИЛЬНАЯ ПОШАГОВАЯ (HOP-BY-HOP) СИМУЛЯЦИЯ
	// Восстанавливаем путь от цели к источнику, чтобы перевернуть его (от источника к цели)
	QVector<NetworkDevice*> path;
	NetworkDevice* curr = targetDevice;
	while (curr != nullptr) {
		path.prepend(curr); // Добавляем в начало, чтобы путь шел от this к targetDevice
		curr = parentMap.value(curr, nullptr);
	}

	bool firstPacketLostDueToArp = false; // Флаг для симуляции потери первого пакета

	// Проходим по пути: от источника (path[0]) к цели (path.last())
	for (int i = 0; i < path.size() - 1; ++i) {
		NetworkDevice* currentHop = path[i];
		NetworkDevice* nextHop = path[i + 1];

		// А) Если текущий узел - Коммутатор (L2), он учит MAC-адрес ИСТОЧНИКА на входящем порту
		if (currentHop->getType() == DeviceType::Switch && currentHop->getMacTable()) {
			int incomingPort = currentHop->getPortIndexConnectedTo(i == 0 ? this : path[i - 1]);
			if (incomingPort != -1) {
				currentHop->getMacTable()->learn(this->getMac(), incomingPort);
			}
		}

		// Б) Если текущий узел - Роутер или ПК (L3), он ДОЛЖЕН знать MAC следующего узла!
		if ((currentHop->getType() == DeviceType::Router || currentHop->getType() == DeviceType::PC || currentHop->getType() == DeviceType::Server)
			&& currentHop->getArpTable()) {

			if (!currentHop->getArpTable()->hasEntry(nextHop->getIP())) {
				//симулируем успешный ответ и запись в таблицу.
				currentHop->getArpTable()->addEntry(nextHop->getIP(), nextHop->getMac());
				// Если это самый первый хоп (от источника), и ARP не было, первый пакет ICMP теряется (классика Cisco)
				if (i == 0) {
					firstPacketLostDueToArp = true;
				}
			}
		}
	}

	//ФОРМИРУЕМ ОТВЕТ С ЧЕСТНЫМ УЧЕТОМ ARP-ЗАДЕРЖКИ:
	if (firstPacketLostDueToArp) {
		// Первый пинг: ПЕРВЫЙ ПАКЕТ ТЕРЯЕТСЯ ИЗ-ЗА ARP-ЗАПРОСА (25% потерь)!
		outLog << QStringLiteral("Превышен интервал ожидания для запроса.");
		for (int i = 0; i < 3; ++i) {
			outLog << QStringLiteral("Ответ от ") + cleanTarget + QStringLiteral(": число байт=32 время=1мс TTL=64");
		}
		outLog << QStringLiteral("\nСтатистика Ping для ") + cleanTarget + QStringLiteral(":");
		outLog << QStringLiteral("    Пакетов: отправлено = 4, получено = 3, потеряно = 1 (25% потерь)");
	}
	else {
		// Повторный пинг: ARP уже в кэше, ВСЕ 4 ПАКЕТА ПРОХОДЯТ (0% потерь)!
		for (int i = 0; i < 4; ++i) {
			outLog << QStringLiteral("Ответ от ") + cleanTarget + QStringLiteral(": число байт=32 время=1мс TTL=64");
		}
		outLog << QStringLiteral("\nСтатистика Ping для ") + cleanTarget + QStringLiteral(":");
		outLog << QStringLiteral("    Пакетов: отправлено = 4, получено = 4, потеряно = 0 (0% потерь)");
	}

	return true;
}