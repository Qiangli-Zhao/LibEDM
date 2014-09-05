/*
Copyright (c) 2014, Qiangli Zhao and Yanhuang Jiang
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

* Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <ctime>
#include <cstring>
#include <string>
using namespace std;
#include "Obj.h"
#include "zString.h"
#include "DateTime.h"
using namespace libep;

CDateTime CDateTime::Now()
{
	time_t TimeNow;
	time(&TimeNow);
	return CDateTime(TimeNow);
}

CDateTime CDateTime::Today()
{
	time_t TimeNow;
	time(&TimeNow);
	return CDateTime(CDateTime(TimeNow).FormatDate());
}

CDateTime::CDateTime(int Year, int Month, int Day, int Hour, int Minute, int Second)
{
	struct tm TimeStruct;
	memset(&TimeStruct,0,sizeof(TimeStruct));

	TimeStruct.tm_year=Year-1900;
	TimeStruct.tm_mon=Month-1;
	TimeStruct.tm_mday=Day-1;
	TimeStruct.tm_hour=Hour;
	TimeStruct.tm_min=Minute;
	TimeStruct.tm_sec=Second;

	TimeValue=(int)mktime(&TimeStruct);
	if(TimeValue<0)
		throw(CError("DateTime: Time before 1970-1-1!",100,0));
}

CDateTime::CDateTime(int Year, int Month, int Day)
{
	struct tm TimeStruct;
	memset(&TimeStruct,0,sizeof(TimeStruct));

	TimeStruct.tm_year=Year-1900;
	TimeStruct.tm_mon=Month-1;
	TimeStruct.tm_mday=Day-1;

	TimeValue=(int)mktime(&TimeStruct);
	if(TimeValue<0)
		throw(CError("DateTime: Time before 1970-1-1!",101,0));
}

CDateTime::CDateTime(time_t Time)
{
	if(Time<0)
		throw(CError("DateTime: Time before 1970-1-1!",102,0));
	TimeValue=Time;
}

CDateTime::CDateTime()
{
	TimeValue=0;
}

//时间格式必须类似：1911-11-11 12:12:12，允许没有时间部分
CDateTime::CDateTime(const string &uTimeStr)
{
	if(uTimeStr.size()<=0)
	{
		TimeValue=0;
		return;
	}
	//
	struct tm TimeStruct;
	memset(&TimeStruct,0,sizeof(TimeStruct));


	//去尾
	string TimeStr=uTimeStr;
	CzString::Trim(TimeStr);
	//必须有年月部分
	TimeStruct.tm_year=CzString::ToInt(TimeStr.substr(0,4))-1900;
	TimeStruct.tm_mon=CzString::ToInt(TimeStr.substr(5,2))-1;
	TimeStruct.tm_mday=CzString::ToInt(TimeStr.substr(8,2));
	//可能没有时间部分
	if(TimeStr.find(':',9)==13)
	{
		TimeStruct.tm_hour=CzString::ToInt(TimeStr.substr(11,2));
		TimeStruct.tm_min=CzString::ToInt(TimeStr.substr(14,2));
		TimeStruct.tm_sec=CzString::ToInt(TimeStr.substr(17,2));
	}

	TimeValue=(int)mktime(&TimeStruct);
	if(TimeValue<0)
		throw(CError("DateTime: Time before 1970-1-1!",101,0));
}

CDateTime CDateTime::operator +(const CDateTime &Time) const
{
	CDateTime x(TimeValue);
	x.TimeValue+=Time.TimeValue;
	return x;
}

CDateTime &CDateTime::operator +=(const CDateTime &Time)
{
	TimeValue+=Time.TimeValue;
	return *this;
}

CDateTime CDateTime::operator -(const CDateTime &Time) const
{
	CDateTime x(TimeValue);
	x.TimeValue-=Time.TimeValue;
	if(x.TimeValue<0)
		throw(CError("DateTime: Time before 1970-1-1!",103,0));
	return x;
}

CDateTime &CDateTime::operator -=(const CDateTime &Time)
{
	TimeValue-=Time.TimeValue;
	if(TimeValue<0)
		throw(CError("DateTime: Time before 1970-1-1!",104,0));
	return *this;
}

bool CDateTime::operator ==(const CDateTime &Time) const
{
	return (TimeValue==Time.TimeValue);
}

bool CDateTime::operator !=(const CDateTime &Time) const
{
	return (TimeValue!=Time.TimeValue);
}

bool CDateTime::operator >(const CDateTime &Time) const
{
	return (TimeValue>Time.TimeValue);
}

bool CDateTime::operator >=(const CDateTime &Time) const
{
	return (TimeValue>=Time.TimeValue);
}

bool CDateTime::operator <(const CDateTime &Time) const
{
	return (TimeValue<Time.TimeValue);
}

bool CDateTime::operator <=(const CDateTime &Time) const
{
	return (TimeValue<=Time.TimeValue);
}

string CDateTime::FormatDateTime() const
{
	//转化为时间结构
	struct tm TimeStruct;
	time_t TimeData=(time_t)TimeValue;
	TimeStruct=*localtime(&TimeData);

	char TempStr[64];
	strftime(TempStr,sizeof(TempStr)-1,"%Y-%m-%d %H:%M:%S",&TimeStruct);

	return string(TempStr);
}

string CDateTime::FormatDate() const
{
	//转化为时间结构
	struct tm TimeStruct;
	time_t TimeData=(time_t)TimeValue;
	TimeStruct=*localtime(&TimeData);

	char TempStr[64];
	strftime(TempStr,sizeof(TempStr)-1,"%Y-%m-%d",&TimeStruct);

	return string(TempStr);
}

string CDateTime::FormatTime() const
{
	//转化为时间结构
	struct tm TimeStruct;
	time_t TimeData=(time_t)TimeValue;
	TimeStruct=*localtime(&TimeData);

	char TempStr[64];
	strftime(TempStr,sizeof(TempStr)-1,"%H:%M:%S",&TimeStruct);
	return string(TempStr);
}

