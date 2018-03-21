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

	bool lastCard(Card c);

	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;

	vector<std::pair<bool, bool>> hintedAt;
	vector<std::pair<bool, bool>> playerHintedAtStored;

	bool canBePlayed(Card c);
	int chooseDiscard(bool uncertain = false);
	int chooseOpponentDiscard();
	bool numberCanBePlayed(int n);
	void playerHintedAt();
	void otherPlayerHintedAt();

	int getCardColor(int index);
	int getCardNumber(int index);

	int cardPlay();

	int turns;

	void print();
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

													// color, number
	std::pair<bool, bool> pos0 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos1 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos2 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos3 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool>pos4 = std::pair<bool, bool>(false, false);

	hintedAt.push_back(pos0);
	hintedAt.push_back(pos1);
	hintedAt.push_back(pos2);
	hintedAt.push_back(pos3);
	hintedAt.push_back(pos4);

	playerHintedAtStored.resize(5);

	turns = 0;
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

	hintedAt = p.hintedAt;
	turns = p.turns;
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
		std::pair<bool, bool> foo = std::pair<bool, bool>(false, false);
		if (pe->legal){
			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// remove all options from this position in the players hand-- will be replaced by draw

				// removes the card from the play, and shifts everything down
				std::map<int,std::map<int, std::list<int>>> temp;
				int count = 0;
				for (int i = 0; i < playerHand.size(); i++)
				{
					if (i != pe->position){
						temp[count] = playerHand[i];
					} else {
						count--;
					}
					count++;
				}
				temp[4] = deck;
				playerHand = temp;

				//playerHintedAt();
				playerHintedAtStored.erase(playerHintedAtStored.begin() + pe->position);
				playerHintedAtStored.push_back(foo);
			} else {
				hintedAt.erase(hintedAt.begin() + pe->position);
				hintedAt.push_back(foo);
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
				// removes the card from the play, and shifts everything down
				std::map<int,std::map<int, std::list<int>>> temp;
				int count = 0;
				for (int i = 0; i < playerHand.size(); i++)
				{
					if (i != pe->position){
						temp[count] = playerHand[i];
					} else {
						count--;
					}
					count++;
				}
				temp[4] = deck;
				playerHand = temp;
				
				playerHintedAtStored.erase(playerHintedAtStored.begin() + pe->position);
				playerHintedAtStored.push_back(foo);
			} else {
				hintedAt.erase(hintedAt.begin() + pe->position);
				hintedAt.push_back(foo);
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
		std::pair<bool, bool> foo = std::pair<bool, bool>(false, false);
		if (de->wasItThisPlayer){
			// removes the card from the play, and shifts everything down
			std::map<int,std::map<int, std::list<int>>> temp;
			int count = 0;
			for (int i = 0; i < playerHand.size(); i++)
			{
				if (i != de->position){
					temp[count] = playerHand[i];
				} else {
					count--;
				}
				count++;
			}
			temp[4] = deck;
			playerHand = temp;

			//playerHintedAt();
			playerHintedAtStored.erase(playerHintedAtStored.begin() + de->position);
			playerHintedAtStored.push_back(foo);
		} else {
			hintedAt.erase(hintedAt.begin() + de->position);
			hintedAt.push_back(foo);
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
		std::list<int>::iterator it = std::find(deck.at(de->c.color).begin(), deck.at(de->c.color).end(), de->c.number);
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
				if (j != ce->color){
					playerHand.at(ce->indices.at(i)).at(j).clear();
				}
			}
			playerHintedAtStored[ce->indices.at(i)].first = true; // hand at this position has been hinted a color
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
				if (it != playerHand.at(i).at(dre->drawnCard.color).end()){
					playerHand.at(i).at(dre->drawnCard.color).erase(it);
				}
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
		std::list<int> choosen;
		if (ne->number == 1){ // if 1, push back three
			for (int i = 0; i < 3; i++)
			{
				choosen.push_back(ne->number);
			}
		}
		else if (ne->number == 5){ // if 5
			choosen.push_back(ne->number);
		} else { // if 2, 3, or 4 push back two
			choosen.push_back(ne->number);
			choosen.push_back(ne->number);
		}



		for (int i = 0; i < ne->indices.size(); i++)
		{
			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			for (int j = 0; j < playerHand.at(i).size(); j++)
			{
				// grab the position of the index specified in indices at i
				playerHand.at(ne->indices.at(i)).at(j) = choosen; // set the new vector there to be the possible options
			}
			playerHintedAtStored[ne->indices.at(i)].second = true; // hand at this position has been hinted a number
		}
	}
	else if (currentAction == 0){
		// TODO: Probably nothing
	}

	this->tableau = board;
	this->hints = hints;
	this->fuses = fuses;
	this->oHand = oHand;
	this->deckSize = deckSize;
}

