#ifndef IBOOST_PERF
#define IBOOST_PERF


template <typename SocketType>
class IBoostPerf
{
public:
	virtual ~IBoostPerf();

	//virtual void runSocketsLoop() = 0;
	//virtual void dispatchSocketsForWork() = 0;
	virtual void addSocket() = 0;
	virtual void cleanupClosedSockets() = 0;
	virtual void initializeTempSocket() = 0;
	
};

template<typename SocketType>
inline IBoostPerf<SocketType>::~IBoostPerf() {}
#endif // IBOOST_PERF


