#include <windows.h>
#include <Strsafe.h>
#include <assert.h>
#include "core/file.h"

File::File() : mStatus(Closed)
{
    assert(sizeof(HANDLE) == sizeof(void *)); // Cannot cast void* to HANDLE*
    mHandle = (void *)INVALID_HANDLE_VALUE;
}


File::~File()
{
	close();
	mHandle = (void *)INVALID_HANDLE_VALUE;
}

File::Status File::setStatus()
{
	switch (GetLastError())
	{
		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_ACCESS:
		case ERROR_TOO_MANY_OPEN_FILES:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_SHARING_VIOLATION:
		case ERROR_HANDLE_DISK_FULL:
			return mStatus = IOError;
		default:
			return mStatus = UnknownError;
	 }
}

File::Status File::open(const char *filename, const AccessMode openMode)
{
   static char filebuf[2048];
	StringCchCopy(filebuf, 2048, filename);
   
   //backslash(filebuf);

	assert(INVALID_HANDLE_VALUE == (HANDLE) mHandle); // File already in use

   if (Closed != mStatus)
		close();
	
   // create the appropriate type of file...
   switch (openMode)
   {
		case Read:
			mHandle = (void *)CreateFile(filebuf,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
			break;
		case Write:
			mHandle = (void *)CreateFile(filebuf,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
			break;
		case ReadWrite:
			mHandle = (void *)CreateFile(filebuf,
                                    GENERIC_WRITE | GENERIC_READ,
                                    0,
                                    NULL,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
			break;
		case WriteAppend:
			mHandle = (void *)CreateFile(filebuf,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
			break;
		default:
        assert(false);    // impossible
   }

	if (INVALID_HANDLE_VALUE == (HANDLE) mHandle)
     return setStatus();

	// successfully created file, so set the file capabilities...
	switch (openMode)
	{
	case Read:
		mCanRead = true;
		break;
	case Write:
	case WriteAppend:
		mCanWrite = true;
		break;
	case ReadWrite:
		mCanRead = true;
		mCanWrite = true;
		break;
	}
	return mStatus = Ok;
}

File::Status File::close()
{
	if (Closed == mStatus)
		return mStatus;

	if (INVALID_HANDLE_VALUE != (HANDLE) mHandle)
	{
	  if (0 == CloseHandle((HANDLE) mHandle))
			return setStatus();
	}
	mHandle = (void *) INVALID_HANDLE_VALUE;
	return mStatus = Closed;
}

File::Status File::flush()
{
	assert(Closed != mStatus); // File closed
	assert(INVALID_HANDLE_VALUE != (HANDLE) mHandle); // Invalid file handle
	assert(true == canWrite()); // Cannot flush a read-only file

	if (0 != FlushFileBuffers((HANDLE) mHandle))
		return setStatus();
	else
		return mStatus = Ok;
}

File::Status File::read(U32 size, void *dst, U32 *bytesRead)
{
	assert(Closed != mStatus); // File closed
	assert(INVALID_HANDLE_VALUE != (HANDLE) mHandle); // Invalid file handle
	assert(NULL != dst); // Destination is NULL
	assert(true == canRead()); // Write only file
	assert(0 != size); // Size is 0

	if (Ok != mStatus || 0 == size)
		return mStatus;
	else
	{
		DWORD lastBytes;
		DWORD *bytes = (NULL == bytesRead) ? &lastBytes : (DWORD *)bytesRead;
		if (0 != ReadFile((HANDLE) mHandle, dst, size, bytes, NULL))
		{
			if(*((U32 *)bytes) != size)
				return mStatus = EOS;
		}
		else
			return setStatus();
	}
	return mStatus = Ok;
}

File::Status File::write(U32 size, const void *src, U32 *bytesWritten)
{
	assert(Closed != mStatus); // File closed
	assert(INVALID_HANDLE_VALUE != (HANDLE) mHandle); // Invalid file handle
	assert(NULL != src); // Src is NULL
	assert(true == canWrite()); // Read only file
	assert(0 != size); // Size is 0

	if ((Ok != mStatus && EOS != mStatus) || 0 == size)
		return mStatus;
	else
	{
		DWORD lastBytes;
		DWORD *bytes = (NULL == bytesWritten) ? &lastBytes : (DWORD *)bytesWritten;
		if (0 != WriteFile((HANDLE) mHandle, src, size, bytes, NULL))
			return mStatus = Ok;
		else
			return setStatus();
	}
}