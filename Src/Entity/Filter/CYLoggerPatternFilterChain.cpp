#include "Entity/Filter/CYLoggerPatternFilterChain.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerPatternFilterChain::CYLoggerPatternFilterChain()
    : ICYLoggerPatternFilter()
{
    AddCustomPatternFiled();
}

CYLoggerPatternFilterChain::~CYLoggerPatternFilterChain()
{

}

/**
 * @brief Add default filter characters to dictionary.
*/
void CYLoggerPatternFilterChain::AddCustomPatternFiled()
{
    //*this << ICYLoggerPatternFilter::TupleFieldType(TEXT("src"), TEXT("dst"));
}

CYLOGGER_NAMESPACE_END