#ifndef BYTES_CONVERSION
#define BYTES_CONVERSION
inline size_t bytesToKiB(size_t bytes)
{
	return bytes / 1024;
}

inline size_t bytesToMiB(size_t bytes)
{
	return bytes / 1048576;
}

inline size_t bytesToGiB(size_t bytes)
{
	return bytes / 1099511627776;
}

inline size_t bytesToMbytes(size_t bytes)
{
	//return bytes / 1.0e6;
	return bytes / 1000000;
}



#endif // !BYTES_CONVERSION

