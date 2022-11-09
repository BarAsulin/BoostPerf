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

size_t WrappedMessage::getBodyLength() const
{
	return m_body_length;
}

void WrappedMessage::setBodyLength(size_t new_length)
{
	m_body_length = new_length;
	if (m_body_length > max_body_length)
	{
		m_body_length = max_body_length;
	}
}


bool WrappedMessage::decodeHeader() // decode the header and assign to body length.
{	
	char tempHeader[header_length + 1];
	std::memcpy(tempHeader, m_data, header_length);
	m_body_length = std::stoi(tempHeader);
	if (m_body_length > max_body_length)
	{
		m_body_length = 0;
		return false;
	}
	return true;
}

void WrappedMessage::encodeHeader()  // encode the body length in the first few bytes of the packet.
{
	std::string bodySizeString = std::to_string(m_body_length);
	std::memcpy(m_data, bodySizeString.data(), header_length);
}

const std::string WrappedMessage::exampleDataToSend = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvw"
"xyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|";


