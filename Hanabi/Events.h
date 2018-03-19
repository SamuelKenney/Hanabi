#include "GameConstants.h"
#include "Card.h"
#include <vector>

using std::vector;

#ifndef EVENTS_H
#define EVENTS_H

class Event
{
public:
	Event();
	Event(const Event& e);
	int getAction();
protected:
	int action;
};

Event::Event()
{
	action = NO_OP;
}

Event::Event(const Event& e)
{
	action = e.action;
}

int Event::getAction()
{
	return action;
}

class DrawEvent: public Event
{
public:
	DrawEvent();
	DrawEvent(int pos, Card c);
	int position;
	Card drawnCard;
};

DrawEvent::DrawEvent()
{
	action = DRAW;
}

DrawEvent::DrawEvent(int pos, Card c): position(pos), drawnCard(c)
{
	action = DRAW;
}

class DiscardEvent: public Event
{
public:
	DiscardEvent();
	DiscardEvent(int pos);
	int position;
	Card c;
	bool wasItThisPlayer;
};

DiscardEvent::DiscardEvent()
{
	action = DISCARD;
}

DiscardEvent::DiscardEvent(int pos): position(pos)
{
	action = DISCARD;
}


class ColorHintEvent: public Event
{
public:
	ColorHintEvent();
	ColorHintEvent(vector<int> i, int color);
	vector<int> indices;
	int color;
};

ColorHintEvent::ColorHintEvent()
{
	action = COLOR_HINT;
}

ColorHintEvent::ColorHintEvent(vector<int> i, int hintColor): indices(i), color(hintColor)
{
	action = COLOR_HINT;
}

class NumberHintEvent: public Event
{
public:
	NumberHintEvent();
	NumberHintEvent(vector<int> i, int hintNumber);
	vector<int> indices;
	int number;
};

NumberHintEvent::NumberHintEvent()
{
	action = NUMBER_HINT;
}

NumberHintEvent::NumberHintEvent(vector<int> i, int hintNumber): indices(i), number(hintNumber)
{
	action = NUMBER_HINT;
}

class PlayEvent: public Event
{
public:
	PlayEvent();
	PlayEvent(int pos);
	int position;
	Card c;
	bool legal;
	bool wasItThisPlayer;
};

PlayEvent::PlayEvent()
{
	action = PLAY;
}

PlayEvent::PlayEvent(int pos): position(pos)
{
	action = PLAY;
}

#endif