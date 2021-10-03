#include <algorithm>
#include <bits/types/wint_t.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <linux/limits.h>
#include <sys/select.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdlib>
#include <cwchar>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <ncurses.h>
#include <limits.h>
#include <iostream>
#include <chrono>

#define ctrl(x) (x & 0x1F)

using std::string;
using std::vector;
using std::ios;
using std::fstream;
using std::cerr;
using std::endl;

/**
 * Stores all vocabulary and their amount
 */
struct VocInfo {
	int vocNum;
	vector<string> en;
	vector<string> ja;
	vector<string> furi;
};

const string opt1 = "Japanese -> English";
const string opt2 = "English ->  Japanese";
const string opt3 = "Japanese <-> English";
const string opt4 = "Dictionaries";
const string opt5 = "Exit";

int corUTrans = 0; // number of correct user translations

/**
 * Saves all vocs and their amount inside a struct
 *
 * @param dict Name of the dictionary file where all vocs are stored
 * @return Struct containing all vocs and how many there are	
 */
VocInfo getVocs(string dict) {
	fstream dictFile (dict, ios::in);
	VocInfo Vocs;
	string line;
	if (!dictFile) throw "File \""+dict+"\" not found.";
	while (!dictFile.eof()) {
		getline(dictFile,line);
		if (dictFile.eof()) break;
		while ( (line.empty()) && (!dictFile.eof()) ) getline(dictFile,line);
		Vocs.en.push_back(line);
		getline(dictFile,line);
		Vocs.ja.push_back(line);
		getline(dictFile,line);
		Vocs.furi.push_back(line);
	}
	dictFile.close();
	Vocs.vocNum = Vocs.en.size();
	return Vocs;
} 

/**
 * Puts each translation of one word into a single element of a vector
 *
 * @param trans	String containing the translation
 * @return Vector containing all translations as single elements
 */
vector<string> partitionAllTrans(string trans) {
	const char delim = ';';
	vector<string> allTrans;	
	while (trans.find_first_of(delim) != -1) {
		allTrans.push_back(trans.substr( 0,trans.find_first_of(delim) ));
		trans.erase(0,trans.find_first_of(delim)+1);
	}
	allTrans.push_back(trans);
	return allTrans;
}

/**
 * Checks if user given translations are a subset of the accepted translations.
 *
 * @param uTrans User translations separated by semicolons
 * @param trans Correct translations from dictionary file 
 * @return True if at least one user trans fits the dictionary file translations
 */
bool isSubSet(string uTrans, string trans) {
	vector<string> vecUTrans = partitionAllTrans(uTrans);
	vector<string> vecTrans = partitionAllTrans(trans);
	int vocsRight = 0;
	for (int i=0; i<vecUTrans.size(); ++i) {
		for (int j=0; j<vecTrans.size();++j) {
			std::transform(vecUTrans[i].begin(),vecUTrans[i].end(),vecUTrans[i].begin(),::tolower); // to lower case
			std::transform(vecTrans[j].begin(),vecTrans[j].end(),vecTrans[j].begin(),::tolower); // to lower case
			if (vecUTrans[i] == vecTrans[j]) {
				++vocsRight;
				break;
			}
		}	
	}
	if (vocsRight == vecUTrans.size()) return true;
	else return false;
}

/**
 * Gets remaining translations that user did not know
 *
 * @param uTrans Translations entered by the user
 * @param trans Correct translations from dictionary file
 * @ return All remaining correct translations
 */
vector<string> getRemTrans(string uTrans, string trans) {
	vector<string> vecUTrans = partitionAllTrans(uTrans);
	vector<string> vecTrans = partitionAllTrans(trans);
	vector<string> vecTransOgCase = vecTrans; /* dictionary translations vector with original case */
	vector<string> remTrans = vecTrans; /* remaining translations the user did not know */

	if (isSubSet(uTrans, trans)) {
		for (int i=0; i<vecUTrans.size();++i) {
			for (int j=0; j<=vecTrans.size();++j) {
				std::transform(vecUTrans[i].begin(),vecUTrans[i].end(),vecUTrans[i].begin(),::tolower); // to lower case
				std::transform(vecTrans[j].begin(),vecTrans[j].end(),vecTrans[j].begin(),::tolower); // to lower case
				if (vecUTrans[i] == vecTrans[j]) {
					remTrans.erase(remTrans.begin()+j);
					vecTrans.erase(vecTrans.begin()+j);
					break;
				}
			}	
			if (vecTrans.size() == 0) break;
		}
	}
	return remTrans;
}

