#include "GameConstants.h"
#include <string>

using std::string;
using std::to_string;

#ifndef CARD_H
#define CARD_H

class Card
{
public:
	Card();
	Card(const Card& c);
	Card(int c, int n);
	string toString() const;
	int color;
	int number;
	bool operator==(const Card& c) const;
	bool operator!=(const Card& c) const;
};

Card::Card()
{
}

Card::Card(const Card& c): color(c.color), number(c.number)
{
}

Card::Card(int c, int n): color(c), number(n)
{
}


string Card::toString() const
{
	string result;
	switch (color)
	{
	case RED:
		result = "Red ";
		break;
	case WHITE:
		result = "White ";
		break;
	case BLUE:
		result = "Blue ";
		break;
	case GREEN:
		result = "Green ";
		break;
	case YELLOW:
		result = "Yellow ";
		break;
	default:
		break;
	}

	result += to_string(static_cast<long long>(number));
	return result;
}

bool Card::operator==(const Card& c) const
{
	return ((number == c.number) && (color == c.color));
}

bool Card::operator!=(const Card& c) const
{
	return ((number != c.number) || (color != c.color));
}

#endif