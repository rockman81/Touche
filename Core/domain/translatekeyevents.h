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

#ifndef TRANSLATEKEYEVENTS_H
#define TRANSLATEKEYEVENTS_H

#include <QtCore/QObject>
#include "domain/deviceinfo.h"
#include "domain/inputevent.h"
class TranslateKeyEventsPrivate;
class KeyboardDatabase;
class DeviceInfo;
class BindingsConfig;
class TranslateKeyEvents : public QObject
{
    Q_OBJECT
public:
    explicit TranslateKeyEvents(KeyboardDatabase *keyboardDatabase, BindingsConfig *bindingsConfig, QObject *parent = 0);
    ~TranslateKeyEvents();
signals:
    void keyEvent(const QString &keyName);
    
public slots:
    void inputEvent(InputEventP keyEvent, DeviceInfo *deviceInfo);
    void noMoreEvents(DeviceInfo *deviceInfo);
    void suspend();
    void resume();

private:
    TranslateKeyEventsPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(TranslateKeyEvents)
};

#endif // TRANSLATEKEYEVENTS_H