/**
 * Creates a user input box around a given window
 *
 * @param winName The window where the user input box is positioned
 */
void mkInputBox(WINDOW * winName) {
	int left,right,top,bottom,tlc,trc,blc,brc;
	top = left = right = tlc = trc = blc = brc = 23; //space
	bottom = 0;
	wborder(winName, left, right, top, bottom, tlc, trc, blc, brc);
	wrefresh(winName);
}

void mkRisingPoints(){
	noecho(); curs_set(false);	

	string point = "1";
	int y = 30;

	WINDOW * xp = newwin(10, 1, 20, 40);
	refresh();

	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	wattron(xp,COLOR_PAIR(2));
	while (y>21) {
		--y;
		wmove(xp, y, 1);
		wprintw(xp,point.c_str());
		wrefresh(xp);
		wmove(xp, y, 0);
		usleep(1E4);
		wclrtoeol(xp);
	}
	wattroff(xp,COLOR_PAIR(2));
	wclear(xp);
}

/**
 * Queries the user for a single japanese to english translation
 *
 * @param queries Window containing the japanese vocabulary that is to be translated by the user
 * @param reply Window containing information whether the user translation is correct or not 
 * @param uInput Window where the user enters his translation
 * @param Vocs Structure containing all vocabulary and their amount
 * @param idx Index of the vocabulary to be queried
 * @param curVoc Number of current vocabulary (to show how many words were queried so far)
 * @return Number which is -1 if ctrl(o) is pressed (go back to main menu) and 0 else
 */
