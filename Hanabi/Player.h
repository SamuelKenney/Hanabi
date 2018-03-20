#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

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

	bool contains(int in);
protected:
	// players hand that holds every possible card
	std::map<int, std::map<int, std::list<int>>> playerHand;
	// deck that holds every possible card
	std::map<int, std::list<int>> deck;
	// map that holds the tableau
	std::vector<int>tableau;
	// discard pile
	std::vector<Card> discardPile;

	std::vector<int> pastColorHintMoves;
	std::vector<int> pastNumberHintMoves;


	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;
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

	// fill the deck with every possible card
	deck[0] = possiblecards; //RED
	deck[1] = possiblecards; //BLUE
	deck[2] = possiblecards; //GREEN
	deck[3] = possiblecards; //YELLOW
	deck[4] = possiblecards; //WHITE

	// fill the player's hand with all the possible cards
	// RED 0 BLUE 1 GREEN 2 YELLOW 3 WHITE 4
	playerHand[0] = deck; //Position 0
	playerHand[1] = deck; //Position 1
	playerHand[2] = deck; //Position 2
	playerHand[3] = deck; //Position 3
	playerHand[4] = deck; //Position 4
}

Player::Player(const Player& p)
{
	hints = p.hints;
	fuses = p.fuses;
	oHand = p.oHand;
	deckSize = p.deckSize;

	playerHand = p.playerHand;
	deck = p.deck;
	tableau = p.tableau;
	discardPile = p.discardPile;
	pastColorHintMoves = p.pastColorHintMoves;
	pastNumberHintMoves = p.pastNumberHintMoves;
}

