#ifndef H_EXTENSIONS_CUTEHMI_DATAACQUISITION_0_INCLUDE_CUTEHMI_DATAACQUISITION_INTERNAL_HISTORYTABLE_HPP
#define H_EXTENSIONS_CUTEHMI_DATAACQUISITION_0_INCLUDE_CUTEHMI_DATAACQUISITION_INTERNAL_HISTORYTABLE_HPP

#include "common.hpp"
#include "TableObject.hpp"
#include "TagCache.hpp"
#include "TableNameTraits.hpp"

#include <QHash>

#include <limits>

namespace cutehmi {
namespace dataacquisition {
namespace internal {

template <typename T>
class HistoryTable:
	public TableObject
{
	public:
		typedef T type;

		struct Tuple
		{
			T open = T();
			T close = T();
			T min = std::numeric_limits<T>::max();
			T max = std::numeric_limits<T>::min();
			QDateTime openTime;
			QDateTime closeTime;
			int count = 0;
		};

		typedef QHash<QString, Tuple> TuplesContainer;

		HistoryTable(TagCache * tagCache, Schema * schema, QObject * parent = nullptr);

		void insert(const TuplesContainer & tuples);

	protected:
		struct ColumnValues
		{
			QStringList tagName;
			QVariantList open;
			QVariantList close;
			QVariantList min;
			QVariantList max;
			QVariantList openTime;
			QVariantList closeTime;
			QVariantList count;

			ColumnValues(const TuplesContainer & tuples);
		};

		TagCache * tagCache() const;

	private:
		struct Members
		{
			TagCache * tagCache;
		};

		MPtr<Members> m;
};

template <typename T>
HistoryTable<T>::HistoryTable(TagCache * tag, Schema * schema, QObject * parent):
	TableObject(schema, parent),
	m(new Members{tag})
{
}

template <typename T>
void HistoryTable<T>::insert(const HistoryTable<T>::TuplesContainer & tuples)
{
	ColumnValues columnValues(tuples);
	QString tableName = TableNameTraits<T>::Affixed("history");

	worker([this, columnValues, tableName](QSqlDatabase & db) {
		if (db.driverName() == "QPSQL") {
			QSqlQuery query(db);
			CUTEHMI_DEBUG("Storing '" << tableName << "' values...");
			QVariantList tagIds;
			for (QStringList::const_iterator tagName = columnValues.tagName.begin(); tagName != columnValues.tagName.end(); ++tagName)
				tagIds.append(tagCache()->getId(*tagName, db));

			query.prepare(QString("INSERT INTO %1.%2(tag_id, open, close, min, max, open_time, close_time, count) VALUES (:tagId, :open, :close, :min, :max, :open_time, :close_time, :count)").arg(schema()->name()).arg(tableName));
			query.bindValue(":tagId", tagIds);
			query.bindValue(":open", columnValues.open);
			query.bindValue(":close", columnValues.close);
			query.bindValue(":min", columnValues.min);
			query.bindValue(":max", columnValues.max);
			query.bindValue(":open_time", columnValues.openTime);
			query.bindValue(":close_time", columnValues.closeTime);
			query.bindValue(":count", columnValues.count);
			query.execBatch();

			pushError(query.lastError());
			query.finish();
		} else if (db.driverName() == "QSQLITE") {
			QSqlQuery query(db);
			CUTEHMI_DEBUG("Storing '" << tableName << "' values...");
			QVariantList tagIds;
			for (QStringList::const_iterator tagName = columnValues.tagName.begin(); tagName != columnValues.tagName.end(); ++tagName)
				tagIds.append(tagCache()->getId(*tagName, db));

			query.prepare(QString("INSERT INTO [%1.%2](tag_id, open, close, min, max, open_time, close_time, count) VALUES (:tagId, :open, :close, :min, :max, :open_time, :close_time, :count)").arg(schema()->name()).arg(tableName));
			query.bindValue(":tagId", tagIds);
			query.bindValue(":open", columnValues.open);
			query.bindValue(":close", columnValues.close);
			query.bindValue(":min", columnValues.min);
			query.bindValue(":max", columnValues.max);
			query.bindValue(":open_time", columnValues.openTime);
			query.bindValue(":close_time", columnValues.closeTime);
			query.bindValue(":count", columnValues.count);
			query.execBatch();

			pushError(query.lastError());
			query.finish();
		} else
			emit errored(CUTEHMI_ERROR(tr("Driver '%1' is not supported.").arg(db.driverName())));
	})->work();
}

template <typename T>
HistoryTable<T>::ColumnValues::ColumnValues(const HistoryTable<T>::TuplesContainer & tuples)
{
	for (typename HistoryTable<T>::TuplesContainer::const_iterator it = tuples.begin(); it != tuples.end(); ++it) {
		tagName.append(it.key());
		open.append(it->open);
		close.append(it->close);
		min.append(it->min);
		max.append(it->max);
		openTime.append(it->openTime);
		closeTime.append(it->closeTime);
		count.append(it->count);
	}
}

template <typename T>
TagCache * HistoryTable<T>::tagCache() const
{
	return m->tagCache;
}

}
}
}

#endif

//(c)C: Copyright © 2020, Michał Policht <michal@policht.pl>. All rights reserved.
//(c)C: This file is a part of CuteHMI.
//(c)C: CuteHMI is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//(c)C: CuteHMI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//(c)C: You should have received a copy of the GNU Lesser General Public License along with CuteHMI.  If not, see <https://www.gnu.org/licenses/>.
