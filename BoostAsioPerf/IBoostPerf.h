#ifndef IBOOST_PERF
#define IBOOST_PERF


template <typename SocketType>
class IBoostPerf
{
public:
	virtual ~IBoostPerf();


	virtual void addSocket() = 0;
	virtual void cleanupClosedSockets() = 0;
	virtual void initializeTempSocket() = 0;
	virtual void printBytes() = 0;
	virtual void runPeriodicPrint() = 0;
	virtual void runPeriodicCleanup() = 0;
};

template<typename SocketType>
inline IBoostPerf<SocketType>::~IBoostPerf() {}
#endif // IBOOST_PERF


