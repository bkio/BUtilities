/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include <sstream>

struct BUTILITIES_API FBQueueStreamParameter
{
public:
    TSharedPtr<FCriticalSection> SharedDataListMutex;
    TSharedPtr<TDoubleLinkedList<TArray<uint8>>> SharedDataList;
    TSharedPtr<bool> WriteDone;

    FBQueueStreamParameter() {}
    FBQueueStreamParameter(TSharedPtr<FCriticalSection> _SharedDataListMutex, TSharedPtr<TDoubleLinkedList<TArray<uint8>>> _SharedDataList, TSharedPtr<bool> _WriteDone);

    static FBQueueStreamParameter MakeInitialized();
};

class FBQueueStreamBuf : public std::streambuf
{
public:
    FBQueueStreamBuf(const FBQueueStreamParameter& _Parameter) : Parameter(_Parameter) {}

    ~FBQueueStreamBuf()
    {
        *Parameter.WriteDone.Get() = true;
    }

    FBQueueStreamBuf(const FBQueueStreamBuf&) = delete;
    FBQueueStreamBuf(FBQueueStreamBuf&&) = default;
    FBQueueStreamBuf& operator = (const FBQueueStreamBuf&) = delete;
    FBQueueStreamBuf& operator = (FBQueueStreamBuf&&) = default;

	virtual std::streamsize xsgetn(char* Dest, std::streamsize ExpectedSize) override;

    virtual std::streamsize xsputn(const char* Source, std::streamsize Size) override;

private:
    FBQueueStreamParameter Parameter;

    bool IsWriteDone() const
    {
        return *Parameter.WriteDone.Get();
    }

    bool IsQueueEmpty() const;
    void Enqueue(TArray<uint8>& Data);
    void AddToQueueHead(TArray<uint8>& Data);
	bool TryDequeue(TArray<uint8>& Result);
};
    
class BUTILITIES_API FBIQueueStream : public std::istream
{
public:
    FBIQueueStream(const FBQueueStreamParameter& _Parameter) : std::istream(new FBQueueStreamBuf(_Parameter))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~FBIQueueStream()
    {
        delete rdbuf();
    }
};

class BUTILITIES_API FBOQueueStream : public std::ostream
{
public:
    FBOQueueStream(const FBQueueStreamParameter& _Parameter) : std::ostream(new FBQueueStreamBuf(_Parameter))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~FBOQueueStream()
    {
        delete rdbuf();
    }
};