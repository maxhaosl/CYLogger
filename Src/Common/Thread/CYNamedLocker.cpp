#include "Common/Thread/CYNamedLocker.hpp"
#include "Common/CYPrivateDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYNamedLocker::CYNamedLocker(const TChar* pszName)
	: std::mutex()
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