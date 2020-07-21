/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::StateEvaluator
    \brief This class helps to evaluate a \l{State} and .

    \ingroup rules
    \inmodule core

    The \l StateEvaluator class helps to evaluate a \l StateDescriptor and check if all \l {State}{States}
    from the given \l StateDescriptor are valid. A \l StateDescriptor is valid if conditions of the
    \l StateDescriptor are true.

    \sa StateDescriptor, State, RuleEngine
*/


#include "stateevaluator.h"
#include "nymeacore.h"
#include "integrations/thingmanager.h"
#include "loggingcategories.h"
#include "nymeasettings.h"

namespace nymeaserver {

/*! Constructs a new StateEvaluator for the given \a stateDescriptor. */
StateEvaluator::StateEvaluator(const StateDescriptor &stateDescriptor):
    m_stateDescriptor(stateDescriptor),
    m_operatorType(Types::StateOperatorAnd)
{
}

/*! Constructs a new StateEvaluator for the given \a childEvaluators and \a stateOperator. */
StateEvaluator::StateEvaluator(QList<StateEvaluator> childEvaluators, Types::StateOperator stateOperator):
    m_stateDescriptor(),
    m_childEvaluators(childEvaluators),
    m_operatorType(stateOperator)
{
}

/*! Returns the \l StateDescriptor of this \l StateEvaluator. */
StateDescriptor StateEvaluator::stateDescriptor() const
{
    return m_stateDescriptor;
}

void StateEvaluator::setStateDescriptor(const StateDescriptor &stateDescriptor)
{
    m_stateDescriptor = stateDescriptor;
}

/*! Returns the list of child \l {StateEvaluator}{StateEvaluators} of this \l StateEvaluator. */
StateEvaluators StateEvaluator::childEvaluators() const
{
    return m_childEvaluators;
}

/*! Sets the list of child evaluators of this \l StateEvaluator to the given \a stateEvaluators.*/
void StateEvaluator::setChildEvaluators(const StateEvaluators &stateEvaluators)
{
    m_childEvaluators = stateEvaluators;
}

/*! Appends the given \a stateEvaluator to the child evaluators of this \l StateEvaluator.
    \sa childEvaluators()
*/
void StateEvaluator::appendEvaluator(const StateEvaluator &stateEvaluator)
{
    m_childEvaluators.append(stateEvaluator);
}

/*! Returns the \l {Types::StateOperator}{StateOperator} for this \l StateEvaluator.*/
Types::StateOperator StateEvaluator::operatorType() const
{
    return m_operatorType;
}

/*! Sets the \l {Types::StateOperator}{StateOperator} for this \l StateEvaluator to the given.
 * \a operatorType. This operator will be used to evaluate the child evaluator list.*/
void StateEvaluator::setOperatorType(Types::StateOperator operatorType)
{
    m_operatorType = operatorType;
}

/*! Returns true, if all child evaluator conditions are true depending on the \l {Types::StateOperator}{StateOperator}.*/
bool StateEvaluator::evaluate() const
{
    qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Evaluating: Operator type" << m_operatorType << "Valid descriptor:" << m_stateDescriptor.isValid() << "Childs:" << m_childEvaluators.count();
    bool descriptorMatching = true;
    if (m_stateDescriptor.isValid()) {
        descriptorMatching = false;
        if (m_stateDescriptor.type() == StateDescriptor::TypeDevice) {
            Thing *device = NymeaCore::instance()->thingManager()->findConfiguredThing(m_stateDescriptor.thingId());
            if (device) {
                ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(device->thingClassId());
                if (device->hasState(m_stateDescriptor.stateTypeId())) {
                    if (m_stateDescriptor == device->state(m_stateDescriptor.stateTypeId())) {
                        qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "State" << device->name() << thingClass.stateTypes().findById(m_stateDescriptor.stateTypeId()).name() << (descriptorMatching ? "is" : "not") << "matching:" << m_stateDescriptor.stateValue() << m_stateDescriptor.operatorType() << device->stateValue(m_stateDescriptor.stateTypeId());
                        descriptorMatching = true;
                    }
                } else {
                    qCWarning(dcRuleEngine) << "StateEvaluator:" << this << "Device found, but it does not appear to have such a state!";
                }
            } else {
                qCWarning(dcRuleEngine) << "StateEvaluator:" << this << "Device not existing!";
            }
        } else { // interface
            foreach (Thing* device, NymeaCore::instance()->thingManager()->configuredThings()) {
                ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(device->thingClassId());
                if (!thingClass.isValid()) {
                    qCWarning(dcRuleEngine()) << "Could not find DeviceClass for Device" << device->name() << device->id();
                    continue;
                }
                if (thingClass.interfaces().contains(m_stateDescriptor.interface())) {
                    StateType stateType = thingClass.stateTypes().findByName(m_stateDescriptor.interfaceState());
                    State state = device->state(stateType.id());
                    // As the StateDescriptor can't compare on it's own against interfaces, generate custom one, matching the device
                    StateDescriptor temporaryDescriptor(stateType.id(), device->id(), m_stateDescriptor.stateValue(), m_stateDescriptor.operatorType());
                    if (temporaryDescriptor == state) {
                        descriptorMatching = true;
                        break;
                    }
                }
            }
        }
    }

    if (m_operatorType == Types::StateOperatorOr) {
        if (m_stateDescriptor.isValid() && descriptorMatching) {
            qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Descriptor is matching. Operator is OR => Evaluation result: true";
            return true;
        }
        foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
            if (stateEvaluator.evaluate()) {
                qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Child evaluator evaluated to true. Operator is OR => Evaluation result: true";
                return true;
            }
        }
        qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "No child evaluator evaluated to true => Evaluation result: false";
        return false;
    }