void Player::tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize)
{
	// ============================================================================================
	// ========This function will store the events and the results into the knowledge base=========
	// ============================================================================================

	// go through deck and remove each instance of possible card from the deck from other Player's card
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
				playerHand.at(pe->position) = deck; // cards that are left over in the deck, not all 50 cards
			}

			// push card onto the tableau when it is a valid move
			tableau = board;

			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			// need to update own hand to reflect that it is no longer an option
			for (int i = 0; i < playerHand.size(); i++)
			{
				// Removes the card from the map that holds the deck
				if (!playerHand.at(i).empty() && !playerHand.at(i).at(pe->c.color).empty()){
					// Removes the card from the map that holds the deck
					std::list<int>::iterator it = std::find(playerHand.at(i).at(pe->c.color).begin(), playerHand.at(i).at(pe->c.color).end(), pe->c.number);
					if (it != playerHand.at(i).at(pe->c.color).end())
						playerHand.at(i).at(pe->c.color).erase(it);		
				}	
			}

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(),pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);

		} else { // if not legal, take away a fuse
			// don't need to worry about the game ending, this is for my knowledge base
			// if it is not a legal move, needs to be discarded

			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// remove all options from this position in the players hand-- will be replaced by draw
				playerHand.at(pe->position) = deck;
			} 

			//add discarded card to the pile
			discardPile.push_back(pe->c);

			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			// need to update own hand to reflect that it is no longer an option
			for (int i = 0; i < playerHand.size(); i++)
			{
				if (!playerHand.at(i).empty() && !playerHand.at(i).at(pe->c.color).empty()){
					// Removes the card from the map that holds the deck
					std::list<int>::iterator it = std::find(playerHand.at(i).at(pe->c.color).begin(), playerHand.at(i).at(pe->c.color).end(), pe->c.number);
					if (it != playerHand.at(i).at(pe->c.color).end())
						playerHand.at(i).at(pe->c.color).erase(it);			
				}
			}

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(), pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);
		}
	}
	else if(currentAction == 12){
		DiscardEvent* de = (DiscardEvent*) e;
		// depending on who it is, the card is removed from the hand and the deck
		if (de->wasItThisPlayer){
			// remove all options from this position in the players hand-- will be replaced by draw
			playerHand.at(de->position).at(de->c.color).clear();
		} else {
			oHand.at(de->position).color = -1; // TODO: figure out a better way than sentinel value for no card in oHand
		}

		// add discarded card to the pile
		discardPile.push_back(de->c);

		// update the players hand based on what is discarded
		for (int i = 0; i < playerHand.size(); i++)
		{
			if (!playerHand.at(i).empty() && !playerHand.at(i).at(de->c.color).empty()){
				// Removes the card from the map that holds the deck
				std::list<int>::iterator it = std::find(playerHand.at(i).at(de->c.color).begin(), playerHand.at(i).at(de->c.color).end(), de->c.number);
				if (it != playerHand.at(i).at(de->c.color).end())
					playerHand.at(i).at(de->c.color).erase(it);		
			}
		}

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(de->c.number).begin(), deck.at(de->c.number).end(), de->c.color);
		// makes sure there are no out of bound errors
		if (it != deck.at(de->c.color).end())
			deck.at(de->c.color).erase(it);
	}
	else if (currentAction == 13){
		ColorHintEvent* ce = (ColorHintEvent*) e;
		for (int i = 0; i < ce->indices.size(); i++)
		{
			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			for (int j = 0; j < playerHand.at(i).size(); j++)
			{
				// for each color not in hint, remove from the map
				// grab the position of the index specified in indices at i
				if (j != ce->color)
					playerHand.at(ce->indices.at(i)).at(j).clear();
			}
		}
	}
	else if (currentAction == 14){
		// will always be for the other player since we can see it
		DrawEvent* dre = (DrawEvent*) e;

		// removes drawn card from the player's hand
		for (int i = 0; i < playerHand.size(); i++)
		{
			if (!playerHand.at(i).empty() && !playerHand.at(i).at(dre->drawnCard.color).empty()){
				// Removes the card from the map that holds the deck
				std::list<int>::iterator it = std::find(playerHand.at(i).at(dre->drawnCard.color).begin(), playerHand.at(i).at(dre->drawnCard.color).end(), dre->drawnCard.number);
				if (it != playerHand.at(i).at(dre->drawnCard.color).end())
					playerHand.at(i).at(dre->drawnCard.color).erase(it);		
			}
		}

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(dre->drawnCard.color).begin(), deck.at(dre->drawnCard.color).end(), dre->drawnCard.number);
		// makes sure there are no out of bound errors
		if (it != deck.at(dre->drawnCard.color).end())
			deck.at(dre->drawnCard.color).erase(it);
	}
	else if (currentAction == 15){
		NumberHintEvent* ne = (NumberHintEvent*) e;

		for (int i = 0; i < ne->indices.size(); i++)
		{
			// <0, 2, 4> are blue
			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			for (int j = 0; j < playerHand.at(i).size(); j++)
			{
				// grab the position of the index specified in indices at i
				playerHand.at(ne->indices.at(i)).at(j).clear();
				playerHand.at(ne->indices.at(i)).at(j).push_back(ne->number);
			}
		}
	}
	else if (currentAction == 0){
		// TODO: Probably nothing
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

	// ============================================================================================
	// ==========This function will figure out the next best move to be made by the player=========
	// ============================================================================================

	/*
	 What do I have in my knowledge base to make a decision?
		- Player's Hand: or what could be
		- Deck: Cards left in the deck
		- Tableau: cards played to win the game
		- Discard Pile: cards that have been discarded or poorly played

	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;
	*/

	// The following hold each of the instantiations of the Event class with all needed arguments
	// Copy and paste from here to each decision made
	/*
	int card = 0;
	ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), card);
	return colorEvent;

	int number = 1;
	NumberHintEvent* numberEvent = new NumberHintEvent(vector<int>(), number);
	return numberEvent;

	int index = 0;
	PlayEvent* playEvent = new PlayEvent(index);
	return playEvent;

	int indexDis = 0;
	DiscardEvent* discardEvent = new DiscardEvent(indexDis);
	return discardEvent;
	*/

	srand (time(NULL));
	int choice = rand()%10;

	/* 
	Three main moves
		1. Give a hint -- based on other player's hand

		2. Play a card -- based on what we know about our hand

		3. Discard a card -- based on what we know about our hand
	*/

	// if the probabilty of giving a hint is high -- if all fuses, 80% chance of giving hint
	if ( choice <= hints){
		// if lots of same color - give that
		vector <int> colors;
		colors.resize(5);
		for (int i = 0; i < oHand.size(); i++)
		{
			switch (oHand[i].color)
			{
			// figure out the most seen color
			case 0:
				colors[0]++;
				break;
			case 1:
				colors[1]++;
				break;
			case 2:
				colors[2]++;
				break;
			case 3:
				colors[3]++;
				break;
			case 4:
				colors[4]++;
				break;
			}
		}
		int colorHigh = 0, colorIn = 0;
		for (int i = 0; i < colors.size(); i++)
		{
			if (colors[i] > colorHigh){
				colorHigh = colors[i]; 
				colorIn = i; // sets the index to be whatever the color most seen is
			}
		}
		
		// if lots of same number - give that
		vector <int> numbers;
		numbers.resize(5);
		for (int i = 0; i < oHand.size(); i++)
		{
			switch (oHand[i].number)
			{
			// figure out the most seen number
			case 0:
				numbers[0]++;
				break;
			case 1:
				numbers[1]++;
				break;
			case 2:
				numbers[2]++;
				break;
			case 3:
				numbers[3]++;
				break;
			case 4:
				numbers[4]++;
				break;
			}
		}
		int numberHigh = 0, numberIn = 0;
		for (int i = 0; i < numbers.size(); i++)
		{
			if (numbers[i] > numberHigh){ // only keep track of last 5 hints
				numberHigh = numbers[i]; 
				numberIn = i; // sets the index to be whatever the color most seen is
			}
		}

		// returns an Event that has the most hints
		if (numberHigh > colorHigh){
			// keep track of last Number Hint
			if (pastNumberHintMoves.size() == 5){
				pastNumberHintMoves.pop_back();
			}
			pastNumberHintMoves.push_back(numberIn);
			
			NumberHintEvent* colorEvent = new NumberHintEvent(vector<int>(), numberIn);
			return colorEvent;
		} else {
			// keep track of last Color Hint
			if (pastColorHintMoves.size() == 5){ // only keep track of last 5 hints
				pastColorHintMoves.pop_back();
			}
			pastColorHintMoves.push_back(colorIn);
			ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), colorIn);
			return colorEvent;
		}
		
		// if a 1 of any color - give that
		

	}


	/*  You must produce an event of the appropriate type. Not all member
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

bool Player::contains(int in){
	for (int i = 0; i < pastColorHintMoves.size(); i++)
	{
		if (pastColorHintMoves[i] == in)
			return true;
	}
	return false;
}
#endif