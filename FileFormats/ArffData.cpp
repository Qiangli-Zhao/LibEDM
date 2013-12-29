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
using namespace std;
#include "Obj.h"
#include "zString.h"
#include "DateTime.h"
#include "RandSequence.h"
#include "DataSet.h"
#include "ArffData.h"
using namespace libep;

//reading information from an ARFF format file
void CArffData::ReadInfo(ifstream &InfoFile)
{
	//read class description
	int ValueNum=0;
	//number of attributes
	CaseInfo.ValidWidth=0;
	CaseInfo.ReadWidth=0;

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

		//read declaration from the line
		string Declaration;
		basic_istringstream<char> DataLine(Line);
		DataLine>>Declaration;
		//type
		if(Declaration=="@relation")
			continue;
		else if(Declaration=="@data")
			break;
		else if(Declaration!="@attribute")
			throw(CError("Information file: unexpected declaration!",306,0));


		//process the attributes and label
		//attribute name
		AttrStr Attr;
		Attr.MMSet=false;
		Attr.Max=Attr.Min=0;
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
		if(value=="string")
		{
			Attr.AttType=ATT_IGNORED;
			Attr.OtherPos=-1;
			CaseInfo.ReadAttrs.push_back(Attr);
			CaseInfo.ReadWidth++;
		}
		else if(value=="numeric" || value=="real")
		{
			Attr.AttType=ATT_CONTINUOUS;
			Attr.OtherPos=CaseInfo.ReadWidth;
			CaseInfo.ValidAttrs.push_back(Attr);
			Attr.OtherPos=CaseInfo.ValidWidth;
			CaseInfo.ReadAttrs.push_back(Attr);

			CaseInfo.ReadWidth++;
			CaseInfo.ValidWidth++;
		}
		else if(value=="date")
		{
			Attr.AttType=ATT_DATETIME;
			Attr.OtherPos=CaseInfo.ReadWidth;
			CaseInfo.ValidAttrs.push_back(Attr);
			Attr.OtherPos=CaseInfo.ValidWidth;
			CaseInfo.ReadAttrs.push_back(Attr);

			CaseInfo.ReadWidth++;
			CaseInfo.ValidWidth++;
		}
		else//discrete attribute and class labels
		{
			//note: class label is treated as a discrete attribute
			//it is temporarily used to show the position of label, will be removed after reading the data

			//number of value for discrete attribute
			ValueNum=0;
			DiscValueStr DiscValue;
			//read attribute name from the line
			do
			{
				//first char of the first value must be '{'
				if(ValueNum==0)
				{
					if(value[0]!='{')
					{
						basic_ostringstream<char> OutMsg;
						OutMsg<<"Information file: err in attribute description "<<CaseInfo.ReadWidth<<ends;
						throw(CError(string(OutMsg.str()),307,0));
					}

					value.erase(value.begin());
					CzString::Trim(value);
				}
				//last char of the last value is '}'
				bool IsLast=false;
				if(value[value.length()-1]=='}')
				{
					value.erase(value.end()-1);
					CzString::Trim(value);
					IsLast=true;
				}
				//save value
				DiscValue.Name=value;
				Attr.Disc.push_back(DiscValue);
				ValueNum++;

				//last?
				if(IsLast)
					break;
				//read next value
				DataLine>>value;
				//read failed
				if(DataLine.fail())
					break;
			}
			while(true);
			//found value descriptions?
			if(ValueNum<=0)
			{
				basic_ostringstream<char> OutMsg;
				OutMsg<<"Information file: no discrete-values description found "<<CaseInfo.ReadWidth<<ends;
				throw(CError(string(OutMsg.str()),306,0));
			}

			//discrete attribute
			Attr.AttType=ATT_DISCRETE;
			Attr.OtherPos=CaseInfo.ReadWidth;
			CaseInfo.ValidAttrs.push_back(Attr);
			Attr.OtherPos=CaseInfo.ValidWidth;
			CaseInfo.ReadAttrs.push_back(Attr);

			CaseInfo.ReadWidth++;
			CaseInfo.ValidWidth++;
		}//end of a discrete attribute (maybe the class label)
	}//end of file

	//the last discrete attribute is the label
	CaseInfo.ReadAttrs[CaseInfo.ReadWidth-1].AttType=ATT_CLASSLABEL;
	CaseInfo.ValidAttrs[CaseInfo.ValidWidth-1].AttType=ATT_CLASSLABEL;
	CaseInfo.Classes.assign(CaseInfo.ReadAttrs[CaseInfo.ReadWidth-1].Disc.begin(),CaseInfo.ReadAttrs[CaseInfo.ReadWidth-1].Disc.end());
	//class label number
	CaseInfo.ClassNum=(int)CaseInfo.Classes.size();
}

