#include <algorithm>
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

#define ctrl(x) (x & 0x1F)

using std::string;
using std::vector;
using std::ios;
using std::fstream;
using std::cerr;
using std::endl;

struct VocInfo {
	int vocNum;
	vector<string> en;
	vector<string> ja;
	vector<string> furi;
};

const string opt1 = "Japanese -> English";
const string opt2 = "Japanese <-  English";
const string opt3 = "Japanese <-> English";
const string opt4 = "Exit";

/*
 *task: 	save all vocs and their amount inside a struct
 * arguments: 	dict: 		name of the dictionary file where all vocs are stored
 * return 	struct containing all vocs and how many there are	
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
	Vocs.vocNum = Vocs.en.size()-1;
	return Vocs;
} 

/*
 * task: 	put each translation into a single element of a vector
 * arguments: 	trans: 		string with at least one translation
 * return 	vector containing all translations as single elements
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

/*
 * task: 	check if translation is part of or equal to solution
 * arguments:
 * 		uTrans: 	user translations separated with semicolons
 * 		trans: 		correct translations from dict fillSides
 * return: 	boolean if at least one user trans is in the dict translations
 */
bool isSubSet(string uTrans, string trans) {
	vector<string> vecUTrans = partitionAllTrans(uTrans);
	vector<string> vecTrans = partitionAllTrans(trans);
	int vocsRight = 0;
	for (int i=0; i<vecUTrans.size(); ++i) {
		for (int j=0; j<vecTrans.size();++j) {
			if (vecUTrans[i] == vecTrans[j]) {
				++vocsRight;
				break;
			}
		}	
	}
	if (vocsRight == vecUTrans.size()) return true;
	else return false;
}

void mkInputBox(WINDOW * winName) {
	int left,right,top,bottom,tlc,trc,blc,brc;
	top = left = right = tlc = trc = blc = brc = 23; //space
	bottom = 0;
	wborder(winName, left, right, top, bottom, tlc, trc, blc, brc);
	wrefresh(winName);
}

int mkOpt1Win(WINDOW * queries, WINDOW * reply, WINDOW * uInput, VocInfo Vocs, int idx) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	refresh();

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	int queriesWidth = 60;
	int maxInputLen = 25;
	char uTrans[maxInputLen];

	/* print query */
	string ja = Vocs.ja[idx];
	string en = Vocs.en[idx];
	string furi = Vocs.furi[idx];
	wattron(queries,COLOR_PAIR(1));
	mvwprintw(queries, 0, queriesWidth/2-ja.length()/3, ja.c_str()); // divided by 6 because one ja char has a length of 3
	wattroff(queries,COLOR_PAIR(1));
	wrefresh(queries);
	/* print query */

	/* get user input */
	mvwgetnstr(uInput, 0, 1, uTrans,maxInputLen);
	
	if (isSubSet(uTrans, en)) {
		wclear(reply);
		mvwprintw(reply, 0, 0, "%s","correct");
		wrefresh(reply);
	}
	else {
		wclear(reply);
		mvwprintw(reply, 0, 0, "false");
		mvwprintw(reply, 1, 0, "answer:");
		wattron(reply, COLOR_PAIR(1));
		(furi.empty()) ? wprintw(reply, ja.c_str()) : wprintw(reply, "%s [%s]",ja.c_str(),furi.c_str()); 
		wattroff(reply, COLOR_PAIR(1));
		wprintw(reply, " -> %s",en.c_str()); 
		wrefresh(reply);
	}
	wclear(queries);
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	refresh();
	if (*uTrans == ctrl('o')) return -1;
	return 0;
	/* get user input */
}