int queryJaToEn(WINDOW * queries, WINDOW * reply, WINDOW * uInput, WINDOW * userStats, VocInfo Vocs, int idx, int curVoc) {
	curs_set(true); cbreak(); nonl(); noecho(); intrflush(stdscr, false); keypad(uInput, true);
	refresh();

	/* colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	/* colors */

	int queriesWidth = 60;
	int userStatsHeight = 6;
	int userStatsWidth = 20;
	int maxInputLen = 25;
	char uTrans[maxInputLen];
	std::fill(uTrans, uTrans+maxInputLen, 0);
	bool isFuriVisible = false;

	/* print query */
	string ja = Vocs.ja[idx];
	string en = Vocs.en[idx];
	string furi = Vocs.furi[idx];
	wattron(queries,COLOR_PAIR(1));
	mvwprintw(queries, 1, queriesWidth/2-ja.length()/3, ja.c_str()); // divided by 6 because one ja char has a length of 3
	wattroff(queries,COLOR_PAIR(1));
	wrefresh(queries);
	/* print query */


	wmove(uInput, 0, 1);
	/* get user input */
	for (int i=0;i<maxInputLen;++i) {
		char u = wgetch(uInput);
		if (u == 13) break;
		else if (u == ctrl('o')) return -1;
	
		else if ( (int) u == ctrl(KEY_BACKSPACE) ) { 
			if (i>0) {
				wmove(uInput, 0, i);
				wclrtoeol(uInput);
				uTrans[i] = 0; // empty corresponding uTrans entry
				i-=2;
			}
		}
		
		/* clear the whole line and delete all saved data in uTrans */
		else if (u == ctrl('n')) {
			wmove(uInput, 0, 0);
			wclrtoeol(uInput);
			wmove(uInput, 0, 1);
			std::fill(uTrans, uTrans+maxInputLen, 0); // reset uTrans 
			i = -1;
		}
		/* toggle furigana visibility */
		else if (u == ctrl('f')) {
			curs_set(false);
			if (isFuriVisible) {
				wattron(queries, A_INVIS);
				isFuriVisible = false;
			} else isFuriVisible = true;
			(furi.empty()) ? : mvwprintw(queries, 0, queriesWidth/2-furi.length()/3-1, "[%s]",furi.c_str()); // only print if not empty
			wattroff(queries, A_INVIS);
			wrefresh(queries);
			i -= 1;
		}
		else {
			wrefresh(uInput);
			curs_set(true);
			uTrans[i] = u;
			wprintw(uInput, "%c",u);
		}
	}
	/* get user input */

	wclear(reply);

	/* getting all remaining translations and storing them in a string separated by semicolons */
	string remainTrans;
	string delim = ";";
	vector<string> remTrans = getRemTrans(uTrans, en);
	for (int i=0;i<remTrans.size();++i) (i==0) ? remainTrans += remTrans[i] : remainTrans += delim+remTrans[i];
	/* getting all remaining translations and storing them in a string separated by semicolons */

	/* if translation is correct */
	if (isSubSet(uTrans, en)) {
		++corUTrans;
		wattron(reply, COLOR_PAIR(2));
		box(reply, 0, 0);
		wattroff(reply, COLOR_PAIR(2));
		string remRply; // all remaining replies
		int startReplyIdx; // index for where reply text in reply window shall start
		(remTrans.size()>0) ? remRply = "also correct:" : remRply = "correct";
		string totReply = remRply+" "+remainTrans;
		startReplyIdx = queriesWidth/2-totReply.length()/2;

		/* checking if reply text fits inside of reply window and if not adjust reply text */
		if (startReplyIdx < 1){ 
			string lenAdjstRply = totReply.substr(0,queriesWidth-6)+"..."; // length adjusted string, as it otherwise would not fit in reply window
			mvwprintw(reply, 2, 1, lenAdjstRply.c_str());   
		} 
		else {
			mvwprintw(reply, 2, startReplyIdx, totReply.c_str());   
		}
		/* checking if reply text fits inside of reply window and if not adjust reply text */
			
	}
	/* if translation is correct */

	/* if translation is false */
	else {
		wattron(reply, COLOR_PAIR(1));
		box(reply, 0, 0);
		wattroff(reply, COLOR_PAIR(1));
		string jpReply; 

		(furi.empty()) ? (jpReply = ja) : (jpReply = ja+" ["+furi+"]"); 
		int jpReplyIdx = queriesWidth/2 - jpReply.length()/2;
		wattron(reply, COLOR_PAIR(1));
		mvwprintw(reply, 1, jpReplyIdx, jpReply.c_str());
		wattroff(reply, COLOR_PAIR(1));

		int enReplyIdx = queriesWidth/2-remainTrans.length()/2;
		/* checking if reply text fits inside of reply window and if not adjust reply text */
		if (enReplyIdx < 1){ 
			string lenAdjstRply = remainTrans.substr(0,queriesWidth-5)+"..."; // length adjusted string, as it otherwise would not fit in reply window
			mvwprintw(reply, 3, 1, lenAdjstRply.c_str());   
		} 
		else {
			mvwprintw(reply, 3, enReplyIdx, remainTrans.c_str());   
		}
		/* checking if reply text fits inside of reply window and if not adjust reply text */
	}
	/* if translation is false */

	wrefresh(reply);
	wclear(queries);
	refresh();
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	if (*uTrans == ctrl('o')) return -1;

	/* fill stats window */
	/* box */
	wattron(userStats, COLOR_PAIR(4));
	box(userStats, 0, 0);
	wattroff(userStats, COLOR_PAIR(4));
	/* box */
	/* header */
	mvwprintw(userStats, 0, 2, "Statistics");
	/* header */
	string userStatsMessage = "Total: ";
	wattron(userStats, COLOR_PAIR(2));
	mvwprintw(userStats, userStatsHeight/2, userStatsWidth/2-1, "%d",corUTrans);
	wattroff(userStats, COLOR_PAIR(2));
	wprintw(userStats, "/");
	wprintw(userStats, "%d",curVoc+1);
	mvwprintw(userStats, userStatsHeight-1, 1, "%s%d",userStatsMessage.c_str(),Vocs.vocNum);
	wrefresh(userStats);
	/* fill stats window */

	return 0;
}

/**
 * Queries the user for a single english to japanese translation
 *
 * @param queries Window containing the english vocabulary that is to be translated by the user
 * @param reply Window containing information whether the user translation is correct or not 
 * @param uInput Window where the user enters his translation
 * @param Vocs Structure containing all vocabulary and their amount
 * @param idx Index of the vocabulary to be queried
 * @param curVoc Number of current vocabulary (to show how many words were queried so far)
 * @return Number which is -1 if ctrl(o) is pressed (go back to main menu) and 0 else
 */
