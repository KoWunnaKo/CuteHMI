#include <cutehmi/dataacquisition/Schema.hpp>
#include <cutehmi/dataacquisition/Exception.hpp>

#include "../../../cutehmi.dirs.hpp"

#include <QFile>
#include <QSqlRecord>

namespace cutehmi {
namespace dataacquisition {

Schema::Schema(QObject * parent):
	DataObject(parent),
	m(new Members)
{
}

QString Schema::name() const
{
	return m->name;
}

void Schema::setName(const QString & name)
{
	if (m->name != name) {
		m->name = name;
		emit nameChanged();
	}
}

QString Schema::user() const
{
	return m->user;
}

void Schema::setUser(const QString & user)
{
	if (m->user != user) {
		m->user = user;
		emit userChanged();
	}
}

void Schema::create()
{
	worker([this](QSqlDatabase & db) {

		bool warning = false;
		bool error = false;

		if (db.driverName() == "QPSQL") {
			QSqlQuery query(db);
			try {
				CUTEHMI_DEBUG("Creating schema...");

				QString queryString = readScript(POSTGRESQL_SCRIPTS_SUBDIR, "create.sql").arg(name());
				CUTEHMI_DEBUG("SQL query:\n```\n" << queryString + "\n```");

				if (!query.exec(queryString))
					error = true;
				pushError(query.lastError());
				query.finish();
			} catch (const Exception & e) {
				CUTEHMI_CRITICAL(e.what());
			}

			if (!user().isEmpty()) {
				CUTEHMI_DEBUG("Setting schema owner...");

				const char * alterSchemaQuery = R"SQL(
					ALTER SCHEMA %1 OWNER TO %2;
				)SQL";

				if (!query.exec(QString(alterSchemaQuery).arg(name()).arg(user())))
					warning = true;

				pushError(query.lastError());
				query.finish();
			}
		} else if (db.driverName() == "QSQLITE") {
			QSqlQuery query(db);
			try {
				CUTEHMI_DEBUG("Creating schema...");

				QString queryString = readScript(SQLITE_SCRIPTS_SUBDIR, "create.sql").arg(name());
				QStringList queryList = queryString.split(';');
				queryList.removeLast();	// Remove empty query.
				for (auto queryIt = queryList.begin(); queryIt != queryList.end(); ++queryIt) {
					CUTEHMI_DEBUG("SQL query:\n```\n" << *queryIt + "\n```");

					if (!query.exec(*queryIt))
						error = true;
					pushError(query.lastError());
					query.finish();
				}
			} catch (const Exception & e) {
				CUTEHMI_CRITICAL(e.what());
			}
		} else
			emit errored(CUTEHMI_ERROR(tr("Driver '%1' is not supported.").arg(db.driverName())));

		if (error)
			Notification::Critical(tr("Failed to create '%1' schema.").arg(name()));
		else if (warning)
			Notification::Warning(tr("Created '%1' schema although the operation wasn't clean.").arg(name()));
		else
			Notification::Info(tr("Successfully created '%1' schema.").arg(name()));

	})->work();
}

void Schema::drop()
{
	worker([this](QSqlDatabase & db) {

		bool warning = false;
		bool error = false;

		if (db.driverName() == "QPSQL") {
			QSqlQuery query(db);
			try {
				CUTEHMI_DEBUG("Dropping schema...");

				QString queryString = readScript(POSTGRESQL_SCRIPTS_SUBDIR, "drop.sql").arg(name());
				CUTEHMI_DEBUG("SQL query:\n```\n" << queryString + "\n```");

				if (!query.exec(queryString))
					error = true;
				pushError(query.lastError());
				query.finish();
			} catch (const Exception & e) {
				CUTEHMI_CRITICAL(e.what());
			}
		} else if (db.driverName() == "QSQLITE") {
			QSqlQuery query(db);
			try {
				CUTEHMI_DEBUG("Dropping schema...");

				QString queryString = readScript(SQLITE_SCRIPTS_SUBDIR, "drop.sql").arg(name());
				QStringList queryList = queryString.split(';');
				queryList.removeLast();	// Remove empty query.
				for (auto queryIt = queryList.begin(); queryIt != queryList.end(); ++queryIt) {
					CUTEHMI_DEBUG("SQL query:\n```\n" << *queryIt + "\n```");

					if (!query.exec(*queryIt))
						warning = true;
					pushError(query.lastError());
					query.finish();
				}
			} catch (const Exception & e) {
				CUTEHMI_CRITICAL(e.what());
			}
		} else
			emit errored(CUTEHMI_ERROR(tr("Driver '%1' is not supported.").arg(db.driverName())));

		if (error)
			Notification::Critical(tr("Failed to drop '%1' schema.").arg(name()));
		else if (warning)
			Notification::Warning(tr("Dropped '%1' schema although the operation wasn't clean.").arg(name()));
		else
			Notification::Info(tr("Dropped '%1' schema.").arg(name()));

	})->work();
}

void Schema::validate()
{
	worker([this](QSqlDatabase & db) {
		if (db.driverName() == "QPSQL") {
			QSqlQuery query(db);

			bool result = true;

			result &= validatePostgresTable("history_int", query);
			result &= validatePostgresTable("history_bool", query);
			result &= validatePostgresTable("history_real", query);

			result &= validatePostgresTable("event_int", query);
			result &= validatePostgresTable("event_bool", query);
			result &= validatePostgresTable("event_real", query);

			result &= validatePostgresTable("recency_int", query);
			result &= validatePostgresTable("recency_bool", query);
			result &= validatePostgresTable("recency_real", query);

			emit validated(result);
		} else if (db.driverName() == "QSQLITE") {
			QSqlQuery query(db);

			bool result = true;

			result &= validateSqliteTable("history_int", query);
			result &= validateSqliteTable("history_bool", query);
			result &= validateSqliteTable("history_real", query);

			result &= validateSqliteTable("event_int", query);
			result &= validateSqliteTable("event_bool", query);
			result &= validateSqliteTable("event_real", query);

			result &= validateSqliteTable("recency_int", query);
			result &= validateSqliteTable("recency_bool", query);
			result &= validateSqliteTable("recency_real", query);

			emit validated(result);
		} else {
			emit errored(CUTEHMI_ERROR(tr("Driver '%1' is not supported.").arg(db.driverName())));
			emit validated(false);
		}
	})->work();
}

bool Schema::validatePostgresTable(const QString & tableName, QSqlQuery & query)
{
	bool result = true;

	CUTEHMI_DEBUG("Checking if '" << tableName << "' table exists...");
	const char * tableExistsQuery = R"SQL(
		SELECT EXISTS (
			SELECT FROM information_schema.tables
			WHERE table_schema = '%1' AND table_name = '%2'
		);
	)SQL";
	query.exec(QString(tableExistsQuery).arg(name()).arg(tableName));
	pushError(query.lastError());
	int existsIndex = query.record().indexOf("exists");
	if (query.first())
		if (!query.value(existsIndex).toBool()) {
			emit errored(CUTEHMI_ERROR(QObject::tr("Table '%1' does not exist in schema '%2'.").arg(tableName).arg(name())));
			result = false;
		}
	query.finish();