Event* Player::ask()
{

	// ============================================================================================
	// ==========This function will figure out the next best move to be made by the player=========
	// ============================================================================================

	print();
	// needs to be called every time to update values based on hints
	//playerHintedAt();
	
	turns++;

	bool hint = true;
	//1 Save Hint
	if (turns >= 2){ // wait till second stage of game TODO:
		int in = chooseOpponentDiscard();
		// TODO: DO WE HAVE ANY HINTS TO SPEND!!!
		if (in != -1){
			if (!hintedAt[in].first && !hintedAt[in].second){ // if both false, high chance of discard
				// TODO: PLAYED TOO MANY LAST CARDS
				if (lastCard(oHand[in])){ // if this is the last card, give a color hint
					ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[in].color);
					for (int j = 0; j < oHand.size(); j++)
					{
						if (oHand[j].color == oHand[in].color)
							hintedAt[j].first = true;
					}
					return colorEvent;
				}
			}
		}
	}

	if (hints < 8) {
		int c = chooseDiscard();
		if (c != -1){ // if there is a good guaranteed card to get rid of, do it
			DiscardEvent* discardEvent = new DiscardEvent(c);
			return discardEvent;
		}
	}

	//2 Good play
	int c = cardPlay();
	if (c != -1 ){ // if its a real card
		PlayEvent* playEvent = new PlayEvent(c); // if a real card that can be played, play it
		return playEvent;
	}
	/* Looking at every card in MY hand */
	// If fully known card, discard (if hints < 8) or play accordingly

	for (int i = 0; i < hintedAt.size(); i++) // TODO: HINTS NEED TO NOT BE 0!!!!
	{
		if (hintedAt[i].second == true){
			ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[i].color);

			for (int j = 0; j < oHand.size(); j++)
			{
				if (oHand[j].color == oHand[i].number)
					hintedAt[j].first = true;
			}
			return colorEvent;
		}

	}

	//3 Uncertain play
		// If number known & playable, play it
		// Perhaps play the rarest card possible? eg play a 5 over a 3
		// If only color known, skip it
	for (int i = 0; i < playerHand.size(); i++)
	{
		int num = getCardNumber(i);
		if (numberCanBePlayed(num)){
			PlayEvent* playEvent = new PlayEvent(i); // if a real card that can be played, play it
			return playEvent;
		}
	}

	//4 Play Hint
		/* Looking at every PLAYABLE card in THEIR hand */
		// Hint at the number of the card in the smallest group
			// Eg if there is a playable 1 and 2, but there are more 1's than 2's, hint the 2
	for (int i = 0; i < oHand.size(); i++)
	{
		// TODO: whittle down options
		if (canBePlayed(oHand[i])){
			if (hintedAt[i].second != true){
				NumberHintEvent* numberEvent = new NumberHintEvent(vector<int>(), oHand[i].number);
				for (int j = 0; j < oHand.size(); j++)
				{
					if (oHand[j].number == oHand[i].number)
						hintedAt[j].second = true;
				}
				return numberEvent;
			}
		}
	}

	if (hints == 8){
		for (int i = 0; i < hintedAt.size(); i++)
		{
			if (hintedAt[i].second == true){
				ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[i].color);

				for (int j = 0; j < oHand.size(); j++)
				{
					if (oHand[j].color == oHand[i].color)
						hintedAt[j].first = true;
				}
				return colorEvent;
			}

		}
	} else {
		//5 Discard
		DiscardEvent* discardEvent = new DiscardEvent(chooseDiscard(true));
		return discardEvent;
	}
	
	

	
}

