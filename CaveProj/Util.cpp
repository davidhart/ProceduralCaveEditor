#include "Util.h"

bool Util::HexCharToInt(char c, int& i)
{
	if (c >= '0' && c <= '9')
	{
		i = c - '0';
		return true;
	}
	if (c >= 'a' && c <= 'f')
	{
		i = c - 'a' + 10;
		return true;
	}
	if (c >= 'A' && c <= 'F')
	{
		i = c - 'A' + 10;
		return true;
	}

	return false;
}

bool Util::ReadHexColor(std::istream& stream, DWORD& color)
{
	int r = 0, g = 0, b = 0;

	int tempi;
	char tempc = '\0';

	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	r += tempi * 16;
	
	tempc = '\0';
	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	r += tempi;

	tempc = '\0';
	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	g += tempi * 16;
	
	tempc = '\0';
	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	g += tempi;

	tempc = '\0';
	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	b += tempi * 16;
	
	tempc = '\0';
	stream >> tempc;
	if (!HexCharToInt(tempc, tempi)) return false;
	b += tempi;

	color = COLOR_ARGB(255, r, g, b);
	return true;
}