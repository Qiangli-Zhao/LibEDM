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


#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <ctime>
using namespace std;
#include "Obj.h"
#include "zString.h"
#include "DateTime.h"
#include "RandSequence.h"
#include "DataSet.h"
#include "UCIData.h"
using namespace libep;

//reading information from an UCI information file
void CUCIData::ReadInfo(ifstream &InfoFile)
{
	//read class description
	int ValueNum=0;
	//is Label putted at the end of line?
	bool LabelAtEnd=true;
	while(!InfoFile.eof())
	{
		//read a line
		string Line;
		try
		{
			getline(InfoFile,Line);
		}
		catch(...)
		{
			//not enough buffer
			throw(CError("Information file: too long a line!",301,0));
		}
		if(InfoFile.fail())
		{
			if(InfoFile.eof())
				continue;
			throw(CError("Information file: read error!",302,0));
		}
		//format and parse the line
		if(FormatLine(Line)==SKIP_LINE)
			continue;
		//read data from the line
		basic_istringstream<char> DataLine(Line);
		DiscValueStr ClassDesc;
		while(!DataLine.eof())
		{
			//read a value
			DataLine>>ClassDesc.Name;
			//is Label putted at the end of line?
			if(ValueNum==0 && ClassDesc.Name=="-")
			{
				//Label is putted at the first non-ignored attribute
				LabelAtEnd=false;
				continue;
			}
			//read failed
			if(DataLine.fail())
				break;
			CaseInfo.Classes.push_back(ClassDesc);
			ValueNum++;
		}
		//found class description?
		if(ValueNum>0)
			break;
	}
	if(ValueNum<=0)
		throw(CError("Information file: error in class description!",303,0));
	CaseInfo.ClassNum=ValueNum;

	//number of attributes
	CaseInfo.ReadWidth=0;
	CaseInfo.ValidWidth=0;
	//read attributes description
	while(!InfoFile.eof())
	{
		string Line;
		try
		{
			getline(InfoFile,Line);
		}
		catch(...)
		{
			//not enough buffer
			throw(CError("Information file: too long a line!",304,0));
		}
		if(InfoFile.fail())
		{
			if(InfoFile.eof())
				continue;
			throw(CError("Information file: read error!",305,0));
		}
		//format and parse the line
		if(FormatLine(Line)==SKIP_LINE)
			continue;

		//read attribute name from the line
		AttrStr Attr;
		Attr.MMSet=false;
		Attr.Max=Attr.Min=0;
		basic_istringstream<char> DataLine(Line);
		DataLine>>Attr.Name;

		//read attribute type
		string value;
		DataLine>>value;
		if(DataLine.fail())
		{
			basic_ostringstream<char> OutMsg;
			OutMsg<<"Information file: err in attribute description "<<CaseInfo.ReadWidth<<ends;
			throw(CError(string(OutMsg.str()),305,0));
		}
		//read attribute data
		if(value=="ignore")
		{
			Attr.AttType=ATT_IGNORED;
			Attr.OtherPos=-1;
			CaseInfo.ReadAttrs.push_back(Attr);
			CaseInfo.ReadWidth++;
		}
		else if(value=="continuous")
		{
			Attr.AttType=ATT_CONTINUOUS;
			Attr.OtherPos=CaseInfo.ReadWidth;
			CaseInfo.ValidAttrs.push_back(Attr);
			Attr.OtherPos=CaseInfo.ValidWidth;
			CaseInfo.ReadAttrs.push_back(Attr);

			CaseInfo.ReadWidth++;
			CaseInfo.ValidWidth++;
		}
 		else
		{
			//number of value for discrete attribute
			ValueNum=0;
			DiscValueStr DiscValue;
			//read attribute name from the line
			do
			{
				DiscValue.Name=value;
				Attr.Disc.push_back(DiscValue);
				ValueNum++;
				//read next value
				DataLine>>value;
				//read failed
				if(DataLine.fail())
					break;
			}
			while(true);
			//found value description?
			if(ValueNum<=0)
			{
				basic_ostringstream<char> OutMsg;
				OutMsg<<"Information file: no discrete-values description found "<<CaseInfo.ReadWidth+1<<ends;
				throw(CError(string(OutMsg.str()),306,0));
			}

			Attr.AttType=ATT_DISCRETE;
			Attr.OtherPos=CaseInfo.ReadWidth;
			CaseInfo.ValidAttrs.push_back(Attr);
			Attr.OtherPos=CaseInfo.ValidWidth;
			CaseInfo.ReadAttrs.push_back(Attr);

			CaseInfo.ReadWidth++;
			CaseInfo.ValidWidth++;
		}
	}

	//position of label
	AttrStr Attr;
	Attr.AttType=ATT_CLASSLABEL;
	Attr.Name="label";
	Attr.Disc.assign(CaseInfo.Classes.begin(),CaseInfo.Classes.end());
	Attr.MMSet=true;
	Attr.Min=0;
	Attr.Max=CaseInfo.ClassNum-1;
	//the position of the label in a line (head or tail)
	if(LabelAtEnd)
	{
		CaseInfo.ReadAttrs.push_back(Attr);
		CaseInfo.ValidAttrs.push_back(Attr);
	}
	else//label is at the 1st column
	{
		CaseInfo.ReadAttrs.insert(CaseInfo.ReadAttrs.begin(),Attr);
		CaseInfo.ValidAttrs.insert(CaseInfo.ValidAttrs.begin(),Attr);
	}
	CaseInfo.ReadWidth++;
	CaseInfo.ValidWidth++;
}

