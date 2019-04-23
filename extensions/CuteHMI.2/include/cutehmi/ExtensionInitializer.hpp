#ifndef INCLUDECUTEHMIINIT_HPP
#define INCLUDECUTEHMIINIT_HPP

#include "internal/common.hpp"
#include "NonCopyable.hpp"

#include <functional>

namespace cutehmi {

/**
 * Extension initializer template. Extension initializer can be used to initialize an extension. This is achieved by subclassing
 * the template and creating a global instance of a derived class, which is a subject of static initialization. This relies on
 * following C++ principle.
 *
 * > "If a variable with static storage duration has initialization or a destructor with side eﬀects, it shall not be eliminated
 * >  even if it appears to be unused, except that a class object or its copy/move may be eliminated as speciﬁed in 11.9.5."
 *                                                                      @quote{-- C++ Standard (6.6.5.1/2 N4810) [basic.stc.static]}
 *
 * Extension initializer counts its own references and runs initialization and deinitialization code only once - for the first
 * constructed and last destroyed instance.
 *
 * @note Special care must be taken for static builds, because a global variable may be skipped by a linker, if it is not used by
 * resulting binary.
 *
 * @warning Beware of static initialization order fiasco (SIOF)! The order in which global instances are initialized is undefined.
 * This means that initialization code should be self-contained and it can not rely on other statically initialized objects,
 * including initialization based on this template performed by other extensions.
 */
template <class DERIVED>
class ExtensionInitializer:
	public NonCopyable
{
	public:
		/**
		 * Constructor.
		 * @param init initialization code. Typically a lambda expression can be passed for initialization code.
		 * @param deinit deinitialization code or @p nullptr if there is no deinitialization.
		 */
		ExtensionInitializer(std::function<void()> init, std::function<void()> deinit = nullptr);

	protected:
		~ExtensionInitializer();

	private:
		struct Members
		{
			std::function<void()> init;
			std::function<void()> deinit;
		};

		MPtr<Members> m;

		static QAtomicInt M_RefCtr;
};

template <class DERIVED>
QAtomicInt ExtensionInitializer<DERIVED>::M_RefCtr;

template <class DERIVED>
ExtensionInitializer<DERIVED>::ExtensionInitializer(std::function<void()> init, std::function<void()> deinit):
	m(new Members{init, deinit})
{
	M_RefCtr.ref();
	if (M_RefCtr == 1)
		m->init();
}

template <class DERIVED>
ExtensionInitializer<DERIVED>::~ExtensionInitializer()
{
	if (!M_RefCtr.deref()) {
		if (m->deinit)
			m->deinit();
	}
}

}

#endif // INCLUDECUTEHMIINIT_HPP
