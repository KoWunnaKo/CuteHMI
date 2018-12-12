#ifndef H_MODULES_CUTEHMI_u_1_INCLUDE_CUTEHMI_LOGGINGMACROS_HPP
#define H_MODULES_CUTEHMI_u_1_INCLUDE_CUTEHMI_LOGGINGMACROS_HPP

#include <QLoggingCategory>

#ifndef CUTEHMI_NDEBUG
	#define CUTEHMI_DEBUG(EXPR) qCDebug(loggingCategory()).nospace().noquote() << EXPR
#else
	#define CUTEHMI_DEBUG(EXPR) (void)0
#endif

#ifndef CUTEHMI_NINFO
	#define CUTEHMI_INFO(EXPR) qCInfo(loggingCategory()).nospace().noquote() << EXPR
#else
	#define CUTEHMI_INFO(EXPR) (void)0
#endif

#ifndef CUTEHMI_NWARNING
	#define CUTEHMI_WARNING(EXPR) qCWarning(loggingCategory()).nospace().noquote() << EXPR
#else
	#define CUTEHMI_WARNING(EXPR) (void)0
#endif

#ifndef CUTEHMI_NCRITICAL
	#define CUTEHMI_CRITICAL(EXPR) qCCritical(loggingCategory()).nospace().noquote() << EXPR
#else
	#define CUTEHMI_CRITICAL(EXPR) (void)0
#endif

// Changes order of arguments so that CUTEHMI_DIE can be used with single or multiple arguments (__VA_ARGS__ can not be empty).
template<typename ...ARGS>
void cutehmiFatalWrapper(const char * category, const char * message, ARGS... args)
{
	qFatal(message, category, args...);
}

#define CUTEHMI_DIE(...) cutehmiFatalWrapper(loggingCategory().categoryName(), "%s [fatal]: " __VA_ARGS__)

#ifndef CUTEHMI_NDEBUG
	#define CUTEHMI_ASSERT(EXPR, MSG) Q_ASSERT_X(EXPR, __FILE__, MSG)
#else
	#define CUTEHMI_ASSERT(EXPR, MSG) (void)0
#endif

#endif

//(c)MP: Copyright © 2018, Michal Policht. All rights reserved.
//(c)MP: This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
