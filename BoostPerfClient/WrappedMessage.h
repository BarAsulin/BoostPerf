//#pragma once
//class WrappedMessage
//{
//
//public:
//	enum { header_length = 8 };
//	enum { max_body_length = 1024 };
//
//
//
//	const char* data() const;
//	char* data();
//	size_t length()const;
//	const char* body() const;
//	char* body();
//	size_t body_length() const;
//	void body_length(size_t new_length);
//	bool decode_header();
//	void encode_header();
//
//
//private:
//	char m_data[header_length + max_body_length];
//	size_t m_body_length{};
//};
//

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
	size_t body_length() const;
	void body_length(size_t new_length);
	bool decode_header();
	void encode_header();
	const static std::string exampleDataToSend;

private:
	char m_data[header_length + max_body_length];
	size_t m_body_length{};
};

