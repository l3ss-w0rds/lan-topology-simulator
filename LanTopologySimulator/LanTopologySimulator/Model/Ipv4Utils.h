#pragma once
#include <QString>
#include <QStringList>
#include <QtGlobal>

namespace Ipv4Utils {

// Принимает ринимает строку и пытается превратить её в quint32.
    inline bool parse(const QString& text, quint32& result)
    {
        const QString value = text.trimmed();
        const QStringList parts =
            value.split('.', QString::KeepEmptyParts);

        if (parts.size() != 4)
            return false;

        quint32 address = 0;

        for (const QString& part : parts) {
            if (part.isEmpty() || part.size() > 3)
                return false;

            for (const QChar ch : part) {
                if (ch < QLatin1Char('0') || ch > QLatin1Char('9'))
                    return false;
            }
            //Превращаем строку в число
            bool ok = false;
            const uint octet = part.toUInt(&ok, 10);

            if (!ok || octet > 255)
                return false;

            address = (address << 8) | octet; //сдвиг влево на 8 бит, вставляет новый октет в младшие 8 бит
        }

        result = address; //итог: 0xC0A8010A
        return true;
    }
    //из 32-битного числа достаёт 4 байта
    inline QString toString(quint32 address)
    {
        return QString("%1.%2.%3.%4")
            .arg((address >> 24) & 0xFF)
            .arg((address >> 16) & 0xFF)
            .arg((address >> 8) & 0xFF)
            .arg(address & 0xFF);
    }

    //принимает маску в виде десятичных октетов. Для этой версии поддерживаем обычные Ethernet-подсети /1 ... /30.
    inline bool parseMask(const QString& text, quint32& mask, int& prefixLength)
    {
        if (!parse(text, mask))
            return false;

        prefixLength = 0;
        bool zeroSeen = false;

        for (int bit = 31; bit >= 0; --bit) {
            const bool isOne = ((mask >> bit) & 1u) != 0;

            if (isOne) {
                if (zeroSeen)
                    return false;

                ++prefixLength;
            }
            else {
                zeroSeen = true;
            }
        }

        return prefixLength >= 1 && prefixLength <= 30;
    }

    inline bool isUsableHost(quint32 address, quint32 mask)
    {
        // Для учебной версии исключаем специальные диапазоны.
        const quint32 firstOctet = address >> 24;

        if (firstOctet == 0 ||
            firstOctet == 127 || //loopback
            firstOctet >= 224)   //multicast и зарезервированные диапазоны
            return false;

        const quint32 network = address & mask;
        const quint32 broadcast = network | ~mask;

        return address != network && address != broadcast;
    }

} 
/*
address = 192.168.1.10 = 0xC0A8010A
mask    = 255.255.255.0 = 0xFFFFFF00

address & mask = 0xC0A80100 = 192.168.1.0
Адрес сети=192.168.1.0

mask  = 0xFFFFFF00
~mask = 0x000000FF

network   = 0xC0A80100
~mask     = 0x000000FF
broadcast = 0xC0A801FF = 192.168.1.255
*/