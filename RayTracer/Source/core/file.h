#ifndef _FILE_H_
#define _FILE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class File
{
public:
	enum Status
	{
		Ok = 0,
		IOError,
		EOS,
		IllegalCall,
		Closed,
		UnknownError
	};
    
	enum AccessMode
	{
		Read         = 0,
		Write        = 1,
		ReadWrite    = 2,
		WriteAppend  = 3
	};
    
public:
	File();
	virtual ~File();
    
	Status setStatus();
    
	Status open(const char *filename, const AccessMode openMode);
    
	Status close();
    
	bool canRead() { return mCanRead; }
	bool canWrite() { return mCanWrite; }
    
	Status flush();
    
    Status read(U32 size, void *dst, U32 *bytesRead = NULL);
    
    Status write(U32 size, const void *src, U32 *bytesWritten = NULL);
    
private:
	void *mHandle;
	Status mStatus;
	bool mCanRead;
	bool mCanWrite;
};

#endif