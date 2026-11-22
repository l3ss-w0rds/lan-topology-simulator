#include "EthernetSimulator.h"

#include <QQueue>

namespace {

    struct FrameEvent {
        NetworkDevice* device = nullptr;
        int inputPort = -1;
        EthernetFrame frame;
    };

    // Проверяем доступность локального конца соединения
    bool canUsePort(NetworkDevice* device, int portIndex)
    {
        if (!device || !device->getOnline())
            return false;

        const auto& ports = device->getPorts();

        if (portIndex < 0 || portIndex >= ports.size())
            return false;

        const auto& port = ports[portIndex];

        return port.adminUp &&
            port.isOccupied &&
            port.connectedDevice &&
            supportsEthernet(port.cableType);
    }

    // Передаём кадр на конкретный порт соседнего устройства
    bool enqueueNeighbor(
        QQueue<FrameEvent>& queue,
        NetworkDevice* sender,
        int outputPort,
        const EthernetFrame& frame)
    {
        if (!canUsePort(sender, outputPort))
            return false;

        const auto& port = sender->getPorts()[outputPort];
        NetworkDevice* neighbor = port.connectedDevice;

        const int inputPort =
            neighbor->getPortIndex(port.connectedPortName);

        if (!canUsePort(neighbor, inputPort))
            return false;

        const auto& reversePort = neighbor->getPorts()[inputPort];

        // Проверяем, что соединение действительно двустороннее
        if (reversePort.connectedDevice != sender ||
            reversePort.connectedPortName != port.name ||
            reversePort.cableType != port.cableType) {
            return false;
        }

        FrameEvent event;
        event.device = neighbor;
        event.inputPort = inputPort;
        event.frame = frame;

        queue.enqueue(event);
        return true;
    }

    // Проверяем групповой бит адреса назначения
    bool isGroupMac(const QString& mac)
    {
        bool ok = false;
        const uint firstOctet = mac.section(':', 0, 0).toUInt(&ok, 16);

        return ok && (firstOctet & 1u) != 0;
    }

} // namespace

EthernetResult EthernetSimulator::transmit(
    NetworkDevice* sender,
    int outputPort,
    const EthernetFrame& sourceFrame)
{
    EthernetResult result;

    if (!sender) {
        result.error = QStringLiteral("Не задан отправитель кадра.");
        return result;
    }

    EthernetFrame frame = sourceFrame;
    frame.sourceMac = frame.sourceMac.trimmed().toUpper();
    frame.destinationMac =
        frame.destinationMac.trimmed().toUpper();

    if (frame.sourceMac.isEmpty() ||
        frame.destinationMac.isEmpty()) {
        result.error = QStringLiteral("Не задан MAC-адрес кадра.");
        return result;
    }

    QQueue<FrameEvent> queue;

    if (!enqueueNeighbor(queue, sender, outputPort, frame)) {
        result.error = QStringLiteral(
            "Выходной интерфейс или соединение недоступны.");
        return result;
    }

    // Защита движка от слишком большой или некорректной топологии
    constexpr int maxEvents = 10000;
    int processedEvents = 0;

    while (!queue.isEmpty()) {
        if (++processedEvents > maxEvents) {
            result.arrivals.clear();
            result.error = QStringLiteral(
                "Превышен лимит обработки кадров. "
                "Проверьте топологию на L2-петли.");
            return result;
        }

        const FrameEvent event = queue.dequeue();
        NetworkDevice* current = event.device;

        if (!canUsePort(current, event.inputPort))
            continue;

        // Коммутатор обучает MAC и выбирает выходные порты
        if (current->getType() == DeviceType::Switch) {
            MacTable* table = current->getMacTable();

            if (!table)
                continue;

            table->cleanExpired();
            table->learn(
                event.frame.sourceMac,
                event.inputPort);

            const bool groupDestination =
                isGroupMac(event.frame.destinationMac);

            const int output = groupDestination
                ? -1
                : table->lookup(event.frame.destinationMac);

            // Blackhole-запись запрещает передачу
            if (output == -2)
                continue;

            // Известный unicast отправляем в один порт
            if (output >= 0) {
                if (output != event.inputPort) {
                    enqueueNeighbor(
                        queue,
                        current,
                        output,
                        event.frame);
                }

                continue;
            }

            // Broadcast, multicast и неизвестный unicast рассылаем
            const auto& ports = current->getPorts();

            for (int i = 0; i < ports.size(); ++i) {
                if (i == event.inputPort)
                    continue;

                enqueueNeighbor(
                    queue,
                    current,
                    i,
                    event.frame);
            }

            continue;
        }

        // Конечный интерфейс принимает свой unicast или broadcast
        const auto& input =
            current->getPorts()[event.inputPort];

        const bool addressedToUs =
            event.frame.destinationMac ==
            input.mac.trimmed().toUpper();

        const bool broadcast =
            event.frame.destinationMac ==
            ethernetBroadcastMac();

        if (!addressedToUs && !broadcast)
            continue;

        EthernetArrival arrival;
        arrival.device = current;
        arrival.portIndex = event.inputPort;
        arrival.frame = event.frame;

        result.arrivals.append(arrival);
    }

    return result;
}