bool Player::canBePlayed(Card c){
	int val = c.number - 1;
	// goes to specific spot in tableau and if the played card is one lower than possible played card, return true
	if (tableau[c.color] == val)
		return true;
		
	return false;
}
bool Player::numberCanBePlayed(int n){
	// goes through tableau and if there is a value anywhere that the number can be played on, it returns true
	int val = n - 1;
	for (int i = 0; i < tableau.size(); i++)
	{
		if (tableau[i] == val)
			return true;
	}

	return false;
}

/// Returns a card that can be played, if no cards playable, return -1 for color and number
int Player::cardPlay(){
	int in = -1;
	Card c = Card(-1, -1); 
	for (int i = 0; i < playerHand.size(); i++)
	{
		int colr = getCardColor(i);
		if (colr != -1){
			int num = getCardNumber(i);
			if (num != -1) {
				c.color = colr;
				c.number = num;
				if (canBePlayed(c)){
					return i;
				} else {
					in = -1;
				}
			}
		}
	}

	return in;
}

bool Player::lastCard(Card c){
	int counter = 0;
	if (c.number == 5){
		return true;
	}

	std::list<int>::iterator it = std::find(deck.at(c.color).begin(), deck.at(c.color).end(), c.number);
	if (it == deck.at(c.color).end()){
		return true;
	}
	return false;

}

/// Returns the index of the best card to discard
///	if uncertain = true than 
int Player::chooseDiscard(bool uncertain) {
	// This case is assuming we have already discarded any obvious discards
	/* Looking a the cards in order of least information known first */
	
	// Any known safe discard
	//playerHintedAt();
	for (int i = 0; i < playerHintedAtStored.size(); i++)
	{
		int color = getCardColor(i);
		int number = getCardNumber(i);

		// if we know the card and it can't be played
		// TODO: Consider if the card is numbered, but we cannot play it
		if (playerHintedAtStored[i].first && playerHintedAtStored[i].second && (tableau[i] >= getCardNumber(i)))
		{
			return i;
		}

		// 1. Any color xor number we know to be 'used up' 
		if (color != -1 && tableau[color] == 5){
			return i; // discard this card
		}

		int min = 5;
		for (int j = 0; j < tableau.size(); j++)
		{
			if (tableau[j] < min){
				min = tableau[j];
			}
		}
		if (number != -1 && number <= min){
			return i;
		}
	}
	// Uncertain discards
	if (uncertain) {
		int in = -1;
		for (int i = 0; i < playerHintedAtStored.size(); i++)
		{
			// 2. Unknown
			if (!playerHintedAtStored[i].first && !playerHintedAtStored[i].second){
				return i;
			} 
		}
		for (int i = 0; i < playerHintedAtStored.size(); i++)
		{
			// 3. Color Known
			if (playerHintedAtStored[i].first && !playerHintedAtStored[i].second){
				return i;
			} 
		}
		return 0;
	}
	return -1;
}

int Player::chooseOpponentDiscard(){
	for (int i = 0; i < hintedAt.size(); i++)
	{
		int color = getCardColor(i);
		int number = getCardNumber(i);

		// if we know the card and it can't be played
		// TODO: Consider if the card is numbered, but we cannot play it
		if (hintedAt[i].first && hintedAt[i].second && (tableau[i] >= oHand[i].number))
		{
			return i;
		}

		// 1. Any color xor number we know to be 'used up' 
		if (color != -1 && tableau[color] == 5){
			return i; // discard this card
		}

		int min = 5;
		for (int j = 0; j < tableau.size(); j++)
		{
			if (tableau[j] < min){
				min = tableau[j];
			}
		}
		if (number != -1 && number <= min){
			return i;
		}
	}

	for (int i = 0; i < playerHintedAtStored.size(); i++)
	{
		// 2. Unknown
		if (!playerHintedAtStored[i].first && !playerHintedAtStored[i].second){
			return i;
		} 
	}
	for (int i = 0; i < playerHintedAtStored.size(); i++)
	{
		// 3. Color Known
		if (playerHintedAtStored[i].first && !playerHintedAtStored[i].second){
			return i;
		} 
	}
	return -1;
}