int mkOpt2Win(WINDOW * queries, WINDOW * reply, WINDOW * uInput, VocInfo Vocs, int idx) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	refresh();

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	int queriesWidth = 60;
	int maxInputLen = 25;
	char uTrans[maxInputLen];

	/* print query */
	string en = Vocs.en[idx];
	string ja = Vocs.ja[idx];
	string furi = Vocs.furi[idx];
	wattron(queries,COLOR_PAIR(1));
	mvwprintw(queries, 0, queriesWidth/2-en.length()/2, en.c_str());
	wattroff(queries,COLOR_PAIR(1));
	wrefresh(queries);
	/* print query */
	
	/* get user input */
	mvwgetnstr(uInput, 0, 1, uTrans,maxInputLen);
		
	if (isSubSet(uTrans, ja)) {
		wclear(reply);
		mvwprintw(reply, 0, 0, "%s","correct");
		wrefresh(reply);
	}
	else if (isSubSet(uTrans, furi)) {
		wclear(reply);
		mvwprintw(reply, 0, 0, "%s","correct, kanji notation: ");
		wattron(reply,COLOR_PAIR(1));
		wprintw(reply, ja.c_str());
		wattroff(reply,COLOR_PAIR(1));
		wrefresh(reply);
	}
	else {
		wclear(reply);
		mvwprintw(reply, 0, 0, "false");
		mvwprintw(reply, 1, 0, "answer:");
		wattron(reply, COLOR_PAIR(1));
		wprintw(reply, en.c_str());
		wattroff(reply, COLOR_PAIR(1));
		(furi.empty()) ? wprintw(reply, " -> %s",ja.c_str()) : wprintw(reply, " -> %s [%s]",ja.c_str(),furi.c_str()); 
		wrefresh(reply);
	}

	wclear(queries);
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	refresh();
	if (*uTrans == ctrl('o')) return -1;
	return 0;
	/* get user input */
}

int mkOpt3Win(WINDOW * queries, WINDOW * reply, WINDOW * uInput, VocInfo Vocs, int idx) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	refresh();

	/* randomly choose to query either ja->en or en->ja */
	int rndm = rand() % 2;
	int status;
	if (rndm == 0) status = mkOpt1Win(queries,reply,uInput,Vocs,idx);
	else status = mkOpt2Win(queries,reply,uInput,Vocs,idx);
	/* randomly choose to query either ja->en or en->ja */

	wclear(queries);
	wmove(uInput, 0, 0); wclrtoeol(uInput);
	return status;
	//refresh();
}

