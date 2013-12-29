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


#ifndef ZQL_DATETIME
#define ZQL_DATETIME

namespace libep
{
	class CDateTime
	{
	public:
		CDateTime();
		CDateTime(int Year, int Month, int Day, int Hour, int Minute, int Second);
		CDateTime(int Year, int Month, int Day);
		CDateTime(time_t Time);
		CDateTime(const string &TimeStr);

		CDateTime operator +(const CDateTime &Time) const;
		CDateTime &operator +=(const CDateTime &Time);
		CDateTime operator -(const CDateTime &Time) const;
		CDateTime &operator -=(const CDateTime &Time);

		bool operator ==(const CDateTime &Time) const;
		bool operator !=(const CDateTime &Time) const;
		bool operator >(const CDateTime &Time) const;
		bool operator >=(const CDateTime &Time) const;
		bool operator <(const CDateTime &Time) const;
		bool operator <=(const CDateTime &Time) const;

		string FormatDateTime() const;
		string FormatDate() const;
		string FormatTime() const;
		static CDateTime Now();
		static CDateTime Today();

		int GetYear() const;
		int GetMonth() const;
		int GetDay() const;
		int GetHour() const;
		int GetMinute() const;
		int GetSecond() const;
		time_t GetNumeric() const {return TimeValue;};

	private:
		time_t TimeValue;
	};
}

#endif