int queryEnToJa(WINDOW * queries, WINDOW * reply, WINDOW * uInput, WINDOW * userStats, VocInfo Vocs, int idx, int curVoc) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(uInput, true);
	refresh();
	
	/* colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	/* colors */

	int queriesWidth = 60;
	int userStatsHeight = 6;
	int userStatsWidth = 20;
	int maxInputLen = 25;
	char uTrans[maxInputLen];
	std::fill(uTrans, uTrans+maxInputLen, 0);

	/* print query */
	string en = Vocs.en[idx];
	string ja = Vocs.ja[idx];
	string furi = Vocs.furi[idx];
	wattron(queries,COLOR_PAIR(1));
	//mvwprintw(queries, 1, queriesWidth/2-en.length()/2, en.c_str());
	int startQryIdx = queriesWidth/2-en.length()/2;

	/* checking if query text fits inside of query window and if not adjust query text */
	if (startQryIdx < 1){ 
		string lenAdjstQry = en.substr(0,queriesWidth-5)+"..."; // length adjusted string, as it otherwise would not fit in query window
		mvwprintw(queries, 1, 1, lenAdjstQry.c_str());   
	} 
	else {
		mvwprintw(queries, 1, startQryIdx, en.c_str());   
	}
	/* checking if query text fits inside of query window and if not adjust query text */

	wattroff(queries,COLOR_PAIR(1));
	wrefresh(queries);
	/* print query */
	
	wmove(uInput, 0, 1);
	/* get user input */
	for (int i=0;i<maxInputLen;++i) {
		char u = wgetch(uInput);
		if (u == 13) break;
		else if (u == ctrl('o')) return -1;
		else if ((int) u == ctrl(KEY_BACKSPACE)) {
			wmove(uInput, 0, 0);
			wclrtoeol(uInput);
			wmove(uInput, 0, 1);
			std::fill(uTrans, uTrans+maxInputLen, 0); // reset uTrans 
			i = -1;
		}
		/* clear the whole line and delete all saved data in uTrans */
		else if (u == ctrl('n')) {
			wmove(uInput, 0, 0);
			wclrtoeol(uInput);
			wmove(uInput, 0, 1);
			std::fill(uTrans, uTrans+maxInputLen, 0); // reset uTrans 
			i = -1;
		}
		else uTrans[i] = u;
	}
	/* get user input */

	wclear(reply);

	if (isSubSet(uTrans, ja)) {
		++corUTrans;
		string answer0 = "correct";
		wattron(reply, COLOR_PAIR(2));
		box(reply, 0, 0);
		wattroff(reply, COLOR_PAIR(2));
		mvwprintw(reply, 2, queriesWidth/2-answer0.length()/2, answer0.c_str());
	}
	else if ( isSubSet(uTrans, furi) && (!furi.empty()) ) {
		++corUTrans;
		string kanjiExis = "kanji notation: ";
		wattron(reply, COLOR_PAIR(2));
		box(reply, 0, 0);
		wattroff(reply, COLOR_PAIR(2));
		wmove(reply, 2, queriesWidth/2-kanjiExis.length()/2-ja.length()/6-ja.length()/6);
		wprintw(reply, kanjiExis.c_str());
		wattron(reply,COLOR_PAIR(1));
		wprintw(reply, ja.c_str());
		wattroff(reply,COLOR_PAIR(1));
	}
	else {
		wattron(reply, COLOR_PAIR(1));
		box(reply, 0, 0);
		wattroff(reply, COLOR_PAIR(1));
		int jpReplyIdx;
		int enReplyIdx;
		string jpReply;

		(furi.empty()) ? (jpReply=ja) : (jpReply=ja+" ["+furi+"]");
		jpReplyIdx = queriesWidth/2 - jpReply.length()/2;
		mvwprintw(reply, 3, jpReplyIdx, jpReply.c_str());

		enReplyIdx = queriesWidth/2-en.length()/2;
		wattron(reply, COLOR_PAIR(1));
		/* checking if reply text fits inside of reply window and if not adjust reply text */
		if (enReplyIdx < 1){ 
			string lenAdjstRply = en.substr(0,queriesWidth-5)+"..."; // length adjusted string, as it otherwise would not fit in reply window
			mvwprintw(reply, 1, 1, lenAdjstRply.c_str());   
		} 
		else {
			mvwprintw(reply, 1, enReplyIdx, en.c_str());   
		}
		/* checking if reply text fits inside of reply window and if not adjust reply text */
		wattroff(reply, COLOR_PAIR(1));
	}

	wrefresh(reply);
	wclear(queries);
	refresh();
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	if (*uTrans == ctrl('o')) return -1;

	/* fill stats window */
	/* box */
	wattron(userStats, COLOR_PAIR(4));
	box(userStats, 0, 0);
	wattroff(userStats, COLOR_PAIR(4));
	/* box */
	/* header */
	mvwprintw(userStats, 0, 2, "Statistics");
	/* header */
	string userStatsMessage = "Total: ";
	wattron(userStats, COLOR_PAIR(2));
	mvwprintw(userStats, userStatsHeight/2, userStatsWidth/2-1, "%d", corUTrans);
	wattroff(userStats, COLOR_PAIR(2));
	wprintw(userStats, "/");
	wprintw(userStats, "%d",curVoc);
	mvwprintw(userStats, userStatsHeight-1, 1, "%s%d",userStatsMessage.c_str(),Vocs.vocNum);
	wrefresh(userStats);
	/* fill stats window */
		
	return 0;
}

