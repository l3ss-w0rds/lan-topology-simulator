#include "NetworkDevice.h"


NetworkDevice::NetworkDevice(const QString& name, const QString& ip, DeviceType type)
	: m_name(name), 
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
	for (auto& port : m_ports) {
		if (m_type != DeviceType::Switch)
			port.mac = generateRandomMac();
	}

	if (m_type != DeviceType::Switch && !ip.trimmed().isEmpty()) {
		QString error;

		if (!configureInterface(0, ip, "255.255.255.0", error)) {
			qWarning() << "Invalid initial interface configuration:"
				<< error;
		}
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
	mac[0] = (mac[0] & 0xFC) | 0x02; // Устанавливаем первый бит в 0 (1байт четный), а второй не 2,6,10.... Устанавливаем второй бит в 1


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
	const int portIndex = getPortIndex(myPortName);

	if (portIndex < 0)
		return;

	auto& port = m_ports[portIndex];

	// Удаляем записи, связанные с отключаемым портом
	if (m_macTable)
		m_macTable->removeByPort(portIndex);

	port.isOccupied = false;
	port.connectedDevice = nullptr;
	port.connectedPortName.clear();
	port.cableType = CableType::TwistedPair;
}

void NetworkDevice::disconnectedFrom(NetworkDevice* other) {
	if (!other)
		return;

	// Освобождаем наши порты, ведущие к соседу
	for (int i = 0; i < m_ports.size(); ++i) {
		if (m_ports[i].connectedDevice == other)
			disconnectPort(m_ports[i].name);
	}

	// Освобождаем порты соседа, ведущие к нам
	for (int i = 0; i < other->m_ports.size(); ++i) {
		if (other->m_ports[i].connectedDevice == this)
			other->disconnectPort(other->m_ports[i].name);
	}

}

