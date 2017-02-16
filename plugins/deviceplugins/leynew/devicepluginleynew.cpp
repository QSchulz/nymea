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
    \page leynew.html
    \title Leynew
    \brief Plugin for the Leynew RF 433 MHz led controller.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.leynew.com/en/}{Leynew}
    devices.

    Currently only following device is supported:

    \l{http://www.leynew.com/en/productview.asp?id=589}{http://www.leynew.com/en/productview.asp?id=589}

    \l{http://image.en.09635.com/2012-8/15/RF-Controller-Aluminum-Version-LN-CON-RF20B-H-3CH-LV-316.jpg}{http://image.en.09635.com/2012-8/15/RF-Controller-Aluminum-Version-LN-CON-RF20B-H-3CH-LV-316.jpg}

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/leynew/devicepluginleynew.json
*/

#include "devicepluginleynew.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>

DevicePluginLeynew::DevicePluginLeynew()
{
}

DeviceManager::DeviceSetupStatus DevicePluginLeynew::setupDevice(Device *device)
{
    Q_UNUSED(device);

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginLeynew::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

DeviceManager::DeviceError DevicePluginLeynew::executeAction(Device *device, const Action &action)
{   

    if (device->deviceClassId() != rfControllerDeviceClassId) {
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }

    QList<int> rawData;
    QByteArray binCode;


    // TODO: find out how the id will be calculated to bin code or make it discoverable
    // =======================================
    // bincode depending on the id
    if (device->paramValue(idParamTypeId) == "0115"){
        binCode.append("001101000001");
    } else if (device->paramValue(idParamTypeId) == "0014") {
        binCode.append("110000010101");
    } else if (device->paramValue(idParamTypeId) == "0008") {
        binCode.append("111101010101");
    } else {
        qCWarning(dcLeynew) << "Could not get id of device: invalid parameter" << device->paramValue(idParamTypeId);
        return DeviceManager::DeviceErrorInvalidParameter;
    }

    int repetitions = 12;
    // =======================================
    // bincode depending on the action
    if (action.actionTypeId() == brightnessUpActionTypeId) {
        binCode.append("000000000011");
        repetitions = 8;
    } else if (action.actionTypeId() == brightnessDownActionTypeId) {
        binCode.append("000000001100");
        repetitions = 8;
    } else if (action.actionTypeId() == powerActionTypeId) {
        binCode.append("000011000000");
    } else if (action.actionTypeId() == redActionTypeId) {
        binCode.append("000000001111");
    } else if (action.actionTypeId() == greenActionTypeId) {
        binCode.append("000000110011");
    } else if (action.actionTypeId() == blueActionTypeId) {
        binCode.append("000011000011");
    } else if (action.actionTypeId() == whiteActionTypeId) {
        binCode.append("000000111100");
    } else if (action.actionTypeId() == orangeActionTypeId) {
        binCode.append("000011001100");
    } else if (action.actionTypeId() == yellowActionTypeId) {
        binCode.append("000011110000");
    } else if (action.actionTypeId() == cyanActionTypeId) {
        binCode.append("001100000011");
    } else if (action.actionTypeId() == purpleActionTypeId) {
        binCode.append("110000000011");
    } else if (action.actionTypeId() == playPauseActionTypeId) {
        binCode.append("000000110000");
    } else if (action.actionTypeId() == speedUpActionTypeId) {
        binCode.append("001100110000");
        repetitions = 8;
    } else if (action.actionTypeId() == speedDownActionTypeId) {
        binCode.append("110000000000");
        repetitions = 8;
    } else if (action.actionTypeId() == autoActionTypeId) {
        binCode.append("001100001100");
    } else if (action.actionTypeId() == flashActionTypeId) {
        binCode.append("110011000000");
    } else if (action.actionTypeId() == jump3ActionTypeId) {
        binCode.append("111100001100");
    } else if (action.actionTypeId() == jump7ActionTypeId) {
        binCode.append("001111000000");
    } else if (action.actionTypeId() == fade3ActionTypeId) {
        binCode.append("110000110000");
    } else if (action.actionTypeId() == fade7ActionTypeId) {
        binCode.append("001100000000");
    } else {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // =======================================
    //create rawData timings list
    int delay = 50;

    // sync signal (starting with ON)
    rawData.append(3);
    rawData.append(90);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            //   _
            //  | |_ _
            rawData.append(3);
            rawData.append(9);
        }else{
            //   _ _
            //  |   |_
            rawData.append(9);
            rawData.append(3);
        }
    }

    // =======================================
    // send data to hardware resource
    if(transmitData(delay, rawData, repetitions)){
        qCDebug(dcLeynew) << "Transmitted" << pluginName() << device->name() << action.id();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qCWarning(dcLeynew) << "Could not transmitt" << pluginName() << device->name() << action.id();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}
