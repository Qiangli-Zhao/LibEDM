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
#include <cstdlib>
#include <cstdio>
#include <string>
using namespace std;
#include "zString.h"
using namespace libedm;

void CzString::TrimLeft(string &Str)
{
	//É¾³ýÇ°µ¼¿ØÖÆ×Ö·û
	for(int i=0;i<(int)Str.size();)
	{
		if(Str[i]<=' ')
		{
			Str.erase(i,1);
			continue;
		}
		break;
	}
}

void CzString::TrimRight(string &Str)
{
	//É¾³ýÎ²²¿¿ØÖÆ×Ö·û
	for(int i=(int)Str.size()-1;i>=0;i--)
	{
		if(Str[i]<=' ')
		{
			Str.erase(i,1);
			continue;
		}
		break;
	}
}

void CzString::Trim(string &Str)
{
	TrimLeft(Str);
	TrimRight(Str);
}

int CzString::ToInt(const string &Str)
{
	string TempStr="";
	for(int i=0;i<(int)Str.size();i++)
		if(Str[i]>='0' && Str[i]<='9')
			TempStr+=Str[i];
	return atoi(TempStr.c_str());
}

double CzString::ToDouble(const string &Str)
{
	string TempStr="";
	for(int i=0;i<(int)Str.size();i++)
		if((Str[i]>='0' && Str[i]<='9') || Str[i]=='.')
			TempStr+=Str[i];

	return atof(TempStr.c_str());
}

string CzString::IntToStr(int a)
{
	char TempStr[1024];

	sprintf(TempStr,"%d",a);
	return string(TempStr);
}

string CzString::DoubleToStr(double a)
{
	char TempStr[1024];

	sprintf(TempStr,"%f",a);
	return string(TempStr);
}

string CzString::IntToBinStr(int a,int Digital)
{
	string TempStr="";
	while(a>0)
	{
		TempStr=IntToStr(a%2)+TempStr;
		a/=2;
	}

    //ÊÇ·ñÇ°Ãæ²¹0£¿
    if(Digital>0)
    {
        while((int)TempStr.size()<Digital)
        	TempStr=IntToStr(0)+TempStr;
    }

	return TempStr;
}

const string CzString::Split(const string &Src,char Del,string &Word)
{
	string Tmp=Src;
	basic_string <char>::size_type Pos;

	if((Pos=Tmp.find(Del))!=string::npos)
	{
		Word=Tmp.substr(0,Pos);
		Tmp=Tmp.substr(Pos+1,Tmp.size());
	}
	else
	{
		Word=Tmp;
		Tmp.clear();
	}

	return Tmp;
}

