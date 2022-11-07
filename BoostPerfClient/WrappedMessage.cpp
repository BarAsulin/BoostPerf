//#define _CRT_SECURE_NO_WARNINGS
//#include "WrappedMessage.h"
//#include <cstring>
//#include <iostream>
//
//enum class MessageType :size_t
//{
//	SINGLE_MESSAGE_TYPE = 123
//};
//
//
//const char* WrappedMessage::data() const
//{
//	return m_data;
//}
//
//char* WrappedMessage::data()
//{
//	return m_data;
//}
//
//size_t WrappedMessage::length()const
//{
//	return header_length + m_body_length;
//}
//
//const char* WrappedMessage::body() const
//{
//	return m_data + header_length;
//}
//
//char* WrappedMessage::body()
//{
//	return m_data + header_length;
//}
//
//size_t WrappedMessage::body_length() const
//{
//	return m_body_length;
//}
//
//void WrappedMessage::body_length(size_t new_length)
//{
//	m_body_length = new_length;
//	if (m_body_length > max_body_length)
//	{
//		m_body_length = max_body_length;
//	}
//}
//
//bool WrappedMessage::decode_header()
//{
//	char header[header_length + 1]{};
//	std::strncat(header, m_data, header_length);
//	if (m_body_length > max_body_length)
//	{
//		m_body_length = 0;
//		std::cout << "returning false from decodeheader()\n";
//		return false;
//	}
//	return true;
//}
//void WrappedMessage::encode_header()
//{
//	char header[header_length + 1]{};
//	MessageType temp = MessageType::SINGLE_MESSAGE_TYPE;
//	std::memcpy(m_data, &temp, header_length);
//
//}
//;
//
//
//
#include "WrappedMessage.h"
#include <cstring>
#include <iostream>
#include <string>


const char* WrappedMessage::data() const
{
	return m_data;
}

char* WrappedMessage::data()
{
	return m_data;
}

size_t WrappedMessage::length()const
{
	return header_length + m_body_length;
}

const char* WrappedMessage::body() const
{
	return m_data + header_length;
}

char* WrappedMessage::body()
{
	return m_data + header_length;
}

size_t WrappedMessage::body_length() const
{
	return m_body_length;
}

void WrappedMessage::body_length(size_t new_length)
{
	m_body_length = new_length;
	if (m_body_length > max_body_length)
	{
		m_body_length = max_body_length;
	}
}


bool WrappedMessage::decode_header() // the purpose of this method is to review the first 4 bytes of the data, and determine the body length and assign to body_legnth
{
	char tempHeader[header_length + 1];
	std::memcpy(tempHeader, m_data, header_length);
	m_body_length = std::atoi(tempHeader);
	if (m_body_length > max_body_length)
	{
		m_body_length = 0;
		return false;
	}
	return true;
}

void WrappedMessage::encode_header()  // the purpose of this method is to look at the body length and write it's size to the first 4 bytes of the data.
{
	std::string bodySizeString = std::to_string(m_body_length);
	std::memcpy(m_data, bodySizeString.data(), header_length);
}

const std::string WrappedMessage::exampleDataToSend = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvw"
"xyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|";


