#include "Src/Common/Thread/CYNamedLocker.hpp"
#include "Src/Common/CYPrivateDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYNamedLocker::CYNamedLocker(const TChar* pszName)
{
	if (pszName != nullptr)
	{
		cy_strcpy(m_szName, pszName);
	}
}

CYNamedLocker::~CYNamedLocker()
{

}

CYLOGGER_NAMESPACE_END