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

#include <QDebug>
#include "touchesystemtray.h"
#include <QMenu>
#include <QCoreApplication>
#include "keysconfigurationdialog.h"
#include "domain/deviceinfo.h"
#include "touchecore.h"
#include "traymanager.h"
#include <QMessageBox>
#include "bindingsGui/qstringlistedit.h"

class ToucheSystemTrayPrivate {
public:
    ToucheSystemTrayPrivate(QMenu *systemTrayMenu, QMenu *profilesMenu, QAction *separator, TrayManager *trayManager, ToucheCore *toucheCore)
        : systemTrayMenu(systemTrayMenu), profilesMenu(profilesMenu), separator(separator), trayManager(trayManager), aboutToQuit(false), toucheCore(toucheCore) {}
    QMenu *systemTrayMenu;
    QMenu *profilesMenu;
    QAction *separator;
    QMap<DeviceInfo*, QAction*> actions;
    TrayManager *trayManager;
    bool aboutToQuit;
    ToucheCore *toucheCore;
};

ToucheSystemTray::ToucheSystemTray(ToucheCore *toucheCore, QMenu *systemTrayMenu, QMenu *profilesMenu, QAction *separator, TrayManager *trayManager) :
    QObject(toucheCore), d_ptr(new ToucheSystemTrayPrivate(systemTrayMenu, profilesMenu, separator, trayManager, toucheCore))
{
    Q_D(ToucheSystemTray);
    profilesMenu->setTitle(tr("Profiles"));
    connect(toucheCore, SIGNAL(connected(DeviceInfo*)), SLOT(deviceConnected(DeviceInfo*)));
    connect(toucheCore, SIGNAL(disconnected(DeviceInfo*)), SLOT(deviceDisconnected(DeviceInfo*)));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
    connect(qApp, SIGNAL(aboutToQuit()), toucheCore, SLOT(quit()));
    connect(d->toucheCore, SIGNAL(profileChanged(QString)), this, SLOT(updateProfilesList()));
    connect(d->toucheCore, SIGNAL(profileChanged(QString)), this, SLOT(profileChanged(QString)));

    updateTooltip();
    updateProfilesList();
}

ToucheSystemTray::~ToucheSystemTray()
{
    delete d_ptr;
}

void ToucheSystemTray::showConfigurationDialog()
{
    Q_D(ToucheSystemTray);
    if(d->toucheCore->currentProfile().isEmpty() || ! d->toucheCore->availableProfiles().contains(d->toucheCore->currentProfile() )) {
        QMessageBox::warning(0, tr("Profile missing"), "Error! You have to add and select a profile first.");
        return;
    }
    QAction *action = dynamic_cast<QAction*>(sender());
    DeviceInfo *deviceInfo = d->actions.key(action);
    KeysConfigurationDialog configDialog(deviceInfo, d->toucheCore->currentProfile());
    connect(d->toucheCore, SIGNAL(disconnected(DeviceInfo*)), &configDialog, SLOT(reject()));
    connect(d->toucheCore, SIGNAL(inputEvent(QString)), &configDialog, SLOT(keyEvent(QString)));
    d->toucheCore->suspendEventsTranslation();
    configDialog.exec();
    d->toucheCore->resumeEventsTranslation();
}

void ToucheSystemTray::deviceConnected(DeviceInfo *deviceInfo)
{
    Q_D(ToucheSystemTray);
    QString messageTitle = tr("%1: Device Connected!", "device connected tray popup").arg(qAppName());
    d->trayManager->showMessage(messageTitle, deviceInfo->name(), "input-keyboard");
    QAction *deviceAction = d->trayManager->createAction(deviceInfo->name(), d->systemTrayMenu);
    connect(deviceAction, SIGNAL(triggered()), this, SLOT(showConfigurationDialog()));
    d->systemTrayMenu->insertAction(d->separator, deviceAction);
    d->actions.insert(deviceInfo, deviceAction);
    updateTooltip();
}

