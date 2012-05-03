/***********************************************************************
Copyright (c) 2012 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Touché: https://github.com/rockman81/Touche

Touché is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "configevent.h"
#include "inputevent.h"
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QDebug>
#include "backend/config/bindingsconfig.h"
#include "domain/binding.h"

class BindingsConfig;
class ConfigEventPrivate {
public:
    QMap<QString, InputEvent*> cfgInputEvents;
};

ConfigEvent::ConfigEvent(QObject *parent) :
    QObject(parent), d_ptr(new ConfigEventPrivate())
{
}

ConfigEvent::~ConfigEvent()
{
    delete d_ptr;
}

bool ConfigEvent::matches(InputEvent *other, const QStringList &tags, BindingsConfig *bindingsConfig)
{
    Q_D(ConfigEvent);

    foreach(QString tag, tags) {
        InputEvent *cfgInputEvent = d->cfgInputEvents.value(tag);
        if(cfgInputEvent->matches(other)) {
            QString eventName = QString("%1_%2")
                    .arg(property("keyName").toString() )
                    .arg(tag);
            qDebug() << "Got match: " << eventName;
            bindingsConfig->bindingFor(eventName, other)->execute();
            return true;
        }
    }

    return false;
}

void ConfigEvent::addInputEvent(const QString &tag, InputEvent *inputEvent)
{
    Q_D(ConfigEvent);
    d->cfgInputEvents.insert(tag, inputEvent);
}
