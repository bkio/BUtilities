/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BQueueStream.h"

FBQueueStreamParameter::FBQueueStreamParameter(TSharedPtr<FCriticalSection> _SharedDataListMutex, TSharedPtr<TDoubleLinkedList<TArray<uint8>>> _SharedDataList, TSharedPtr<bool> _WriteDone)
{
	SharedDataListMutex = _SharedDataListMutex;
	SharedDataList = _SharedDataList;
	WriteDone = _WriteDone;
}

FBQueueStreamParameter FBQueueStreamParameter::MakeInitialized()
{
	return FBQueueStreamParameter(
		MakeShareable(new FCriticalSection),
		MakeShareable(new TDoubleLinkedList<TArray<uint8>>()),
		MakeShareable(new bool(false)));
}

std::streamsize FBQueueStreamBuf::xsgetn(char* Dest, std::streamsize ExpectedSize)
{
	TArray<uint8> FinalData;

	bool bDoneRetrieving = false;

	//Underflow data
	while (true)
	{
		int32 MissingDataSize = ExpectedSize - FinalData.Num();
		if (MissingDataSize <= 0)
		{
			break;
		}

		while (IsQueueEmpty())
		{
			if (IsWriteDone())
			{
				bDoneRetrieving = true;
				break;
			}
			FPlatformProcess::Sleep(0.05f);
		}
		if (bDoneRetrieving) break;

		TArray<uint8> NewData;
		TryDequeue(NewData);
		int32 NewData_ProcessLength = NewData.Num();

		int32 SurplusDataSize = NewData_ProcessLength - MissingDataSize;
		if (SurplusDataSize > 0)
		{
			TArray<uint8> SurplusData;
			SurplusData.AddUninitialized(SurplusDataSize);

			FMemory::Memcpy(SurplusData.GetData(), NewData.GetData() + MissingDataSize, SurplusDataSize);
			AddToQueueHead(SurplusData);

			NewData_ProcessLength = MissingDataSize;
		}

		int32 PrevNumFinalData = FinalData.Num();
		FinalData.AddUninitialized(NewData_ProcessLength);
		FMemory::Memcpy(FinalData.GetData() + PrevNumFinalData, NewData.GetData(), NewData_ProcessLength);
	}

	int32 FinalData_ProcessLength = FinalData.Num();

	//Overflow data
	int32 SurplusDataSize = FinalData_ProcessLength - ExpectedSize;
	if (SurplusDataSize > 0)
	{
		TArray<uint8> SurplusData;
		SurplusData.AddUninitialized(SurplusDataSize);

		FMemory::Memcpy(SurplusData.GetData(), FinalData.GetData() + ExpectedSize, SurplusDataSize);
		AddToQueueHead(SurplusData);

		FinalData_ProcessLength = ExpectedSize;
	}

	if (FinalData_ProcessLength > 0)
	{
		FMemory::Memcpy(Dest, FinalData.GetData(), FinalData_ProcessLength);
	}

	return FinalData_ProcessLength;
}

std::streamsize FBQueueStreamBuf::xsputn(const char* Source, std::streamsize Size)
{
	if (Size == 0) return 0;

	TArray<uint8> Data;
	Data.AddUninitialized(Size);

	FMemory::Memcpy(Data.GetData(), Source, Size);

	Enqueue(Data);

	return Size;
}

//Safe queue functions
void FBQueueStreamBuf::Enqueue(TArray<uint8>& Data)
{
	FScopeLock Lock(Parameter.SharedDataListMutex.Get());
	Parameter.SharedDataList->AddTail(Data);
}

void FBQueueStreamBuf::AddToQueueHead(TArray<uint8>& Data)
{
	FScopeLock Lock(Parameter.SharedDataListMutex.Get());
	Parameter.SharedDataList->AddHead(Data);
}

bool FBQueueStreamBuf::TryDequeue(TArray<uint8>& Result)
{
	FScopeLock Lock(Parameter.SharedDataListMutex.Get());

	auto HeadNode = Parameter.SharedDataList->GetHead();
	if (HeadNode == nullptr) return false;

	Result = HeadNode->GetValue();
	Parameter.SharedDataList->RemoveNode(HeadNode);

	return true;
}

bool FBQueueStreamBuf::IsQueueEmpty() const
{
	return Parameter.SharedDataList->Num() == 0;
}