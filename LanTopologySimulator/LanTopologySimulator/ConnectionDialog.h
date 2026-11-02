#pragma once
#include <QDialog>
#include <QComboBox>
#include "CableItem.h"
#include "NetworkDevice.h"

class ConnectionDialog : public QDialog {
	Q_OBJECT
public:
	ConnectionDialog(NetworkDevice* dev1, NetworkDevice* dev2, QWidget* parent = nullptr);
	~ConnectionDialog() override = default;


	CableType getSelectedCableType() const;
	QString getSelectedPort1() const;
	QString getSelectedPort2() const;
private:
	void setupUI(NetworkDevice* dev1, NetworkDevice* dev2);

	QComboBox* m_cableTypeCombo = nullptr;
	QComboBox* m_port1Combo = nullptr;
	QComboBox* m_port2Combo = nullptr;
};