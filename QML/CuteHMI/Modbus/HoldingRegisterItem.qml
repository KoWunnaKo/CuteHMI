import QtQuick 2.0
import QtQuick.Controls 1.3

import CuteHMI.Modbus 1.0

Item
{
	id: root

	anchors.verticalCenter: parent.verticalCenter
	anchors.horizontalCenter: parent.horizontalCenter

	property alias device: holdingRegisterController.device
	property alias address: holdingRegisterController.address
	property alias encoding: holdingRegisterController.encoding
	property alias valueScale: holdingRegisterController.valueScale
	property alias busy: holdingRegisterController.busy
	property alias busyIndicator: busyIndicator
	property alias controller: holdingRegisterController

	ExtBusyIndicator
	{
		id: busyIndicator

		running: root.busy
		centerIn: parent
	}

	HoldingRegisterController
	{
		id: holdingRegisterController

		delegate: root.parent
		device: root.device
		address: root.address
		encoding: root.encoding
		valueScale: root.valueScale
	}
}

//(c)MP: Copyright © 2016, Michal Policht. All rights reserved.
//(c)MP: This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.