bool NetworkDevice::connectPorts(NetworkDevice* first, const QString& firstPortName,
								 NetworkDevice* second, const QString& secondPortName,
								 CableType cableType, QString& error) 
{
	error.clear();

	if (!first || !second) {
		error = QStringLiteral("Не передано устройство.");
		return false;
	}

	if (first == second) {
		error = QStringLiteral("Нельзя соединить устройство само с собой.");
		return false;
	}

	const int firstIndex = first->getPortIndex(firstPortName);
	const int secondIndex = second->getPortIndex(secondPortName);

	if (firstIndex < 0 || secondIndex < 0) {
		error = QStringLiteral("Не найден выбранный порт.");
		return false;
	}

	if (first->m_ports[firstIndex].isOccupied ||
		second->m_ports[secondIndex].isOccupied) {
		error = QStringLiteral("Один из портов уже занят.");
		return false;
	}

	// Параллельные соединения пока не поддерживаем
	for (const auto& port : first->m_ports) {
		if (port.isOccupied && port.connectedDevice == second) {
			error = QStringLiteral("Эти устройства уже соединены.");
			return false;
		}
	}

	// Без STP запрещаем создание петли между коммутаторами
	if (supportsEthernet(cableType) &&
		first->m_type == DeviceType::Switch &&
		second->m_type == DeviceType::Switch) {

		QQueue<NetworkDevice*> queue;
		QSet<NetworkDevice*> visited;

		queue.enqueue(first);
		visited.insert(first);

		while (!queue.isEmpty()) {
			NetworkDevice* current = queue.dequeue();

			if (current == second) {
				error = QStringLiteral(
					"Соединение создаёт L2-петлю. STP пока не реализован.");
				return false;
			}

			for (const auto& port : current->m_ports) {
				NetworkDevice* neighbor = port.connectedDevice;

				if (!port.isOccupied ||
					!supportsEthernet(port.cableType) ||
					!neighbor ||
					neighbor->m_type != DeviceType::Switch ||
					visited.contains(neighbor)) {
					continue;
				}

				visited.insert(neighbor);
				queue.enqueue(neighbor);
			}
		}
	}

	// Все проверки пройдены — занимаем оба порта
	auto& firstPort = first->m_ports[firstIndex];
	auto& secondPort = second->m_ports[secondIndex];

	firstPort.isOccupied = true;
	firstPort.connectedDevice = second;
	firstPort.connectedPortName = secondPortName;
	firstPort.cableType = cableType;

	secondPort.isOccupied = true;
	secondPort.connectedDevice = first;
	secondPort.connectedPortName = firstPortName;
	secondPort.cableType = cableType;

	return true;
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

int NetworkDevice::getPortIndex(const QString& portName) const {
	for (int i = 0; i < m_ports.size(); ++i) {
		if (m_ports[i].name == portName)
			return i;
	}
	return -1;
}


bool NetworkDevice::ping(const QString& targetIp, QStringList& outLog)
{
	QString cleanTarget = targetIp.trimmed();
	outLog.clear();

	//Проверка пинга самого себя (Loopback)
	if (cleanTarget == getIP()) {
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


bool NetworkDevice::configureInterface(	int portIndex,	const QString& ip,	const QString& mask,QString& error)
{
	error.clear();

	if (m_type == DeviceType::Switch) {
		error = QStringLiteral(
			"У L2-коммутатора нет маршрутизируемых интерфейсов.");
		return false;
	}

	if (portIndex < 0 || portIndex >= m_ports.size()) {
		error = QStringLiteral("Неверный индекс интерфейса.");
		return false;
	}

	quint32 address = 0;
	quint32 subnetMask = 0;
	int prefixLength = 0;

	if (!Ipv4Utils::parse(ip, address)) {
		error = QStringLiteral("Некорректный IPv4-адрес.");
		return false;
	}

	if (!Ipv4Utils::parseMask(mask, subnetMask, prefixLength)) {
		error = QStringLiteral(
			"Некорректная маска. Поддерживаются /1 ... /30.");
		return false;
	}

	if (!Ipv4Utils::isUsableHost(address, subnetMask)) {
		error = QStringLiteral(
			"Этот адрес нельзя назначить интерфейсу.");
		return false;
	}

	for (int i = 0; i < m_ports.size(); ++i) {
		if (i == portIndex)
			continue;

		quint32 otherAddress = 0;

		if (Ipv4Utils::parse(m_ports[i].ipv4, otherAddress) &&
			otherAddress == address) {
			error = QStringLiteral(
				"Этот IP уже назначен другому интерфейсу устройства.");
			return false;
		}
	}

	// Не позволяем изменением IP сделать существующий шлюз
	// недопустимым для его интерфейса.
	if (!m_defaultGateway.isEmpty()) {
		quint32 gateway = 0;

		if (!Ipv4Utils::parse(m_defaultGateway, gateway)) {
			error = QStringLiteral("Некорректная конфигурация шлюза.");
			return false;
		}

		if (gateway == address) {
			error = QStringLiteral(
				"IP интерфейса не должен совпадать со шлюзом.");
			return false;
		}

		if (m_gatewayPortIndex == portIndex) {
			if ((address & subnetMask) != (gateway & subnetMask) ||
				!Ipv4Utils::isUsableHost(gateway, subnetMask)) {
				error = QStringLiteral(
					"Сначала удалите или измените шлюз: "
					"он не подходит для новой подсети.");
				return false;
			}
		}
	}

	// Меняем данные только после всех проверок.
	NetworkPort& port = m_ports[portIndex];
	port.ipv4 = Ipv4Utils::toString(address);
	port.subnetMask = Ipv4Utils::toString(subnetMask);

	return true;
}


bool NetworkDevice::configureDefaultGateway( const QString& gateway, int portIndex,	QString& error)
{
	error.clear();

	// Пустая строка означает удалить default gateway.
	if (gateway.trimmed().isEmpty()) {
		m_defaultGateway.clear();
		m_gatewayPortIndex = -1;
		return true;
	}

	if (m_type == DeviceType::Switch) {
		error = QStringLiteral(
			"Шлюз для L2-коммутатора пока не поддерживается.");
		return false;
	}

	if (portIndex < 0 || portIndex >= m_ports.size()) {
		error = QStringLiteral("Выберите интерфейс шлюза.");
		return false;
	}

	const NetworkPort& port = m_ports[portIndex];

	quint32 address = 0;
	quint32 mask = 0;
	quint32 gatewayAddress = 0;
	int prefixLength = 0;

	if (!Ipv4Utils::parse(port.ipv4, address) ||
		!Ipv4Utils::parseMask(port.subnetMask, mask, prefixLength)) {
		error = QStringLiteral(
			"Сначала настройте IP и маску интерфейса.");
		return false;
	}

	if (!Ipv4Utils::parse(gateway, gatewayAddress) ||
		!Ipv4Utils::isUsableHost(gatewayAddress, mask)) {
		error = QStringLiteral("Некорректный адрес шлюза.");
		return false;
	}

	if ((address & mask) != (gatewayAddress & mask)) {
		error = QStringLiteral(
			"Шлюз должен находиться в подсети выбранного интерфейса.");
		return false;
	}

	for (const auto& candidate : m_ports) {
		quint32 ownAddress = 0;

		if (Ipv4Utils::parse(candidate.ipv4, ownAddress) &&
			ownAddress == gatewayAddress) {
			error = QStringLiteral(
				"Шлюз не должен совпадать с собственным IP устройства.");
			return false;
		}
	}

	m_defaultGateway = Ipv4Utils::toString(gatewayAddress);
	m_gatewayPortIndex = portIndex;

	return true;
}


RouteDecision NetworkDevice::selectRoute(const QString& destinationIp) const
{
	RouteDecision result;

	if (!m_isOnline) {
		result.error = QStringLiteral("Устройство выключено.");
		return result;
	}

	if (m_type == DeviceType::Switch) {
		result.error = QStringLiteral(
			"Исходящие IP-пакеты для L2-коммутатора пока не поддерживаются.");
		return result;
	}

	quint32 destination = 0;

	if (!Ipv4Utils::parse(destinationIp, destination)) {
		result.error = QStringLiteral("Некорректный IPv4 назначения.");
		return result;
	}

	const quint32 firstOctet = destination >> 24;

	// На этом этапе поддерживаем обычную unicast-доставку.
	if (firstOctet == 0 ||
		firstOctet == 127 ||
		firstOctet >= 224) {
		result.error = QStringLiteral(
			"Этот диапазон адресов пока не поддерживается.");
		return result;
	}

	// 1. Локальная доставка.
	for (int i = 0; i < m_ports.size(); ++i) {
		const auto& port = m_ports[i];

		if (!port.adminUp)
			continue;

		quint32 ownAddress = 0;

		if (Ipv4Utils::parse(port.ipv4, ownAddress) &&
			ownAddress == destination) {
			result.type = RouteDecisionType::Local;
			result.portIndex = i;
			return result;
		}
	}

	// 2. Ищем наиболее специфичную подключённую сеть.
	int bestPort = -1;
	int bestPrefix = -1;
	quint32 bestMask = 0;

	for (int i = 0; i < m_ports.size(); ++i) {
		const auto& port = m_ports[i];

		if (!port.adminUp)
			continue;

		quint32 address = 0;
		quint32 mask = 0;
		int prefixLength = 0;

		if (!Ipv4Utils::parse(port.ipv4, address) ||
			!Ipv4Utils::parseMask(
				port.subnetMask, mask, prefixLength)) {
			continue;
		}

		if ((address & mask) == (destination & mask) &&
			prefixLength > bestPrefix) {
			bestPort = i;
			bestPrefix = prefixLength;
			bestMask = mask;
		}
	}

	if (bestPort >= 0) {
		if (!Ipv4Utils::isUsableHost(destination, bestMask)) {
			result.error = QStringLiteral(
				"Доставка на адрес сети или broadcast пока не поддерживается.");
			return result;
		}

		result.type = RouteDecisionType::Direct;
		result.portIndex = bestPort;
		result.nextHopIp = Ipv4Utils::toString(destination);
		return result;
	}

	// 3. Подключённого маршрута нет — используем default gateway.
	if (!m_defaultGateway.isEmpty() &&
		m_gatewayPortIndex >= 0 &&
		m_gatewayPortIndex < m_ports.size()) {

		const auto& port = m_ports[m_gatewayPortIndex];

		if (port.adminUp) {
			result.type = RouteDecisionType::ViaGateway;
			result.portIndex = m_gatewayPortIndex;
			result.nextHopIp = m_defaultGateway;
			return result;
		}
	}

	result.error = QStringLiteral("Нет маршрута до адреса назначения.");
	return result;
}

bool NetworkDevice::applyConfiguration(const QString& name,	const QVector<NetworkPort>& interfaces, const QString& gateway,	int gatewayPort, QString& error)
{
	error.clear();
	if (name.trimmed().isEmpty()) {
		error = QStringLiteral("Введите имя устройства.");
		return false;
	}
	if (interfaces.size() != m_ports.size()) {
		error = QStringLiteral("Неверное число интерфейсов.");
		return false;
	}
	// Validate the whole draft before changing any live state.
	NetworkDevice candidate(name.trimmed(), QString(), m_type);
	for (int i = 0; i < interfaces.size(); ++i) {
		const auto& input = interfaces[i];
		if (m_type == DeviceType::Switch)
			continue;
		if (input.ipv4.trimmed().isEmpty() &&
			input.subnetMask.trimmed().isEmpty()) {
			candidate.m_ports[i].adminUp = input.adminUp;
			continue;
		}
		if (!candidate.configureInterface(i, input.ipv4, input.subnetMask, error)) {
			error = interfaces[i].name + ": " + error;
			return false;
		}
		candidate.m_ports[i].adminUp = input.adminUp;
	}
	if (!candidate.configureDefaultGateway(gateway, gatewayPort, error))
		return false;

	m_name = name.trimmed();
	for (int i = 0; i < m_ports.size(); ++i) {
		// Do not replace MACs, connections or port identity from the draft.
		m_ports[i].ipv4 = candidate.m_ports[i].ipv4;
		m_ports[i].subnetMask = candidate.m_ports[i].subnetMask;
		m_ports[i].adminUp = interfaces[i].adminUp;
	}
	m_defaultGateway = candidate.m_defaultGateway;
	m_gatewayPortIndex = candidate.m_gatewayPortIndex;
	if (m_arpTable) m_arpTable->clear();
	return true;
}
