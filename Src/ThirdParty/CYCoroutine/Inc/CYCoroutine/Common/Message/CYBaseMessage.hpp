#ifndef __CY_BASE_MESSAGE_HPP__
#define __CY_BASE_MESSAGE_HPP__

#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

class CYBaseMessage
{
public:
	/**
	* @param strChannel message channel.
	* @param nMsgType message type
	* @param nServerCode message severity code
	* @param strFunction code location - can be function name or anything else
	* @param m_strMsg text of the message
	* @param inThrowMode if true cannot throw
	*/
	CYBaseMessage(const TString& strChannel, int nMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine);


	/**
	 * @param strChannel message channel.
	 * @param nMsgType message type
	 * @param nServerCode  message severity code
	 * @param strMsg text of the message
	*/
	CYBaseMessage(const TString& strChannel, int nMsgType, int nServerCode, const TString& strMsg);

	/**
	 * @brief Destructor.
	*/
	virtual ~CYBaseMessage() = default;

	/**
	* @brief Every derived class should implement this method.
	*/
	virtual const TString GetFormatMessage() const = 0;

	/**
	 * @brief Get Message Type.
	*/
	int GetMsgType() const;

	/**
	 * @brief Get Server Code.
	*/
	int GetSeverCode() const;

	/**
	 * @brief Get Log Message Function.
	*/
	const TString& GetFunction() const;

	/**
	 * @brief Get Log Message.
	*/
	const TString& GetMsg() const;

	/**
	 * @brief Get Log Channel.
	*/
	const TString& GetChannel() const;

	/**
	 * @brief Get File Path.
	*/
	const TString& GetFile() const;

	/**
	 * @brief Get Code Line.
	*/
	int GetLine() const;

	/**
	 * @brief Set Channel.
	*/
	void SetChannel(const TString& strChannel);

	/**
	 * @brief Get Log TimeStamp.
	*/
	const CYTimeStamps& GetTimeStamp() const;

	/**
	 * @brief Get Thread Id.
	*/
	unsigned long GetThreadId() const;

    /**
     * @brief Get Type Index
    */
	virtual int32_t GetTypeIndex() = 0;

private:
	int    		    m_nMsgType;
	int				m_nLine;
	int				m_nServerCode;
	TString         m_strChannel;
	TString			m_strMsg;
	TString			m_strFile;
	TString			m_strFunction;
	CYTimeStamps	m_objTimeStamp;
	unsigned long	m_nThreadId;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_BASE_MESSAGE_HPP__