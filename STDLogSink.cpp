/***********************************************************************************************************************
 * Copyright (C) 2016 Andrew Zonenberg and contributors                                                                *
 *                                                                                                                     *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the    *
 * following conditions are met:                                                                                       *
 *                                                                                                                     *
 *    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the        *
 *      following disclaimer.                                                                                          *
 *                                                                                                                     *
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the      *
 *      following disclaimer in the documentation and/or other materials provided with the distribution.               *
 *                                                                                                                     *
 *    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products    *
 *      derived from this software without specific prior written permission.                                          *
 *                                                                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL*
 * THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES       *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR      *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT*
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE      *
 * POSSIBILITY OF SUCH DAMAGE.                                                                                         *
 *                                                                                                                     *
 **********************************************************************************************************************/

#include "log.h"
#include <cstdio>
#include <cstdarg>
#include <string>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

STDLogSink::STDLogSink(Severity min_severity)
	: m_min_severity(min_severity)
{
	//TODO: Get it via escape sequences or something?
	m_termWidth = 80;

	//For now, get the actual terminal width on Linux via stty
#ifdef __linux__
	FILE* fp = popen("stty size", "r");
	if(fp == NULL)
		return;
	unsigned int height;
	fscanf(fp, "%u %u", &height, &m_termWidth);
	pclose(fp);
#endif

}

STDLogSink::~STDLogSink()
{
	fflush(stdout);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String formatting

/**
	@brief Wraps long lines and adds indentation as needed
 */
string STDLogSink::WrapString(string str)
{
	string ret = "";

	//Cache the indent string so we don't have to re-generate it each time
	string indent = GetIndentString();

	//Split the string into lines
	string tmp = indent;
	for(size_t i=0; i<str.length(); i++)
	{
		//Append it
		char ch = str[i];
		tmp += ch;

		//If the pending line is longer than m_termWidth, break it up
		if(tmp.length() == m_termWidth)
		{
			ret += tmp;
			ret += "\n";
			tmp = indent;
		}

		//If we hit a newline, wrap and indent the next line
		if(ch == '\n')
		{
			ret += tmp;
			tmp = indent;
		}
	}

	//If we have any remaining stuff, append it
	if(tmp != indent)
		ret += tmp;

	//Done
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging

void STDLogSink::Log(Severity severity, const string &msg)
{
	//Skip messages which aren't important enough
	if(severity > m_min_severity)
		return;

	//Prevent newer messages on stderr from appearing before older messages on stdout
	if(severity <= Severity::WARNING)
		fflush(stdout);

	//Wrap the string and re-indent as needed
	string wrapped = WrapString(msg);
	fputs(wrapped.c_str(), stderr);

	//Ensure that this message is displayed immediately even if we print lower severity stuff later
	if(severity <= Severity::WARNING)
		fflush(stderr);
}

void STDLogSink::Log(Severity severity, const char *format, va_list va)
{
	//Skip messages which aren't important enough
	if(severity > m_min_severity)
		return;

	//Prevent newer messages on stderr from appearing before older messages on stdout
	if(severity <= Severity::WARNING)
		fflush(stdout);

	//Convert to an in-memory buffer we can do wrapping on
	//TODO: handle truncation if buffer is too small
	char buf[2048];
	vsnprintf(buf, sizeof(buf), format, va);
	string msg(buf);

	//Wrap the string and re-indent as needed
	string wrapped = WrapString(msg);
	fputs(wrapped.c_str(), stderr);

	//Ensure that this message is displayed immediately even if we print lower severity stuff later
	if(severity <= Severity::WARNING)
		fflush(stderr);
}
