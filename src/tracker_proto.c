/* 
* @Author: jql
* @Date:   2015-12-20 01:31:23
* @Last Modified by:   jql
* @Last Modified time: 2015-12-20 01:34:19
*/

#include <errno.h>
#include <string.h>

int tracker_validate_group_name(const char *group_name)
{
	const char *p;
	const char *pEnd;
	int len;

	len = strlen(group_name);
	if (0 == len)
	{
		return EINVAL;
	}

	pEnd = group_name + len;
	for (p = group_name; p < pEnd; p++)
	{
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || \
			(*p >= '0' && *p <= '9')))
		{
			return EINVAL;
		}
	}
	return 0;
}