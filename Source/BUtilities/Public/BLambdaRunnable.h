/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"

#define NUMBER_OF_WORKER_THREADS 256

class FBLambdaThreadPoolManager
{
public:
	static FBLambdaThreadPoolManager& Get() 
	{
		static FBLambdaThreadPoolManager Instance;
		return Instance;
	}

	void EnqueueTask(TFunction<void()> InTask);

private:
	FBLambdaThreadPoolManager();
	FBLambdaThreadPoolManager(const FBLambdaThreadPoolManager&) = delete;
	FBLambdaThreadPoolManager& operator=(const FBLambdaThreadPoolManager&) = delete;

	TQueue<TFunction<void()>> EnqueuedTasks;
	FCriticalSection EnqueuedTasks_Mutex;
	bool TrySafelyDequeuing(TFunction<void()>& Result);

	void StartNewPoolThread_TSEnsured();

	TArray<TTuple<FCriticalSection*, bool*, FEvent*>> WorkerThreadEvents;
};

class BUTILITIES_API FBLambdaRunnable : public FRunnable
{
public:
	/*
	Runs the passed lambda on the background thread, new thread per call
	*/
	static void RunLambdaOnDedicatedBackgroundThread(TFunction<void()> InFunction);

	static void RunLambdaOnBackgroundThreadPool(TFunction<void()> InFunction);

	static void RunLambdaOnGameThread(TFunction<void()> InFunction);
};