#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include "Player.h"
#include <vector>

using std::vector;

#ifndef PLAYER_AND_HAND_H
#define PLAYER_AND_HAND_H

struct PlayerAndHand
{
	PlayerAndHand();
	PlayerAndHand(const PlayerAndHand& f);
	PlayerAndHand(Player& play, vector<Card> h);
	Player p;
	vector<Card> hand;
};

PlayerAndHand::PlayerAndHand()
{
}

PlayerAndHand::PlayerAndHand(const PlayerAndHand& f)
{
	p = f.p;
	hand = f.hand;
}

PlayerAndHand::PlayerAndHand(Player& play, vector<Card> h): p(play)
{
	hand = h;
}

#endif