/**
 * Queries the user for mixed translations, meaning english to japanese and vice versa
 *
 * @param queries Window containing the english or japanese vocabulary that is to be translated by the user
 * @param reply Window containing information whether the user translation is correct or not 
 * @param uInput Window where the user enters his translation
 * @param Vocs Structure containing all vocabulary and their amount
 * @param idx Index of the vocabulary to be queried
 * @param curVoc number of current vocabulary (to show how many words were queried so far)
 * @return Number which is -1 if ctrl(o) is pressed (go back to main menu) and 0 else
 */
int queryMixed(WINDOW * queries, WINDOW * reply, WINDOW * uInput, WINDOW * userStats, VocInfo Vocs, int idx, int curVoc) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	refresh();

	/* randomly choose to query either ja->en or en->ja */
	int rndm = rand() % 2;
	int status;
	if (rndm == 0) status = queryJaToEn(queries, reply, uInput, userStats, Vocs, idx, curVoc);
	else status = queryEnToJa(queries, reply, uInput, userStats, Vocs, idx, curVoc);
	/* randomly choose to query either ja->en or en->ja */

	wclear(queries);
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	return status;
	//refresh();
}

/**
 * Queries the user for all vocabulary found in the dictionary file
 *
 * @param dict Name of the dictionary file
 * @param uOption Query option selected by user (english to japanese, japanese to english or mixed)
 */
void queryAll(string dict,int uOption) {
	cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	clear();

	/* colors */
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	/* colors */

	int maxY, maxX; getmaxyx(stdscr, maxY, maxX);

	/* frame with option name */
	attron(COLOR_PAIR(3));
	box(stdscr, 0, 0);
	attroff(COLOR_PAIR(3));
	/* frame with option name */

	/* queries window */
	int queriesHeight, queriesWidth, queriesPosY, queriesPosX;
	queriesHeight = 3;
	queriesWidth = 60;
	queriesPosY = maxY/4-queriesHeight/2;
	queriesPosX = maxX/2-queriesWidth/2;
	WINDOW * queries = newwin(queriesHeight, queriesWidth, queriesPosY, queriesPosX);
	refresh();
	/* queries window */

	/* reply window */
	int replyHeight, replyWidth, replyY, replyX;
	replyHeight = 5;
	replyWidth = 60;
	replyY = 2*maxY/4-replyHeight/2;
	replyX = maxX/2-replyWidth/2;
	WINDOW * reply = newwin(replyHeight, replyWidth, replyY, replyX);
	refresh();
	/* reply window */

	/* user statistics window */
	int userStatsHeight, userStatsWidth, userStatsY, userStatsX;
	userStatsY = maxY/16;
	userStatsX = 6*maxX/7;
	userStatsHeight = 7;
	userStatsWidth = 20;
	WINDOW * userStats = newwin(userStatsHeight, userStatsWidth, userStatsY, userStatsX);
	refresh();
	/* user statistics window */

	/* user results window */
	int resultsHeight, resultsWidth, resultsY, resultsX;
	userStatsY = maxY/2-resultsHeight/2;
	userStatsX = maxX/2-resultsWidth/2;
	userStatsHeight = 12;
	userStatsWidth = 25;
	WINDOW * results = newwin(resultsHeight, resultsWidth, resultsY, resultsX);
	refresh();
	box(results, 0, 0);
	wrefresh(results);
	/* user results window */

	/* user input */
	int uInputHeight = 2; int uInputWidth = 30;
	int uInputPosY = 3*maxY/4-uInputHeight/2; int uInputPosX = (maxX-uInputWidth)/2;
	WINDOW * uInput = newwin(uInputHeight, uInputWidth, uInputPosY, uInputPosX);
	refresh();
	mkInputBox(uInput);
	/* user input */

	VocInfo Vocs = getVocs(dict);
	/* to query in a random order */
	srand(time(NULL)); // init random seed based on sys time
	vector<int> indexes;
	for (int t=0; t<Vocs.vocNum;++t) indexes.push_back(t);	
	std::random_shuffle(indexes.begin(),indexes.end());
	/* to query in a random order */
	int status = 0;
	
	if (uOption == 0) {
		wattron(stdscr, COLOR_PAIR(3));
		mvwprintw(stdscr,0, 2, opt1.c_str());
		wattroff(stdscr, COLOR_PAIR(3));
		for (int i=0; i<Vocs.vocNum; ++i) {
			status = queryJaToEn(queries, reply, uInput, userStats, Vocs, indexes[i],i);
			if (status == -1) break;
		} 
	}
	if (uOption == 1) {
		wattron(stdscr, COLOR_PAIR(3));
		mvwprintw(stdscr,0, 2, opt2.c_str());
		wattroff(stdscr, COLOR_PAIR(3));
		for (int i=0; i<Vocs.vocNum; ++i) {
			status = queryEnToJa(queries, reply, uInput, userStats, Vocs, indexes[i],i);
			if (status == -1) break;
		}
	}
	if (uOption == 2) {
		wattron(stdscr, COLOR_PAIR(3));
		mvwprintw(stdscr,0, 2, opt3.c_str());
		wattroff(stdscr, COLOR_PAIR(3));
		for (int i=0; i<Vocs.vocNum; ++i) {
			status = queryMixed(queries, reply, uInput, userStats, Vocs, indexes[i],i);
			if (status == -1) break;
		}
	}

	if (status != -1) getch();
	corUTrans = 0;
}

