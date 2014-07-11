/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "buildgraphlocker.h"

#include "error.h"

#include <logging/translator.h>

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace qbs {
namespace Internal {

BuildGraphLocker::BuildGraphLocker(const QString &buildGraphFilePath)
#if HAS_QLOCKFILE
    : m_lockFile(buildGraphFilePath + QLatin1String(".lock"))
#endif
{
#if HAS_QLOCKFILE
    const QString buildDir = QFileInfo(buildGraphFilePath).absolutePath();
    if (!QDir::root().mkpath(buildDir))
        throw ErrorInfo(Tr::tr("Cannot lock build graph file '%1': Failed to create directory."));
    m_lockFile.setStaleLockTime(0);
    int attemptsToGetInfo = 0;
    do {
        if (m_lockFile.tryLock(250))
            return;
        switch (m_lockFile.error()) {
        case QLockFile::LockFailedError: {
            qint64 pid;
            QString hostName;
            QString appName;
            if (m_lockFile.getLockInfo(&pid, &hostName, &appName)) {
                throw ErrorInfo(Tr::tr("Cannot lock build graph file '%1': "
                        "Already locked by '%2' (PID %3).")
                                .arg(buildGraphFilePath, appName).arg(pid));
            }
            break;
        }
        case QLockFile::PermissionError:
            throw ErrorInfo(Tr::tr("Cannot lock build graph file '%1': Permission denied.")
                            .arg(buildGraphFilePath));
        case QLockFile::UnknownError:
        case QLockFile::NoError:
            throw ErrorInfo(Tr::tr("Cannot lock build graph file '%1' (reason unknown).")
                            .arg(buildGraphFilePath));
        }
    } while (++attemptsToGetInfo < 10);

    // This very unlikely case arises if tryLock() repeatedly returns LockFailedError
    // with the subsequent getLockInfo() failing as well.
    throw ErrorInfo(Tr::tr("Cannot lock build graph file '%1' (reason unknown).")
                    .arg(buildGraphFilePath));
#else
    Q_UNUSED(buildGraphFilePath);
#endif
}

BuildGraphLocker::~BuildGraphLocker()
{
#if HAS_QLOCKFILE
    m_lockFile.unlock();
#endif
}

} // namespace Internal
} // namespace qbs