void ToucheSystemTray::deviceDisconnected(DeviceInfo *deviceInfo)
{
    Q_D(ToucheSystemTray);
    qDebug() << "about to quit: " << d->aboutToQuit;
    if(d->aboutToQuit)
        return;
    QString messageTitle = QString("<b>%1</b>: %2").arg(qAppName()).arg(tr("Device Disconnected!", "device disconnected tray popup"));
    d->trayManager->showMessage(messageTitle, deviceInfo->name(), "input-keyboard");
    QAction *action = d->actions.take(deviceInfo);
    d->systemTrayMenu->removeAction(action);
    delete action;
    updateTooltip();

}

void ToucheSystemTray::updateTooltip()
{
    Q_D(ToucheSystemTray);
    QStringList devices;
    foreach(DeviceInfo* deviceInfo, d->actions.keys()) {
        devices << deviceInfo->name();
    }
    d->trayManager->updateTooltip(devices.join("\n"));
}

void ToucheSystemTray::aboutToQuit()
{
    Q_D(ToucheSystemTray);
    d->aboutToQuit=true;
}




void ToucheSystemTray::updateProfilesList()
{
    Q_D(ToucheSystemTray);
    d->profilesMenu->clear();
    d->profilesMenu->addAction(tr("Edit Profiles..."), this, SLOT(editProfiles()));
    QAction *nextProfile = d->profilesMenu->addAction(tr("Next Profile"),
        this, SLOT(switchToNextProfile()));
    nextProfile->setShortcutContext(Qt::ApplicationShortcut);
    d->profilesMenu->addSeparator();
    foreach(QString profile, d->toucheCore->availableProfiles()) {
        QAction *profileAction = d->profilesMenu->addAction(profile);
        profileAction->setObjectName(profile);
        profileAction->setCheckable(true);
        profileAction->setChecked(profile == d->toucheCore->currentProfile());
        connect(profileAction, SIGNAL(triggered()), this, SLOT(setProfile()));
    }
}


void ToucheSystemTray::setProfile()
{
    Q_D(ToucheSystemTray);
    QAction *profileAction = (QAction*) sender();
    d->toucheCore->setProfile(profileAction->objectName());
}

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>

EditProfilesDialog::EditProfilesDialog(ToucheCore *core)
    : QDialog(){
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    settings = new QSettings("GuLinux", qAppName(), this);
    setWindowTitle(QString("%1 profiles").arg(qAppName()));
    profilesList = new QStringListEdit();
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    vlayout->addWidget(new QLabel("You can add new profiles here.\nEach profile will have its own key bindings."));
    vlayout->addWidget(profilesList);
    vlayout->addWidget(buttonBox);
    profilesList->setStringList(core->availableProfiles());
}

void ToucheSystemTray::editProfiles()
{
    Q_D(ToucheSystemTray);
    EditProfilesDialog editProfilesDialog(d->toucheCore);
    editProfilesDialog.exec();
    updateProfilesList();
}


void EditProfilesDialog::accept()
{
    foreach(QString profile, settings->childGroups()) {
        if(!profile.startsWith("bindings_")) continue;
        if(profilesList->stringList().contains(QString(profile).replace("bindings_", ""))) continue;
        settings->beginGroup(profile);
        settings->remove("");
        settings->endGroup();
    }

    foreach(QString profile, profilesList->stringList()) {
        if(settings->childGroups().contains(QString("bindings_%1").arg(profile))) continue;
        settings->beginGroup(QString("bindings_%1").arg(profile));
        settings->setValue("name", profile);
        settings->endGroup();
    }

    QDialog::accept();
}


void ToucheSystemTray::switchToNextProfile()
{
    Q_D(ToucheSystemTray);
    QStringList profiles = d->toucheCore->availableProfiles();
    QList<QString>::const_iterator i;
    QString newProfile;
    for (i = profiles.constBegin(); i != profiles.constEnd(); ++i) {
        if(*i == d->toucheCore->currentProfile()) {
            i++;
            if(i==profiles.constEnd())
                newProfile = profiles.first();
            else newProfile = *i;
            break;
        }
    }
    d->toucheCore->setProfile(newProfile);
}


void ToucheSystemTray::profileChanged(const QString &profile)
{
    Q_D(ToucheSystemTray);
    d->trayManager->showMessage(tr("%1 Profile").arg(qAppName()), tr("Profile changed to %1").arg(profile), "input-keyboard");
}