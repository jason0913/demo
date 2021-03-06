#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "shared_func.h"
#include "logger.h"

void daemon_init(bool bCloseFiles)
{

	pid_t pid;
	int i;

	if (0 != (pid = fork()))
	{
		exit(0);
	}
	setsid();
	if((pid=fork())!=0)
	{
		exit(0);
	}
	chdir("/");

	if (true == bCloseFiles)
	{
		for (i = 0; i < 64; ++i)
		{
			close(i);
		}
	}
#ifdef __DEBUG__
	printf("daemon_init done file:%s,line:%d\n",
			__FILE__,__LINE__);
#endif
	return;
}

int init_pthread_lock(pthread_mutex_t *pthread_lock)
{

	pthread_mutexattr_t mat;

	if(0 != pthread_mutexattr_init(&mat))
	{
		logError("file:%s,line:%d," \
			"call pthread_mutexattr_t fail ,errno = %d,err info = %s",
			__FILE__,__LINE__,errno,strerror(errno));

		return errno != 0? errno:EAGAIN;

	}

	if (0 != pthread_mutexattr_settype(&mat,PTHREAD_MUTEX_ERRORCHECK))
	{
		logError("file:%s,line:%d," \
			"call pthread_mutexattr_settype fail ,errno = %d,err info = %s",
			__FILE__,__LINE__,errno,strerror(errno));

		return errno != 0? errno:EAGAIN;
	}

	if (0 != pthread_mutex_init(pthread_lock,&mat))
	{
		logError("file:%s,line:%d," \
			"call pthread_mutex_init fail ,errno = %d,err info = %s",
			__FILE__,__LINE__,errno,strerror(errno));

		return errno != 0? errno:EAGAIN;
	}

	if (0 != pthread_mutexattr_destroy(&mat))
	{
		logError("file:%s,line:%d," \
			"call pthread_mutexattr_destroy fail ,errno = %d,err info = %s",
			__FILE__,__LINE__,errno,strerror(errno));

		return errno != 0? errno:EAGAIN;
	}

#ifdef __DEBUG__
	printf("init_pthread_lock done,file:%s,line:%d\n",
			__FILE__,__LINE__);
#endif

	return 0;
}


bool fileExists(const char *filename)
{
	return access(filename,0) ==0;
}

void chopPath(char *filePath)
{
	int lastIndex;
	if ('\0' == filePath)
	{
		return;
	}
	lastIndex = strlen(filePath)-1;
	if (filePath[lastIndex] == '/')
	{
		*(filePath +lastIndex) = '\0';
	}

}
bool isDir(const char *filename)
{
	struct stat buf;
	if (0 != stat(filename,&buf))
	{
		return false;
	}
	return S_ISDIR(buf.st_mode);
}

char * trim_right(char *pStr)
{
	int len;
	char *pEnd;
	char *p;
	char ch;

	len = strlen(pStr);
	if (0 == len)
	{
		return pStr;
	}
	pEnd = pStr + strlen(pStr) -1;
	for (p = pEnd; p >= pStr; --p)
	{
		ch = *p;
		if (!(' ' == ch || '\n' == ch || '\r' == ch || '\t' == ch))
		{
			break;
		}
	}
	if (p != pEnd)
	{
		*(p +1) = '\0';
	}
	return pStr;
}

char * trim_left(char *pStr)
{
	int iLength;
	char *pTemp;
	char ch;
	int nDestlen;

	iLength = strlen(pStr);
	int i;
	for (i =0; i < iLength; i++)
	{
		ch = pStr[i];
		if (!(' ' == ch || '\n' == ch || '\r' == ch || '\t' == ch))
		{
			break;
		}
	}
	if (i == 0)
	{
		return pStr;
	}
	nDestlen = iLength - i;
	pTemp = (char *)malloc(nDestlen + 1);
	strcpy(pTemp, pStr + i);
	strcpy(pStr, pTemp);
	free(pTemp);
	return pStr;
}

char *trim(char *pStr)
{
	trim_right(pStr);
	trim_left(pStr);
	return pStr;
}

int splitEx(char *src, const char seperator, char **pCols, const int nMaxCols)
{
	char *p;
	char **pCurrent;
	int count = 0;

	if (nMaxCols <= 0)
	{
		return 0;
	}

	p = src;
	pCurrent = pCols;

	while (true)
	{
		*pCurrent = p;
		pCurrent++;

		count++;
		if (count >= nMaxCols)
		{
			break;
		}

		p = strchr(p, seperator);
		if (p == NULL)
		{
			break;
		}

		*p = '\0';
		p++;
	}

	return count;
}