	return result;
}

bool Schema::validateSqliteTable(const QString & tableName, QSqlQuery & query)
{
	bool result = true;

	CUTEHMI_DEBUG("Checking if '" << tableName << "' table exists...");
	const char * tableExistsQuery = R"SQL(
		SELECT name FROM sqlite_master WHERE type='table' AND name='[%1.%2]';
	)SQL";
	query.exec(QString(tableExistsQuery).arg(name()).arg(tableName));
	pushError(query.lastError());
	int existsIndex = query.record().indexOf("exists");
	if (query.first())
		if (!query.value(existsIndex).toBool()) {
			emit errored(CUTEHMI_ERROR(QObject::tr("Table '%1' does not exist in schema '%2'.").arg(tableName).arg(name())));
			result = false;
		}
	query.finish();

	return result;
}

QString Schema::readScript(const QString & dbms, const QString & scriptName) const
{
	QFile file(QString(CUTEHMI_DIRS_TOOL_RELATIVE_PATH) + "/" + SQL_SCRIPTS_SUBDIR + "/" + dbms + "/" + scriptName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw Exception(QString("Could not open file '%1'").arg(file.fileName()));
	return QTextStream(& file).readAll();
}

}
}

//(c)C: Copyright © 2020, Michał Policht <michal@policht.pl>, Yuri Chornoivan <yurchor@ukr.net>. All rights reserved.
//(c)C: This file is a part of CuteHMI.
//(c)C: CuteHMI is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//(c)C: CuteHMI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//(c)C: You should have received a copy of the GNU Lesser General Public License along with CuteHMI.  If not, see <https://www.gnu.org/licenses/>.
