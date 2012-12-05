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

#include "artifact.h"
#include "transformer.h"
#include "buildgraph.h"

#include <language/language.h>
#include <logging/logger.h>
#include <tools/fileinfo.h>
#include <tools/persistence.h>

QT_BEGIN_NAMESPACE

static QDataStream &operator >>(QDataStream &s, qbs::Internal::Artifact::ArtifactType &t)
{
    int i;
    s >> i;
    t = static_cast<qbs::Internal::Artifact::ArtifactType>(i);
    return s;
}

static QDataStream &operator <<(QDataStream &s, const qbs::Internal::Artifact::ArtifactType &t)
{
    return s << (int)t;
}

QT_END_NAMESPACE

namespace qbs {
namespace Internal {

Artifact::Artifact(BuildProject *p)
    : project(p)
    , product(0)
    , transformer(0)
    , artifactType(Unknown)
    , buildState(Untouched)
    , inputsScanned(false)
    , timestampRetrieved(false)
    , alwaysUpdated(true)
{
}

Artifact::~Artifact()
{
}

void Artifact::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
    FileInfo::splitIntoDirectoryAndFileName(m_filePath, &m_dirPath, &m_fileName);
}

void Artifact::load(PersistentPool &pool)
{
    setFilePath(pool.idLoadString());
    fileTags = pool.idLoadStringSet();
    properties = pool.idLoadS<PropertyMap>();
    transformer = pool.idLoadS<Transformer>();
    unsigned char c;
    pool.stream()
            >> artifactType
            >> timestamp
            >> autoMocTimestamp
            >> c;
    alwaysUpdated = c;
}

void Artifact::store(PersistentPool &pool) const
{
    pool.storeString(m_filePath);
    pool.storeStringSet(fileTags);
    pool.store(properties);
    pool.store(transformer);
    pool.stream()
            << artifactType
            << timestamp
            << autoMocTimestamp
            << static_cast<unsigned char>(alwaysUpdated);
}

void Artifact::disconnectChildren()
{
    if (qbsLogLevel(LoggerTrace))
        qbsTrace("[BG] disconnectChildren: '%s'", qPrintable(relativeArtifactFileName(this)));
    foreach (Artifact * const child, children)
        child->parents.remove(this);
    children.clear();
}

void Artifact::disconnectParents()
{
    if (qbsLogLevel(LoggerTrace))
        qbsTrace("[BG] disconnectParents: '%s'", qPrintable(relativeArtifactFileName(this)));
    foreach (Artifact * const parent, parents)
        parent->children.remove(this);
    parents.clear();
}

void Artifact::disconnectAll()
{
    disconnectChildren();
    disconnectParents();
}

} // namespace Internal
} // namespace qbs