/**
 * Creates a menu where the user selects a query type (english to japanese, japanese to english, mixed)
 *
 * @param opts Window holding all options
 * @param choices The options which the user can select
 */
int selectionMenu(WINDOW * opts, vector<string> choices) {
	keypad(opts, true);
	/* colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	/* colors */

	int selected = 0;
	while (true) {
		for (int i=0;i<choices.size();++i) {
			if (selected == i) wattron(opts, COLOR_PAIR(1));
			mvwprintw(opts, (opts->_maxy+1)/5+2*i, 1, choices[i].c_str());
			wattroff(opts, COLOR_PAIR(1));
		}	
		int uDir = wgetch(opts);

		if ( (uDir==(int) 'k') || (uDir==KEY_UP) ) {
			--selected;
			if (selected == -1) selected = choices.size()-1;
		}
		else if ( (uDir==(int) 'j') || (uDir==KEY_DOWN) ) {
			++selected;
			if (selected == choices.size()) selected = 0;
		}

		if (uDir == 13) break;
	}
	return selected;
}

/**
 * Creates window showing all dictionary files and lets user choose one
 * @param projectDir The main project directory
 * @return Name of the selected dictionary file
 */
string dictSelect(string projectDir) {
	int y,x;
	int dictSelectH, dictSelectW;
	int choice;
	vector<string> dicts;
	string dictsDir = "/dicts";
	getmaxyx(stdscr, y, x);
	projectDir += dictsDir;
	/* get all file names in dictionary subfolder */
	for (const auto & entry : std::filesystem::directory_iterator(projectDir)){
		string dictFileName = entry.path();
		dictFileName.erase(0,dictFileName.find_last_of('/')+1);
		dicts.push_back(dictFileName);
	}
	/* get all file names in dictionary subfolder */
	/* dictionary select window */
	dictSelectW = 20;
	dictSelectH = 2*dicts.size()+3;
	WINDOW * dictsSelect = newwin(dictSelectH, dictSelectW, y/2-dictSelectH, 2*x/3);
	refresh();
	box(dictsSelect, 0, 0);
	wrefresh(dictsSelect);
	mvwprintw(dictsSelect, 0, 2, "Dictionaries");
	/* dictionary select window */
	choice = selectionMenu(dictsSelect, dicts);
	werase(dictsSelect);
	return dicts[choice];
}

