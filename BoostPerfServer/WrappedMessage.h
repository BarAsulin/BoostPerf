#pragma once
#include <string>
class WrappedMessage
{

public:
	enum { header_length = 4 };
	enum { max_body_length = 1024 };



	const char* data() const;
	char* data();
	size_t length()const;
	const char* body() const;
	char* body();
	size_t getBodyLength() const;
	void setBodyLength(size_t new_length);
	bool decodeHeader();
	void encodeHeader();
	const static std::string exampleDataToSend;

private:
	char m_data[header_length + max_body_length];
	size_t m_body_length{};
};