/// Returns the color of the card if it is definite, returns -1 otherwise
int Player::getCardColor(int card) {
	// Sentinel value
	int color = -1;
	int cols = 0;

	// check if the color is known
	for (int j = 0; j < playerHand.at(card).size(); j++)
	{
		if (playerHand.at(card).at(j).empty()) {
			cols++;
		}
		else {
			color = j;
		}
	}
	if (cols == 4) { // color has been hinted
		return color;
	} else {
		return -1;
	}
}

/// Returns the number of the card if it is definite, returns -1 otherwise
int Player::getCardNumber(int card) {
	// Sentinel value
	int num = -1;

	// TODO: not used right now
	//for (int i = 0; i < playerHand.at(card).size(); i++)
	//{
	//	for (auto v : playerHand.at(card).at(i))
	//	{
	//		if (numberCanBePlayed(v))
	//			return v;
	//	}
	//}

	for (int i = 0; i < playerHand.at(card).size(); i++){
		bool same = true;
		std::list<int> test = playerHand.at(card).at(i);

		if (!test.empty()){
			int prev = test.front();
			for (auto v :test)
			{
				if (v != prev)
					same = false;
			}
			if (same){
				return prev;
			}
		}
	}

	return -1;
}



void Player::playerHintedAt(){
	// updates vector of pairs that let the player know what parts of their hand have been hinted at
	// Possible values are of the form (color, number) which could be FF, FT, TF, or TT

	int cols = 0;
	int nums = 0;

	bool colorHinted = false;
	bool numberHinted = false;

	for (int i = 0; i < playerHand.size(); i++)
	{
		cols = 0;
		nums = 0;
		colorHinted = false;
	    numberHinted = false;

		// check if the color is known
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).empty()){
				cols++;
			}
		}
		if (cols == 4){ // color has been hinted
			colorHinted = true;
		}
		// check if the color is known
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).size() == 1){
				nums++;
			}
		}
		if (nums == 1 || nums == 5){
			numberHinted = true;
		}

		// keep track of each hand whether that be FF, TF, FT, or TT
		std::pair<bool, bool> temp = std::pair<bool, bool>(colorHinted, numberHinted);
		playerHintedAtStored[i] = temp;
	}
}

void Player::print(){
	std::cout << "Other hints: \n";
	for (int i = 0; i < hintedAt.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: ";
			break;
		case 1:
			std::cout << "Second Card: ";
			break;
		case 2:
			std::cout << "Third Card: ";
			break;
		case 3:
			std::cout << "Fourth Card: ";
			break;
		case 4:
			std::cout << "Fifth Card: ";
			break;
		}
		switch(hintedAt[i].first){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}

		switch(hintedAt[i].second){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}
	}
	std::cout << "\nOur hints: " << std::endl;
	for (int i = 0; i < playerHintedAtStored.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: ";
			break;
		case 1:
			std::cout << "Second Card: ";
			break;
		case 2:
			std::cout << "Third Card: ";
			break;
		case 3:
			std::cout << "Fourth Card: ";
			break;
		case 4:
			std::cout << "Fifth Card: ";
			break;
		}
		switch(playerHintedAtStored[i].first){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}

		switch(playerHintedAtStored[i].second){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}
	}
	std::cout << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < playerHand.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: \n";
			break;
		case 1:
			std::cout << "Second Card: \n";
			break;
		case 2:
			std::cout << "Third Card: \n";
			break;
		case 3:
			std::cout << "Fourth Card: \n";
			break;
		case 4:
			std::cout << "Fifth Card: \n";
			break;
		}
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			switch(j){
			case 0:
				std::cout << "Red: ";
				break;
			case 1:
				std::cout << "Blue: ";
				break;
			case 2:
				std::cout << "Green: ";
				break;
			case 3:
				std::cout << "Yellow: ";
				break;
			case 4:
				std::cout << "White: ";
				break;
			}
			for (auto v : playerHand.at(i).at(j))
			{
				std::cout << v << " ";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void otherPlayerHintedAt(){

}
#endif