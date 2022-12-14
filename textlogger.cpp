//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Textlogger
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "definitions.h"

#ifndef __CONSOLE__
#include "textlogger.h"
#include "gui.h"
#include "tools.h"

TextLogger::TextLogger()
{
	out = std::cerr.rdbuf();
	err = std::cout.rdbuf();
	displayDate = true;
}

TextLogger::~TextLogger()
{
	std::cerr.rdbuf(err);
	std::cout.rdbuf(out);
}

int32_t TextLogger::overflow(int32_t c)
{
	#ifdef __GUI_LOGS__
	char buf[21], buffer[85], date[21];
	#else
	char date[21];
	#endif
	formatDate(time(NULL), date);
	#ifdef __GUI_LOGS__
	formatDate2(time(NULL), buf);
	sprintf(buffer, "%s%s.log", getFilePath(FILE_TYPE_LOG, "server/").c_str(), buf);

	if(FILE* file = fopen(buffer, "a"))
	{
		if(displayDate)
			fprintf(file, "[%s] ", date);

		fprintf(file, "%c", c);
		fclose(file);
	}
	#endif

	if(c == '\n')
	{
		GUI::getInstance()->m_logText += "\r\n";
		SendMessage(GetDlgItem(GUI::getInstance()->m_mainWindow, ID_LOG), WM_SETTEXT, 0, (LPARAM)GUI::getInstance()->m_logText.c_str());
		GUI::getInstance()->m_lineCount++;
		SendMessage(GUI::getInstance()->m_logWindow, EM_LINESCROLL, 0, GUI::getInstance()->m_lineCount);
		displayDate = true;
	}
	else
	{
		if(displayDate)
		{
			GUI::getInstance()->m_logText += "[";
			GUI::getInstance()->m_logText += date;
			GUI::getInstance()->m_logText += "] ";
			displayDate = false;
		}
		GUI::getInstance()->m_logText += (char)c;
	}

	return c;
}
#endif
