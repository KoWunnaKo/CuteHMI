#ifndef H_EXTENSIONS_CUTEHMI_MODBUS_2_INCLUDE_CUTEHMI_MODBUS_INTERNAL_REGISTERCONTROLLERMIXIN_HPP
#define H_EXTENSIONS_CUTEHMI_MODBUS_2_INCLUDE_CUTEHMI_MODBUS_INTERNAL_REGISTERCONTROLLERMIXIN_HPP

#include "common.hpp"
#include "functions.hpp"
#include "RegisterControllerTraits.hpp"
#include "../AbstractDevice.hpp"

#include <QBasicTimer>
#include <QJsonObject>

namespace cutehmi {
namespace modbus {
namespace internal {

template <typename DERIVED>
class RegisterControllerMixin
{
	protected:
		typedef typename RegisterControllerTraits<DERIVED>::ValueType ValueType;

		RegisterControllerMixin();

		void setValue(ValueType value);

		void writeValue();

		void timerEvent(QTimerEvent * event);

		void onRequestCompleted(QJsonObject request, QJsonObject reply);

		void clearPostponedWrite();

	private:
		const DERIVED & derived() const;

		DERIVED & derived();
};

template <typename DERIVED>
RegisterControllerMixin<DERIVED>::RegisterControllerMixin()
{
}

template <typename DERIVED>
void RegisterControllerMixin<DERIVED>::setValue(ValueType value)
{
	derived().m->requestedValue = value;

	if (derived().device() == nullptr)
		CUTEHMI_WARNING("Attempting to set value, but no device has been assigned to the controller.");
	else {
		if (derived().m->value != value) {
			derived().m->adjustingValue = true;

			if (derived().writeMode() == DERIVED::WRITE_DELAYED)
				derived().m->writeTimer.start(derived().writeDelay(), & derived());
			else if (derived().writeMode() == DERIVED::WRITE_POSTPONED) {
				// If m->requestId is not null, then controller has not finished with previous request yet.
				if (derived().m->requestId != nullptr)
					derived().m->postponedWritePending = true;
				else
					derived().writeValue();
			} else if (derived().writeMode() == DERIVED::WRITE_IMMEDIATE)
				derived().writeValue();
			// If write mode is WRITE_EXPLICIT, then do nothing.
		} else {
			if (derived().writeMode() == DERIVED::WRITE_DELAYED) {
				if (derived().m->requestId == nullptr) {
					derived().m->adjustingValue = false;
					derived().m->writeTimer.stop();
				} else {
					derived().m->adjustingValue = true;
					derived().m->writeTimer.start(derived().writeDelay(), & derived());
				}
			} else if (derived().writeMode() == DERIVED::WRITE_POSTPONED) {
				if (derived().m->requestId != nullptr) {
					derived().m->adjustingValue = true;
					derived().m->postponedWritePending = true;
				}
			} else if ((derived().writeMode() == DERIVED::WRITE_IMMEDIATE) && (derived().m->requestId != nullptr))
				derived().writeValue();
		}
	}
}

template<typename DERIVED>
void RegisterControllerMixin<DERIVED>::writeValue()
{
	derived().requestWrite(derived().m->requestedValue);
	derived().m->adjustingValue = false;
}

template<typename DERIVED>
void RegisterControllerMixin<DERIVED>::timerEvent(QTimerEvent * event)
{
	Q_UNUSED(event)

	derived().m->writeTimer.stop();
	derived().writeValue();
}

template<typename DERIVED>
void RegisterControllerMixin<DERIVED>::onRequestCompleted(QJsonObject request, QJsonObject reply)
{
	AbstractDevice::Function function = static_cast<AbstractDevice::Function>(request.value("function").toInt());
	QUuid requestId = QUuid::fromString(request.value("id").toString());
	bool success = reply.value("success").toBool();
	quint16 address = static_cast<quint16>(request.value("payload").toObject().value("address").toDouble());
	if (function == derived().writeRegisterFunction()) {
		if (requestId == derived().m->requestId) {
			if (success) {
				if (derived().readOnWrite())
					// Non-null requestId implies that device is not null (see setDevice() and setupRegister()).
					derived().requestReadRegisters(static_cast<quint16>(derived().address()), derived().bytes(), & derived().m->requestId);
				else {
					derived().setBusy(derived().m->postponedWritePending);

					emit derived().valueWritten();

					// Without readOnWrite verification, written value acts as one, which is currently set in register.
					/// @todo Consider removing this and updateValue(QJsonValue).
					derived().updateValue(request.value("payload").toObject().value("value"));

					derived().m->requestId = nullptr;
				}
			} else {
				if (!derived().readOnWrite())
					derived().setBusy(derived().m->postponedWritePending);

				emit derived().valueFailed();

				derived().m->requestId = nullptr;
			}
		}
	} else if (function == derived().readRegistersFunction()) {
		quint16 endAddress = address + static_cast<quint16>(request.value("payload").toObject().value("amount").toDouble()) - 1;
		if (static_cast<quint16>(derived().address()) >= address && static_cast<quint16>(derived().address()) <= endAddress) {
			if (requestId == derived().m->requestId) {
				// If requestId == m->requestId, then request must have been made by controller due to readOnWrite.

				derived().setBusy(!success || derived().m->postponedWritePending);

				// Non-null requestId implies that register is not null (see setDevice() and setupRegister()).
				if (success && (derived().verifyRegisterValue()))
					emit derived().valueWritten();
				else
					emit derived().valueMismatch(); // In case of read failure we can't verify value. Even though write request must have succeeded assume valueMismatch() in such case.

				derived().updateValue();

				derived().m->requestId = nullptr;
			} else if (derived().m->requestId.isNull()) {
				// Standard update, if controller is not waiting for its own request made due to readOnWrite.

				derived().setBusy(!success || derived().m->postponedWritePending);

				derived().updateValue();
			}
		}
	}
	clearPostponedWrite();
}

template<typename DERIVED>
void RegisterControllerMixin<DERIVED>::clearPostponedWrite()
{
	if (derived().m->postponedWritePending)
		derived().writeValue();
	derived().m->postponedWritePending = false;
}

template <typename DERIVED>
const DERIVED & RegisterControllerMixin<DERIVED>::derived() const
{
	return static_cast<const DERIVED &>(*this);
}

template <typename DERIVED>
DERIVED & RegisterControllerMixin<DERIVED>::derived()
{
	return static_cast<DERIVED &>(*this);
}

}
}
}

#endif

//(c)C: Copyright © 2019, Michał Policht <michal@policht.pl>. All rights reserved.
//(c)C: This file is a part of CuteHMI.
//(c)C: CuteHMI is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//(c)C: CuteHMI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//(c)C: You should have received a copy of the GNU Lesser General Public License along with CuteHMI.  If not, see <https://www.gnu.org/licenses/>.
