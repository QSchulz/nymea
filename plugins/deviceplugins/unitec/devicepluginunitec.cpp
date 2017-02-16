/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page unitec.html
    \title Unitec
    \brief Plugin for Unitech RF 433 MHz devices.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.unitec-elektro.de}{Unitec}
    devices.

    The unitec socket units have a learn function. If you plug in the switch, a red light will start to blink. This means
    the socket is in the learning mode. Now you can add a Unitec switch (48111) to guh with your desired Channel (A,B,C or D).
    In order to pair the socket you just have to press the power ON, and the switch has to be in the pairing mode.
    If the pairing was successfull, the switch will turn on. If the switches will be removed from the socket or there will
    be a power breakdown, the switch has to be re-paired. The device can not remember the teached channel.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/unitec/devicepluginunitec.json
*/

#include "devicepluginunitec.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>

DevicePluginUnitec::DevicePluginUnitec()
{
}

DeviceManager::HardwareResources DevicePluginUnitec::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

DeviceManager::DeviceSetupStatus DevicePluginUnitec::setupDevice(Device *device)
{
    if (device->deviceClassId() != switchDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    foreach (Device* d, myDevices()) {
        if (d->paramValue(channelParamTypeId).toString() == device->paramValue(channelParamTypeId).toString()) {
            qCWarning(dcUnitec) << "Unitec switch with channel " << device->paramValue(channelParamTypeId).toString() << "already added.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginUnitec::executeAction(Device *device, const Action &action)
{   
    QList<int> rawData;
    QByteArray binCode;

    if (action.actionTypeId() != powerActionTypeId) {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Bin codes for buttons
    if (device->paramValue(channelParamTypeId).toString() == "A" && action.param(powerParamTypeId).value().toBool() == true) {
        binCode.append("111011000100111010111111");
    } else if (device->paramValue(channelParamTypeId).toString() == "A" && action.param(powerParamTypeId).value().toBool() == false) {
        binCode.append("111001100110100001011111");
    } else if (device->paramValue(channelParamTypeId).toString() == "B" && action.param(powerParamTypeId).value().toBool() == true) {
        binCode.append("111011000100111010111011");
    } else if (device->paramValue(channelParamTypeId).toString() == "B" && action.param(powerParamTypeId).value().toBool() == false) {
        binCode.append("111000111001100111101011");
    } else if (device->paramValue(channelParamTypeId).toString() == "C" && action.param(powerParamTypeId).value().toBool() == true) {
        binCode.append("111000000011011111000011");
    } else if (device->paramValue(channelParamTypeId).toString() == "C" && action.param(powerParamTypeId).value().toBool() == false) {
        binCode.append("111001100110100001010011");
    } else if (device->paramValue(channelParamTypeId).toString() == "D" && action.param(powerParamTypeId).value().toBool() == true) {
        binCode.append("111001100110100001011101");
    } else if (device->paramValue(channelParamTypeId).toString() == "D" && action.param(powerParamTypeId).value().toBool() == false) {
        binCode.append("111000000011011111001101");
    }

    // =======================================
    //create rawData timings list
    int delay = 500;

    // add sync code
    rawData.append(6);
    rawData.append(14);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(2);
            rawData.append(1);
        }else{
            rawData.append(1);
            rawData.append(2);
        }
    }

    // =======================================
    // send data to hardware resource
    if(transmitData(delay, rawData)){
        qCDebug(dcUnitec) << "transmitted" << pluginName() << device->name() << "power: " << action.param(powerParamTypeId).value().toBool();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qCWarning(dcUnitec) << "could not transmitt" << pluginName() << device->name() << "power: " << action.param(powerParamTypeId).value().toBool();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}
