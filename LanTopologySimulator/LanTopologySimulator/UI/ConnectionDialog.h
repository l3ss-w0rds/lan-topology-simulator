#pragma once
#include <QDialog>
#include <QComboBox>
#include "CableItem.h"
#include "NetworkDevice.h"

class ConnectionDialog : public QDialog {
public:
	ConnectionDialog(NetworkDevice* dev1, NetworkDevice* dev2, QWidget* parent = nullptr);
	~ConnectionDialog() override = default;


	CableType getSelectedCableType() const { return static_cast<CableType>(m_cableTypeCombo->currentData().toInt());	};
	QString getSelectedPort1() const { return m_port1Combo->currentText(); };
	QString getSelectedPort2() const { return m_port2Combo->currentText(); };
private:
	void setupUI(NetworkDevice* dev1, NetworkDevice* dev2);

	QComboBox* m_cableTypeCombo = nullptr;
	QComboBox* m_port1Combo = nullptr;
	QComboBox* m_port2Combo = nullptr;
};