void queryVocs(string dict,int uOption) {
	curs_set(true); cbreak(); echo(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	clear();

	/* frame with option name */
	int maxY, maxX; getmaxyx(stdscr, maxY, maxX);
	box(stdscr, 0, 0);
	/* frame with option name */

	/* queries window */
	int queriesHeight = 3; int queriesWidth = 60;
	int queriesPosY = maxY/4-queriesHeight/2; int queriesPosX = maxX/2-queriesWidth/2;
	WINDOW * queries = newwin(queriesHeight, queriesWidth, queriesPosY, queriesPosX);
	refresh();
	/* queries window */

	/* reply window */
	int replyHeight, replyWidth, replyY, replyX;
	replyHeight = 3;
	replyWidth = 60;
	replyY = 2*maxY/4-replyHeight/2;
	replyX = maxX/2-replyWidth/2;
	WINDOW * reply = newwin(replyHeight, replyWidth, replyY, replyX);
	refresh();
	/* reply window */

	/* user input */
	int uInputHeight = 2; int uInputWidth = 30;
	int uInputPosY = 3*maxY/4-uInputHeight/2; int uInputPosX = (maxX-uInputWidth)/2;
	WINDOW * uInput = newwin(uInputHeight, uInputWidth, uInputPosY, uInputPosX);
	refresh();
	mkInputBox(uInput);
	/* user input */

	VocInfo Vocs = getVocs(dict);

	if (uOption == 0) {
		mvwprintw(stdscr,0, 2, opt1.c_str());
		for (int i=0; i<Vocs.vocNum-1; ++i) {
			int status = mkOpt1Win(queries,reply,uInput,Vocs,i);
			if (status == -1) break;
		} 
	}
	if (uOption == 1) {
		mvwprintw(stdscr,0, 2, opt2.c_str());
		for (int i=0; i<Vocs.vocNum-1; ++i) {
			int status = mkOpt2Win(queries,reply,uInput,Vocs,i);
			if (status == -1) break;
		}
	}
	if (uOption == 2) {
		mvwprintw(stdscr,0, 2, opt3.c_str());
		srand(time(NULL)); // init random seed based on sys time
		for (int i=0; i<Vocs.vocNum-1; ++i) {
			int status = mkOpt3Win(queries,reply,uInput,Vocs,i);
			if (status == -1) break;
		}
	}
}

int selectionMenu(WINDOW * opts, string choices[], int fields) {
	keypad(opts, true);
	int selected = 0;
	while (true) {
		for (int i=0;i<fields;++i) {
			if (selected == i) wattron(opts, A_REVERSE);
			mvwprintw(opts, (opts->_maxy+1)/5+2*i, 1, choices[i].c_str());
			wattroff(opts, A_REVERSE);
		}	
		int uDir = wgetch(opts);

		if ( (uDir==(int) 'k') || (uDir==KEY_UP) ) {
			--selected;
			if (selected == -1) selected = fields-1;
		}
		else if ( (uDir==(int) 'j') || (uDir==KEY_DOWN) ) {
			++selected;
			if (selected == 4) selected = 0;
		}

		if (uDir == 13) break;
	}
	return selected;
}

int mkOptsWin(string query1, string query2, string query3, string exit, int optsHeight){
	curs_set(false); cbreak(); noecho(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);
	clear();

	int maxY, maxX; getmaxyx(stdscr, maxY, maxX);
	int optsWidth = query3.length()+7;
	WINDOW* opts = newwin(optsHeight, optsWidth, maxY/2-optsHeight, maxX/2-optsWidth/2);
	refresh();

	/* box with name */
	box(opts, 0, 0);
	string tag = "Options";
	mvwprintw(opts, 0, optsWidth/2-tag.length()/2-1, tag.c_str());
	/* box with name */
	wrefresh(opts);

	string choices[] = {query1,query2,query3,opt4};
	return selectionMenu(opts, choices, 4);
}

void mkStartWin(string name, string subtitle) {
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
		
		/* box */
		refresh();
		box(title, 0,0);
		wrefresh(title);
		/* box */

		/* box text */
		init_pair(1, COLOR_RED, COLOR_BLACK);
		wattron(title,COLOR_PAIR(1)); wattron(title, A_BOLD);
		mvwprintw(title, titleHeight/2, horPadding/2, name.c_str());
		wattroff(title, A_BOLD); wattroff(title,COLOR_PAIR(1));
		wrefresh(title);
		/* box text */

		usleep(3E4);
	}
	/* header */

	/* sub header */
	int lenEnter = subtitle.length();
	attron(A_BLINK);
	mvprintw(titlePosY+titleHeight, maxX/2-(lenEnter+8)/2, subtitle.c_str());

	printw(" ("); 
	init_pair(2, COLOR_YELLOW, COLOR_BLACK); attron(COLOR_PAIR(2));
	printw("Enter");
	attroff(COLOR_PAIR(2));
	printw(")");
	attroff(A_BLINK);

	int uChar = getch();
	while (uChar != 13) uChar = getch();
	/* sub header */
}

int main(int argc, char** argv) {
	char buffer[250];
	string path = __FILE__;
	int pos = path.find("/cursary.cc");
	size_t len = path.copy(buffer, pos, 0);
	buffer[len] = '\0';
	string locDictDir = "/dicts/enja.txt";
	const string dict = buffer + locDictDir;

	setlocale(LC_ALL, "");
	curs_set(false); initscr(); cbreak(); noecho(); nonl(); intrflush(stdscr, false); keypad(stdscr, true);

	if (!has_colors()) throw "TERMINAL DOES NOT SUPPORT COLORS.";
	try {
			start_color();
			mkStartWin("Cursary: Your Friendly Neighborhood Voc Trainer", "Insert Coin");
		while (true) {
			char uOption = mkOptsWin(opt1,opt2,opt3,opt4,11);
			if (uOption == 3) break;
			else if ( (uOption==0)||(uOption==1)||(uOption==2) ) queryVocs(dict,uOption);
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