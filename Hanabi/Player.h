#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <list>

using std::vector;
using std::cout;
using std::endl;

#ifndef PLAYER_H
#define PLAYER_H


class Player
{
public:
	Player();
	Player(const Player& p);
	void tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize);
	Event* ask();
protected:
	// players hand that holds every possible card
	std::map<int, std::list<int>> playerHand;
	// deck that holds every possible card
	std::map<int, std::list<int>> deck;
	// map that holds the tableu
	std::map<int, std::list<int>> tableu;

	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;

	bool first_clear;
};

Player::Player()
{
	// Start with 8 hints
	hints = 8;
	// Start with 3 fuses
	fuses = 3;
	// 50 cards to start the deck
	deckSize = 50;

	// fill vector with possible cards for each color
	// 3 - 1's, 2 - 2's, 2 - 3's, 2 - 4's, 1 - 5
	std::list<int> possiblecards;
	possiblecards.push_back(1);
	possiblecards.push_back(1);
	possiblecards.push_back(1);
	possiblecards.push_back(2);
	possiblecards.push_back(2);
	possiblecards.push_back(3);
	possiblecards.push_back(3);
	possiblecards.push_back(4);
	possiblecards.push_back(4);
	possiblecards.push_back(5);

	// fill the player's hand with all the possible cards
	// RED 0 BLUE 1 GREEN 2 YELLOW 3 WHITE 4
	playerHand[0] = possiblecards; //Position 0
	playerHand[1] = possiblecards; //Position 1
	playerHand[2] = possiblecards; //Position 2
	playerHand[3] = possiblecards; //Position 3
	playerHand[4] = possiblecards; //Position 4

	// fill the deck with every possible card
	deck[0] = possiblecards; //RED
	deck[1] = possiblecards; //BLUE
	deck[2] = possiblecards; //GREEN
	deck[3] = possiblecards; //YELLOW
	deck[4] = possiblecards; //WHITE

	first_clear = false;
}

Player::Player(const Player& p)
{
	hints = p.hints;
	fuses = p.fuses;
	oHand = p.oHand;
	deckSize = p.deckSize;

	playerHand = p.playerHand;
	deck = p.deck;
	tableu = p.tableu;
}

void Player::tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize)
{
	// This function will store the events and their results into the knowledge base
	
	// go through deck and remove each instance of possible card from the deck from other Player's card
	if (oHand.size() == 5 && !first_clear){
		for (int i = 0; i < oHand.size(); i++)
		{
			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(oHand[i].color).begin(), deck.at(oHand[i].color).end(), oHand[i].number);
			deck.at(oHand[i].color).erase(it);
		}
		first_clear = true; // make sure deck does not keep getting cleared
	}

	int currentAction = e->getAction();
	/*
	PLAY 11
	DISCARD 12
	COLOR_HINT 13
	DRAW 14
	NUMBER_HINT 15
	NO_OP 0
	*/
	if (currentAction == 11){
		PlayEvent* pe = (PlayEvent*) e;
		//only play card if legal
		if (pe->legal){
			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// remove all options from this position in the players hand-- will be replaced by draw
				playerHand.at(pe->c.color).clear();
			} else { // the other players hand needs to be updated
				oHand.at(pe->position).color = -1; // TODO: figure out a better way than sentinel value for no card in oHand
			}

			// push card onto the tableu when it is a valid move
			tableu.at(pe->c.color).push_back(pe->c.number); 

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(), pe->c.number);
			deck.at(pe->c.color).erase(it);

		} else { // if not legal, take away a fuse
			fuses--;
		}
	}
	else if(currentAction == 12){
		DiscardEvent* de = (DiscardEvent*) e;
		// depending on who it is, the card is removed from the hand and the deck
		if (de->wasItThisPlayer){
			// remove all options from this position in the players hand-- will be replaced by draw
			playerHand.at(de->c.color).clear();
		} else {
			oHand.at(de->position).color = -1; // TODO: figure out a better way than sentinel value for no card in oHand
		}

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(de->c.color).begin(), deck.at(de->c.color).end(), de->c.number);
		deck.at(de->c.color).erase(it);
	}
	else if (currentAction == 13){
		ColorHintEvent* ce = (ColorHintEvent*) e;
		// TODO:
	}
	else if (currentAction == 14){
		DrawEvent* dre = (DrawEvent*) e;
		// TODO:
	}
	else if (currentAction == 15){
		NumberHintEvent* ne = (NumberHintEvent*) e;
		// TODO:
	}
	else if (currentAction == 0){
		// TODO:
	}

	this->hints = hints;
	this->fuses = fuses;
	this->oHand = oHand;
	this->deckSize = deckSize;


	/*  Possible kinds of event:
		DiscardEvent - can be for us or other player
			c - the card discarded
			wasItThisPlayer - true if we discarded, false otherwise
			position - the index in hand of the discarded card (0 base)
		ColorHintEvent - always for other player
			indices - all indices of the chosen color
			color - the color in question
		NumberHintEvent - always for the other player
			indices - all indices of the chosen number
			color - the number in question
		PlayEvent - can be for us or other player
			position - the index in hand of the discarded card (0 base)
			c - the card played
			legal - whether the card was a legal play
			wasItThisPlayer - true if we played, false otherwise
	*/
}

Event* Player::ask()
{
	Event* foo = new Event();
	return foo;
	/* You must produce an event of the appropriate type. Not all member
	variables of a given event type need to be filled in; some will be
	ignored even if they are. Summary follows.
	Options:
	ColorHintEvent - you must declare a color; no other member variables
	necessary.
	NumberHintEvent - you must declare a number; no other member variables
	necessary.
	PlayEvent - you must declare the index to be played; no other member
	variables necessary.
	DiscardEvent - you must declare the index to be discarded; no other
	member variables necessary.
	*/
}
#endif