    if (!descriptorMatching) {
        qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "StateDescriptor not matching and operator is AND => Evaluation result: false";
        return false;
    }

    foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
        if (!stateEvaluator.evaluate()) {
            qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Child evaluator not matching => Evaluation result: false";
            return false;
        }
    }
    qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "StateDescriptor and all child evaluators matching => Evaluation result: true";
    return true;
}

/*! Returns true if this \l StateEvaluator contains a \l{Thing} in it with the given \a thingId. */
bool StateEvaluator::containsThing(const ThingId &thingId) const
{
    if (m_stateDescriptor.thingId() == thingId)
        return true;

    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        if (childEvaluator.containsThing(thingId)) {
            return true;
        }
    }
    return false;
}

/*! Removes the \l Thing with the given \a thingId from this \l StateEvaluator and all its child evaluators. */
void StateEvaluator::removeThing(const ThingId &thingId)
{
    if (m_stateDescriptor.thingId() == thingId)
        m_stateDescriptor = StateDescriptor();

    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators[i].removeThing(thingId);
    }
}

/*! Returns a list of \l{ThingIds} of this StateEvaluator. */
QList<ThingId> StateEvaluator::containedThings() const
{
    QList<ThingId> ret;
    if (!m_stateDescriptor.thingId().isNull()) {
        ret.append(m_stateDescriptor.thingId());
    }
    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        ret.append(childEvaluator.containedThings());
    }
    return ret;
}

/*! This method will be used to save this \l StateEvaluator to the given \a settings.
    The \a groupName will normally be the corresponding \l Rule. */
void StateEvaluator::dumpToSettings(NymeaSettings &settings, const QString &groupName) const
{
    settings.beginGroup(groupName);

    settings.beginGroup("stateDescriptor");
    settings.setValue("stateTypeId", m_stateDescriptor.stateTypeId().toString());
    settings.setValue("thingId", m_stateDescriptor.thingId().toString());
    settings.setValue("interface", m_stateDescriptor.interface());
    settings.setValue("interfaceState", m_stateDescriptor.interfaceState());
    settings.setValue("value", m_stateDescriptor.stateValue());
    settings.setValue("valueType", (int)m_stateDescriptor.stateValue().type());
    settings.setValue("operator", m_stateDescriptor.operatorType());
    settings.endGroup();

    settings.setValue("operator", m_operatorType);

    settings.beginGroup("childEvaluators");
    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators.at(i).dumpToSettings(settings, "stateEvaluator-" + QString::number(i));
    }
    settings.endGroup();

    settings.endGroup();
}

/*! This method will be used to load a \l StateEvaluator from the given \a settings.
    The \a groupName will be the corresponding \l RuleId. Returns the loaded \l StateEvaluator. */
StateEvaluator StateEvaluator::loadFromSettings(NymeaSettings &settings, const QString &groupName)
{
    settings.beginGroup(groupName);
    settings.beginGroup("stateDescriptor");
    StateTypeId stateTypeId(settings.value("stateTypeId").toString());
    ThingId thingId(settings.value("thingId").toString());
    if (thingId.isNull()) { // Retry with deviceId for backwards compatibility (<0.19)
        thingId = ThingId(settings.value("deviceId").toString());
    }
    QVariant stateValue = settings.value("value");
    if (settings.contains("valueType")) {
        QVariant::Type valueType = (QVariant::Type)settings.value("valueType").toInt();
        // Note: only warn, and continue with the QVariant guessed type
        if (valueType == QVariant::Invalid) {
            qCWarning(dcRuleEngine()) << "Could not load the value type of the state evaluator. The value type will be guessed by QVariant" << stateValue;
        } else if (!stateValue.canConvert(valueType)) {
            qCWarning(dcRuleEngine()) << "Could not convert the state evaluator value" << stateValue << "to the stored type" << valueType << ". The value type will be guessed by QVariant.";
        } else {
            stateValue.convert(valueType);
        }
    }

    QString interface = settings.value("interface").toString();
    QString interfaceState = settings.value("interfaceState").toString();
    Types::ValueOperator valueOperator = (Types::ValueOperator)settings.value("operator").toInt();
    StateDescriptor stateDescriptor;
    if (!thingId.isNull() && !stateTypeId.isNull()) {
        stateDescriptor = StateDescriptor(stateTypeId, thingId, stateValue, valueOperator);
    } else {
        stateDescriptor = StateDescriptor(interface, interfaceState, stateValue, valueOperator);
    }

    settings.endGroup();

    StateEvaluator ret(stateDescriptor);
    ret.setOperatorType((Types::StateOperator)settings.value("operator").toInt());

    settings.beginGroup("childEvaluators");
    foreach (const QString &evaluatorGroup, settings.childGroups()) {
        ret.appendEvaluator(StateEvaluator::loadFromSettings(settings, evaluatorGroup));
    }
    settings.endGroup();
    settings.endGroup();
    return ret;
}

