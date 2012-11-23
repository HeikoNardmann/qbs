/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "application.h"

#include "consoleprogressobserver.h"
#include "ctrlchandler.h"
#include <logging/logger.h>
#include <logging/translator.h>

namespace qbs {

Application::Application(int &argc, char **argv)
    : QCoreApplication(argc, argv), m_observer(new ConsoleProgressObserver)
{
    installCtrlCHandler();
}

Application *Application::instance()
{
    return qobject_cast<Application *>(QCoreApplication::instance());
}

void Application::init()
{
    setApplicationName(QLatin1String("qbs"));
    setOrganizationName(QLatin1String("QtProject"));
    setOrganizationDomain(QLatin1String("qt-project.org"));
}

/**
 * Interrupt the application. This is directly called from a signal handler.
 */
void Application::userInterrupt()
{
    if (!m_observer)
        return;

    qbsInfo() << Tr::tr("Received termination request from user; canceling build. [pid=%1]")
                 .arg(applicationPid());
    m_observer->setCanceled(true);
}

} // namespace qbs
