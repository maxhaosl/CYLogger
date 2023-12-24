#ifndef __CY_TIMESTAMPS_HPP__
#define __CY_TIMESTAMPS_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

class CYTimeStamps final
{
public:
	/**
	* Creates using a current time
	*/
	CYTimeStamps();

	/**
	 * @brief Destructor.
	*/
	virtual ~CYTimeStamps();

	/**
	* Sets itself to be a current time
	*/
	void SetTime();

	/**
	 * @brief Set Offset second.
	*/
	void SetOffsetTime(int nOffsetSec);

	/**
	 * @brief Get Time
	*/
	const int64_t GetTime() const;

	/**
	 * @brief Get Time
	*/
	const TString GetTimeStr() const;

	/**
	* Formats the timestamp - YYYYMMDD hh:mm:ss:nnnnnn where nnnnnn is microseconds.
	* @returns reference to a formatted time stamp
	*/
	const TString ToString() const;

	/**
	 * @brief Get Date Value.
	*/
	int GetYY() { return m_nYY; }
	int GetMM() { return m_nMM; }
	int GetDD() { return m_nDD; }
	int GetHR() { return m_nHR; }
	int GetMN() { return m_nMN; }
	int GetSC() { return m_nSC; }
	int GetMMN(){ return m_nMMN; }

private:
	/**
	 * @brief Set Local Time.
	*/
	inline void SetLocalTimeData();

private:
	int				m_nYY;		// Year
	int				m_nMM;		// Month
	int				m_nDD;		// Day
	int				m_nHR;		// Hour
	int				m_nMN;		// Minute
	int				m_nSC;		// Second
	int				m_nMMN;		// MicroSecond

	mutable TString m_strTime;
};

CYCOROUTINE_NAMESPACE_END

#endif // __CY_TIMESTAMPS_HPP__