/**
 * Creates the window containing the option select window
 *
 * @param query1 Query type prompting japanese to english translations
 * @param query2 Query type prompting english to japanese translations
 * @param query3 Query type prompting mixed translations
 * @param exit Option to quit the program
 * @return Number of the selected option
 */
int mkOptsWin(string query1, string query2, string query3, string exit, string dicts){
	curs_set(false); cbreak(); noecho(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	clear();

	/* colors */
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	/* colors */

	int maxY, maxX; getmaxyx(stdscr, maxY, maxX);
	int optsWidth = query3.length()+7;
	int optsHeight = 13;
	WINDOW* opts = newwin(optsHeight, optsWidth, maxY/2-optsHeight, maxX/2-optsWidth/2);
	refresh();

	/* box with name */
	wattron(opts, COLOR_PAIR(3));
	box(opts, 0, 0);
	string tag = "Options";
	mvwprintw(opts, 0, optsWidth/2-tag.length()/2-1, tag.c_str());
	wattroff(opts, COLOR_PAIR(3));
	/* box with name */
	wrefresh(opts);

	//string choices[] = {query1,query2,query3,opt4};
	vector<string> choices;	
	choices.push_back(query1); choices.push_back(query2); choices.push_back(query3); choices.push_back(exit);
	choices.push_back(dicts);
	return selectionMenu(opts, choices);
}

/**
 * Creates the start up window
 *
 * @param name The name of the program
 * @param subtitle A subtitle 
 */
void mkStartWin(string name, string subtitle) {

	/* colors */
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	/* colors */

	/* header */
	noecho(); curs_set(false);
	int horPadding = 8;
	int titleHeight = 5; int titleWidth = name.length()+horPadding;
	int maxY, maxX; getmaxyx(stdscr, maxY, maxX);
	int titlePosY = 2*maxY/5-titleHeight/2; int titlePosX = maxX/2-titleWidth/2;
	int curPosY = 0;

	while (curPosY <= titlePosY) {
		clear();
		WINDOW * title = newwin(titleHeight, titleWidth, curPosY, titlePosX);
		++curPosY;
		refresh();
		
		/* box */
		wattron(title, COLOR_PAIR(3));
		box(title, 0,0);
		wattroff(title, COLOR_PAIR(3));
		wrefresh(title);
		/* box */

		/* box text */
		wattron(title,COLOR_PAIR(1)); wattron(title, A_BOLD);
		mvwprintw(title, titleHeight/2, horPadding/2, name.c_str());
		wattroff(title, A_BOLD); wattroff(title,COLOR_PAIR(1));
		wrefresh(title);
		/* box text */

		usleep(2E4);
	}
	/* header */

	/* sub header */
	int lenEnter = subtitle.length();
	attron(A_BLINK);
	mvprintw(titlePosY+titleHeight, maxX/2-(lenEnter+8)/2, subtitle.c_str());

	printw(" ("); 
	attron(COLOR_PAIR(3));
	printw("Enter");
	attroff(COLOR_PAIR(3));
	printw(")");
	attroff(A_BLINK);

	int uChar = getch();
	while (uChar != 13) uChar = getch();
	/* sub header */
}

int main(int argc, char** argv) {
	vector<string> dicts;
	char buffer[250];
	string path = __FILE__;
	int pos = path.find("/cursary.cc");
	size_t len = path.copy(buffer, pos, 0);
	buffer[len] = '\0';
	string dictSubDir = "/dicts/";
	string dictFile = dictSubDir+"enja.txt";
	string dict = buffer + dictFile;

	setlocale(LC_ALL, "");
	initscr(); cbreak(); noecho(); nonl(); intrflush(stdscr, false); keypad(stdscr, true); curs_set(false);

	if (!has_colors()) throw "TERMINAL DOES NOT SUPPORT COLORS.";
	try {
			start_color();
			mkStartWin("Cursary: Your Friendly Neighborhood Voc Trainer", "Insert Coin");
		while (true) {
			char uOption = mkOptsWin(opt1,opt2,opt3,opt4,opt5);
			if (uOption == 4) break;
			else if (uOption == 3) dict = buffer+dictSubDir+dictSelect(buffer);
			else if ( (uOption==0)||(uOption==1)||(uOption==2) ) queryAll(dict,uOption);
			else continue;
		}

	}
	catch (string message) {
		endwin();
		cerr << message << endl;
		return -1;
	}
	endwin();
	return 0;
}