/*! Returns true, if all child evaluators are valid, the devices exist and all descriptors are in allowed paramerters.*/
bool StateEvaluator::isValid() const
{
    if (m_stateDescriptor.isValid()) {
        if (m_stateDescriptor.type() == StateDescriptor::TypeDevice) {
            Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_stateDescriptor.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine) << "State evaluator device does not exist!";
                return false;
            }

            if (!thing->hasState(m_stateDescriptor.stateTypeId())) {
                qCWarning(dcRuleEngine) << "State evaluator device found, but it does not appear to have such a state!";
                return false;
            }

            ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(thing->thingClassId());
            foreach (const StateType &stateType, thingClass.stateTypes()) {
                if (stateType.id() == m_stateDescriptor.stateTypeId()) {

                    QVariant stateValue = m_stateDescriptor.stateValue();
                    if (!stateValue.convert(stateType.type())) {
                        qCWarning(dcRuleEngine) << "Could not convert value of state descriptor" << m_stateDescriptor.stateTypeId() << " to:" << QVariant::typeToName(stateType.type()) << " Got:" << m_stateDescriptor.stateValue();
                        return false;
                    }

                    if (stateType.maxValue().isValid() && stateValue > stateType.maxValue()) {
                        qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Max:" << stateType.maxValue();
                        return false;
                    }

                    if (stateType.minValue().isValid() && stateValue < stateType.minValue()) {
                        qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Min:" << stateType.minValue();
                        return false;
                    }

                    if (!stateType.possibleValues().isEmpty() && !stateType.possibleValues().contains(stateValue)) {
                        QStringList possibleValues;
                        foreach (const QVariant &value, stateType.possibleValues()) {
                            possibleValues.append(value.toString());
                        }

                        qCWarning(dcRuleEngine) << "Value not in possible values for state type" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Possible values:" << possibleValues.join(", ");
                        return false;
                    }
                }
            }
        } else { // TypeInterface
            Interface iface = NymeaCore::instance()->thingManager()->supportedInterfaces().findByName(m_stateDescriptor.interface());
            if (!iface.isValid()) {
                qWarning(dcRuleEngine()) << "No such interface:" << m_stateDescriptor.interface();
                return false;
            }
            if (iface.stateTypes().findByName(m_stateDescriptor.interfaceState()).name().isEmpty()) {
                qWarning(dcRuleEngine()) << "Interface" << iface.name() << "has no such state:" << m_stateDescriptor.interfaceState();
                return false;
            }
        }
    }

    if (m_operatorType == Types::StateOperatorOr) {
        foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
            if (stateEvaluator.isValid()) {
                return true;
            }
        }
        return false;
    }

    foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
        if (!stateEvaluator.isValid()) {
            return false;
        }
    }
    return true;
}

/*! Returns true if the StateEvaluator is empty, that is, has no StateDescriptor and no ChildEvaluators */
bool StateEvaluator::isEmpty() const
{
    return !m_stateDescriptor.isValid() && m_childEvaluators.isEmpty();
}

/*! Print a StateEvaluator including childEvaluators recuresively to QDebug. */
QDebug operator<<(QDebug dbg, const StateEvaluator &stateEvaluator)
{
    dbg.nospace() << "StateEvaluator: Operator:" << stateEvaluator.operatorType() << endl << "  " << stateEvaluator.stateDescriptor() << endl;
    for (int i = 0; i < stateEvaluator.childEvaluators().count(); i++) {
        dbg.nospace() << "    " << i << ": " << stateEvaluator.childEvaluators().at(i);
    }
    return dbg;
}

StateEvaluators::StateEvaluators()
{

}

StateEvaluators::StateEvaluators(const QList<StateEvaluator> &other): QList<StateEvaluator>(other)
{

}

QVariant StateEvaluators::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void StateEvaluators::put(const QVariant &variant)
{
    append(variant.value<StateEvaluator>());
}

}
