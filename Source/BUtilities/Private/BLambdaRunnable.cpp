/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BLambdaRunnable.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/Async.h"

void FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread(TFunction<void()> InFunction)
{
	Async(EAsyncExecution::Thread, InFunction);
}

void FBLambdaRunnable::RunLambdaOnBackgroundThreadPool(TFunction<void()> InFunction)
{
	FBLambdaThreadPoolManager::Get().EnqueueTask(InFunction);
}

void FBLambdaRunnable::RunLambdaOnGameThread(TFunction<void()> InFunction)
{
	FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

// Thread pool implementation
FBLambdaThreadPoolManager::FBLambdaThreadPoolManager()
{
	for (int32 i = 0; i < NUMBER_OF_WORKER_THREADS; i++)
	{
		StartNewPoolThread_TSEnsured();
	}
}

void FBLambdaThreadPoolManager::EnqueueTask(TFunction<void()> InTask)
{
	{
		FScopeLock Lock(&EnqueuedTasks_Mutex);
		EnqueuedTasks.Enqueue(InTask);
	}

	int32 NumCurrentWorker = WorkerThreadEvents.Num();

	for (int32 i = 0; i < NumCurrentWorker; i++)
	{
		if (*(WorkerThreadEvents[i].Get<1>()) == true) //Sleeping == true
		{
			FScopeLock WorkerStateLock(WorkerThreadEvents[i].Get<0>());

			//Still sleeping?
			if (*(WorkerThreadEvents[i].Get<1>()) == true) //Sleeping == true
			{
				WorkerThreadEvents[i].Get<2>()->Trigger();
				return;
			}
		}
	}
}

bool FBLambdaThreadPoolManager::TrySafelyDequeuing(TFunction<void()>& Result)
{
	FScopeLock Lock(&EnqueuedTasks_Mutex);
	return EnqueuedTasks.Dequeue(Result);
}

void FBLambdaThreadPoolManager::StartNewPoolThread_TSEnsured()
{
	FCriticalSection* NewMutex = new FCriticalSection;
	bool* NewSleepingState = new bool(false);
	FEvent* NewEvent = FGenericPlatformProcess::GetSynchEventFromPool();

	WorkerThreadEvents.Add(TTuple<FCriticalSection*, bool*, FEvent*>(NewMutex, NewSleepingState, NewEvent));

	FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread([this, NewMutex, NewSleepingState, NewEvent]()
		{
			while (true)
			{
				TFunction<void()> ExecuteTask;
				while (TrySafelyDequeuing(ExecuteTask))
				{
					ExecuteTask();
				}

				*NewSleepingState = true;
				NewEvent->Wait();

				{
					FScopeLock WorkerStateLock(NewMutex);
					NewEvent->Reset();
					*NewSleepingState = false;
				}
			}
		});
}