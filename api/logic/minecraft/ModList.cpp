/* Copyright 2013-2015 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ModList.h"
#include <FileSystem.h>
#include <QMimeData>
#include <QUrl>
#include <QUuid>
#include <QString>
#include <QFileSystemWatcher>
#include <QDebug>

ModList::ModList(const QString &dir) : QAbstractListModel(), m_dir(dir)
{
	FS::ensureFolderPathExists(m_dir.absolutePath());
	m_dir.setFilter(QDir::Readable | QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs |
					QDir::NoSymLinks);
	m_dir.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
	m_watcher = new QFileSystemWatcher(this);
	is_watching = false;
	connect(m_watcher, SIGNAL(directoryChanged(QString)), this,
			SLOT(directoryChanged(QString)));
}

void ModList::startWatching()
{
	update();
	is_watching = m_watcher->addPath(m_dir.absolutePath());
	if (is_watching)
	{
		qDebug() << "Started watching " << m_dir.absolutePath();
	}
	else
	{
		qDebug() << "Failed to start watching " << m_dir.absolutePath();
	}
}

void ModList::stopWatching()
{
	is_watching = !m_watcher->removePath(m_dir.absolutePath());
	if (!is_watching)
	{
		qDebug() << "Stopped watching " << m_dir.absolutePath();
	}
	else
	{
		qDebug() << "Failed to stop watching " << m_dir.absolutePath();
	}
}

void ModList::internalSort(QList<Mod> &what)
{
	auto predicate = [](const Mod &left, const Mod &right)
	{
		if (left.name() == right.name())
		{
			return left.mmc_id().localeAwareCompare(right.mmc_id()) < 0;
		}
		return left.name().localeAwareCompare(right.name()) < 0;
	};
	std::sort(what.begin(), what.end(), predicate);
}

bool ModList::update()
{
	if (!isValid())
		return false;

	QList<Mod> orderedMods;
	QList<Mod> newMods;
	m_dir.refresh();
	auto folderContents = m_dir.entryInfoList();
	bool orderOrStateChanged = false;

	// if there are any untracked files...
	if (folderContents.size())
	{
		// the order surely changed!
		for (auto entry : folderContents)
		{
			newMods.append(Mod(entry));
		}
		internalSort(newMods);
		orderedMods.append(newMods);
		orderOrStateChanged = true;
	}
	// otherwise, if we were already tracking some mods
	else if (mods.size())
	{
		// if the number doesn't match, order changed.
		if (mods.size() != orderedMods.size())
			orderOrStateChanged = true;
		// if it does match, compare the mods themselves
		else
			for (int i = 0; i < mods.size(); i++)
			{
				if (!mods[i].strongCompare(orderedMods[i]))
				{
					orderOrStateChanged = true;
					break;
				}
			}
	}
	beginResetModel();
	mods.swap(orderedMods);
	endResetModel();
	if (orderOrStateChanged)
	{
		emit changed();
	}
	return true;
}

void ModList::directoryChanged(QString path)
{
	update();
}

bool ModList::isValid()
{
	return m_dir.exists() && m_dir.isReadable();
}

bool ModList::installMod(const QString &filename)
{
	// NOTE: fix for GH-1178: remove trailing slash to avoid issues with using the empty result of QFileInfo::fileName
	QFileInfo fileinfo(FS::NormalizePath(filename));

	qDebug() << "installing: " << fileinfo.absoluteFilePath();

	if (!fileinfo.exists() || !fileinfo.isReadable())
	{
		return false;
	}
	Mod m(fileinfo);
	if (!m.valid())
		return false;

	auto type = m.type();
	if (type == Mod::MOD_UNKNOWN)
		return false;
	if (type == Mod::MOD_SINGLEFILE || type == Mod::MOD_ZIPFILE || type == Mod::MOD_LITEMOD)
	{
		QString newpath = FS::PathCombine(m_dir.path(), fileinfo.fileName());
		if (!QFile::copy(fileinfo.filePath(), newpath))
			return false;
		m.repath(newpath);
		update();
		return true;
	}
	else if (type == Mod::MOD_FOLDER)
	{

		QString from = fileinfo.filePath();
		QString to = FS::PathCombine(m_dir.path(), fileinfo.fileName());
		if (!FS::copy(from, to)())
			return false;
		m.repath(to);
		update();
		return true;
	}
	return false;
}

bool ModList::deleteMod(int index)
{
	if (index >= mods.size() || index < 0)
		return false;
	Mod &m = mods[index];
	if (m.destroy())
	{
		emit changed();
		return true;
	}
	return false;
}

bool ModList::deleteMods(int first, int last)
{
	for (int i = first; i <= last; i++)
	{
		Mod &m = mods[i];
		m.destroy();
	}
	emit changed();
	return true;
}

int ModList::columnCount(const QModelIndex &parent) const
{
	return 3;
}

QVariant ModList::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= mods.size())
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole:
		switch (column)
		{
		case NameColumn:
			return mods[row].name();
		case VersionColumn:
			return mods[row].version();

		default:
			return QVariant();
		}

	case Qt::ToolTipRole:
		return mods[row].mmc_id();

	case Qt::CheckStateRole:
		switch (column)
		{
		case ActiveColumn:
			return mods[row].enabled() ? Qt::Checked : Qt::Unchecked;
		default:
			return QVariant();
		}
	default:
		return QVariant();
	}
}

bool ModList::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= rowCount(index) || !index.isValid())
	{
		return false;
	}

	if (role == Qt::CheckStateRole)
	{
		auto &mod = mods[index.row()];
		if (mod.enable(!mod.enabled()))
		{
			emit dataChanged(index, index);
			return true;
		}
	}
	return false;
}

QVariant ModList::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		switch (section)
		{
		case ActiveColumn:
			return QString();
		case NameColumn:
			return tr("Name");
		case VersionColumn:
			return tr("Version");
		default:
			return QVariant();
		}

	case Qt::ToolTipRole:
		switch (section)
		{
		case ActiveColumn:
			return tr("Is the mod enabled?");
		case NameColumn:
			return tr("The name of the mod.");
		case VersionColumn:
			return tr("The version of the mod.");
		default:
			return QVariant();
		}
	default:
		return QVariant();
	}
	return QVariant();
}

Qt::ItemFlags ModList::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
	if (index.isValid())
		return Qt::ItemIsUserCheckable | defaultFlags;
	else
		return defaultFlags;
}
