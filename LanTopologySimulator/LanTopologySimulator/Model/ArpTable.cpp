#include "ArpTable.h"

ArpTable::ArpTable(int ttlSeconds)
    : m_ttlSeconds(ttlSeconds)
{
}

void ArpTable::addEntry(const QString& ip, const QString& mac, ArpEntryType type)
{
    if (ip.isEmpty() || mac.isEmpty()) return;

    QString cleanIP = ip.trimmed();
    QString cleanMAC = mac.trimmed().toUpper();

    //Если запись для этого IP уже есть — обновляем MAC и время
    for (auto& entry : m_entries) {
        if (entry.ip == cleanIP) {
            // Статические записи не перезаписываются динамикой
            if (entry.type == ArpEntryType::Static && type == ArpEntryType::Dynamic) {
                return;
            }
            entry.mac = cleanMAC;
            entry.type = type;
            entry.timestamp = QDateTime::currentDateTime();
            return;
        }
    }

    //Иначе добавляем новую запись
    ArpEntry newEntry;
    newEntry.ip = cleanIP;
    newEntry.mac = cleanMAC;
    newEntry.type = type;
    newEntry.timestamp = QDateTime::currentDateTime();

    m_entries.append(newEntry);
}

QString ArpTable::lookupMac(const QString& ip) const
{
    QString cleanIP = ip.trimmed();
    for (const auto& entry : m_entries) {
        if (entry.ip == cleanIP) {
            return entry.mac; // Нашли готовый MAC-адрес!
        }
    }
    return ""; // В кэше нет, нужно слать широковещательный ARP-запрос
}

bool ArpTable::hasEntry(const QString& ip) const
{
    return !lookupMac(ip).isEmpty();
}

void ArpTable::cleanExpired()
{
    QDateTime now = QDateTime::currentDateTime();
    // Удаляем динамические записи, которые живут дольше m_ttlSeconds
    for (int i = m_entries.size() - 1; i >= 0; --i) {
        if (m_entries[i].type == ArpEntryType::Dynamic) {
            if (m_entries[i].timestamp.secsTo(now) > m_ttlSeconds) {
                m_entries.removeAt(i);
            }
        }
    }
}

void ArpTable::clear()
{
    m_entries.clear();
}