program EnJa;
{$MODE OBJFPC}

uses 	SysUtils, StrUtils;

const 	dictName = 'DictEnJa.txt';
	ProgramName = 'EnJa: The Terminal English-Japanese Vocabulary Trainer';
	DictSize = 4000; { number of lines in dictionary }

type 	TAllVoc = array [1..DictSize,1..3] of String; { [x,y]: each x holds one vocabulary, i.e. english, kanji or hiragana and if still needed hiragana }
						      { [x,y]: each y holds one specific part of the vocabulary, i.e. y=1: english, y=2: kanji, y=3: hiragana }

procedure CreateOrnament(Header: String; Ornament: Char; Padding: Integer);
var 	OrnamentLength: Integer;
	ExpandedOrnament: String;
begin
	OrnamentLength := length(Header)+Padding*2;
	ExpandedOrnament := DupeString(Ornament,OrnamentLength);
	writeln;
	writeln(ExpandedOrnament);
	writeln(PadLeft(Header,length(Header)+Padding));
	writeln(ExpandedOrnament);
	writeln;
end;

{ - get all vocabulary up to line numVoc - }
function getNVoc(dictName: String; numVoc: Integer): TAllVoc;
var	dict: TextFile;
	curLine: String;
	vocIndex: Integer;
	AllVoc: TAllVoc;
	
begin
	Assign(dict,dictName);
	Reset(dict);
	for vocIndex:=1 to numVoc do
	begin
		if eof(dict) then break;
		ReadLn(dict,curLine);
		while curLine = '' do ReadLn(dict,curLine);
		AllVoc[vocIndex][1] := curLine; {en}
		ReadLn(dict,curLine);
		AllVoc[vocIndex][2] := curLine; {ja}
		ReadLn(dict,curLine);
		AllVoc[vocIndex][3] := curLine; {hira if present, else empty}
	end;
	result := AllVoc;
	close(dict);
end;

procedure promptVoc(dictName: String; numVoc: Integer);
var 	userTrans: String;
	i: Integer;
	AllVoc: TAllVoc;
begin
	AllVoc := getNVoc(dictName, numVoc);	
	for i:=1 to numVoc do 
	begin
		write(AllVoc[i][1]+': ');
		readln(userTrans);

		if (CompareText(userTrans,AllVoc[i][2]) <> 0) and (CompareText(userTrans,AllVoc[i][3]) <> 0) then 
			if AllVoc[i][3] <> '' then writeln('Incorrect. Correct answer: '+AllVoc[i][2]+' ['+AllVoc[i][3]+']')
			else writeln('Incorrect. Correct answer: '+AllVoc[i][2])
		else if CompareText(userTrans,AllVoc[i][2]) = 0 then
		begin
			if AllVoc[i][3] <> '' then writeln('Correct. Pronunciation: '+AllVoc[i][3]+'.')
			else writeln('Correct.');
		end
		else if (CompareText(userTrans,AllVoc[i][3]) = 0) and (AllVoc[i][3] <> '') then
			writeln('Correct. Kanji notation exists: '+AllVoc[i][2]+' ['+AllVoc[i][3]+']')
		else 	writeln('Oh oh, this statement should not have been reached!');

		writeln;
	end;
end;

begin
	CreateOrnament(ProgramName,'-',6);
	promptVoc(dictName, DictSize);
end.
