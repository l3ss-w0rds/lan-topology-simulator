#include "MacTable.h"

MacTable::MacTable(int agingTimeSec):
m_agingTimeSec(agingTimeSec){
}

QString MacTable::makeKey(const QString& mac, int VlanID) const {
    return QString("%1:%2").arg(VlanID).arg(mac.toUpper());
}

void MacTable::learn(const QString& mac, int portIndex, int VlanID) {
    if (mac.isEmpty() || portIndex < 0 || VlanID < 1 || VlanID > 4094) {
        qWarning() << "Неправильные параметры для обучения: mac=" << mac << ", portIndex=" << portIndex << ", VlanID=" << VlanID;
        return;
	}
	QString key = makeKey(mac, VlanID); //VLAN:MAC
    if (m_macTable.contains(key)) {
        if (m_macTable[key].type == TypeEntry::Static || m_macTable[key].type == TypeEntry::Blackhole) return;
    }
//ДОБАВЛЕНИЕ В ТАБЛИЦУ ЗАПИСИ
	MacEntry entry;
    entry.VlanID = VlanID;
    entry.macAddress = mac.trimmed().toUpper();
    entry.type = TypeEntry::Dynamic;
    entry.portIndex = portIndex;
	entry.timestamp = QDateTime::currentDateTime(); //обнуляем таймер старения
	entry.isValid = true;
	m_macTable[key] = entry;
}

void MacTable::addStatic(const QString& mac, int portIndex, int VlanID) {
    if (mac.isEmpty() || portIndex < 0 || VlanID < 1 || VlanID > 4094) {
        qWarning() << "Неправильные параметры для обучения: mac=" << mac << ", portIndex=" << portIndex << ", VlanID=" << VlanID;
        return;
    }
    QString key = makeKey(mac, VlanID);
 //ДОБАВЛЕНИЕ В ТАБЛИЦУ ЗАПИСИ
    MacEntry entry;
    entry.VlanID = VlanID;
    entry.macAddress = mac.trimmed().toUpper();
    entry.type = TypeEntry::Dynamic;
    entry.portIndex = portIndex;
    entry.timestamp = QDateTime::currentDateTime(); //обнуляем таймер старения
    entry.isValid = true;
    m_macTable[key] = entry;
}
void MacTable::addBlackhole(const QString& mac, int portIndex, int VlanID) {
    if (mac.isEmpty() || portIndex < 0 || VlanID < 1 || VlanID > 4094) {
        qWarning() << "Неправильные параметры для обучения: mac=" << mac << ", portIndex=" << portIndex << ", VlanID=" << VlanID;
        return;
    }
    QString key = makeKey(mac, VlanID);
 //ДОБАВЛЕНИЕ В ТАБЛИЦУ ЗАПИСИ
    MacEntry entry;
    entry.VlanID = VlanID;
    entry.macAddress = mac.trimmed().toUpper();
    entry.type = TypeEntry::Blackhole;
    entry.portIndex = portIndex;
    entry.timestamp = QDateTime::currentDateTime(); //обнуляем таймер старения
    entry.isValid = true;
    m_macTable[key] = entry;
}

int MacTable::lookup(const QString& mac, int VlanID)  {
    QString key = makeKey(mac, VlanID);

    if (!m_macTable.contains(key)) {
        return -1; //не найден
	}
	auto& entry = m_macTable[key];

    if (entry.type == TypeEntry::Blackhole) return -2;

    if (entry.type == TypeEntry::Dynamic) {
        qint64 ageSec = entry.timestamp.secsTo(QDateTime::currentDateTime());
        if (ageSec > m_agingTimeSec) {
            m_macTable.remove(key); //удаляем устаревший мак
            return -1; //не найден
		}
		entry.timestamp = QDateTime::currentDateTime(); //обнул. таймер старения
	}
    return entry.portIndex; //не найден
}

void MacTable::cleanExpired() {
    QDateTime now = QDateTime::currentDateTime();
    QList<QString> keysToRemove;
	// Собираем ключи устаревших динамических записей
    for (auto it = m_macTable.begin(); it != m_macTable.end(); ++it) {
        if (it.value().type == TypeEntry::Dynamic) {
            if (it.value().timestamp.secsTo(now) > m_agingTimeSec) {
                keysToRemove.append(it.key());
            }
        }
    }
	// Удаляем устаревшие записи
    for (const auto& k : keysToRemove) {
        m_macTable.remove(k);
    }
}

void MacTable::removeByPort(int portIndex) {
    QList<QString> keysToRemove;
    for (auto it = m_macTable.begin(); it != m_macTable.end(); ++it) {
        if (it.value().portIndex == portIndex) {
            keysToRemove.append(it.key());
        }
    }

    for (const auto& k : keysToRemove) {
        m_macTable.remove(k);
    }
}

void MacTable::clear()
{
    m_macTable.clear();
}