program EnJa;
{$MODE OBJFPC}

uses 	SysUtils, StrUtils;

const 	dictName = 'DictEnJa.txt';
	ProgramName = 'EnJa: The Terminal English-Japanese Vocabulary Trainer';
	DictSize = 4000; { number of lines in dictionary }
	MultiTransSize = 5;
	MultiTransDelim = ';';

type 	TAllVoc = array [1..DictSize,1..3] of String; { [x,y]: each x holds one vocabulary, i.e. english, kanji or hiragana and if still needed hiragana }
						      { [x,y]: each y holds one specific part of the vocabulary, i.e. y=1: english, y=2: kanji, y=3: hiragana }
	TMultiTrans = array [1..MultiTransSize] of String;

var 	userPromptDirection: Char;

{ return multiple translations in array. If there is only one translation return array with empty string }
function getMultiTrans(trans: String): TMultiTrans;
var 	MultiTrans: TMultiTrans;
	i: Integer;
	containsMultiTrans: Boolean;
begin
	i := 1;
	containsMultiTrans := pos(MultiTransDelim,trans) > 0;
	while pos(MultiTransDelim,trans) > 0 do 
	begin
		MultiTrans[i] := copy(trans,1,pos(MultiTransDelim,trans)-1);
		Delete(trans,1,pos(MultiTransDelim,trans));
		Inc(i);
	end;
	if containsMultiTrans then MultiTrans[i] := trans
	else MultiTrans[i] := ''; { if there is only one translation, return empty string }
	result := MultiTrans;
end;

{ create header of program }
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

{ prompt user for english -> japanese translations }
procedure promptVocEnJa(dictName: String; numVoc: Integer);
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

{ prompt user for japanese -> english translations }
{ need to add verification if more than one and less than all userTrans are given, currently message is always 'Incorrect.' }
procedure promptVocJaEn(dictName: String; numVoc: Integer);
var 	userTrans: String;
	i,j: Integer;
	AllVoc: TAllVoc;
	MultiTrans: TMultiTrans;
	TransCorrect: Boolean;
begin
	AllVoc := getNVoc(dictName, numVoc);	
	for i:=1 to numVoc do 
	begin
		multitrans := getmultitrans(allvoc[i][1]);
		if AllVoc[i][3] <> '' then write(AllVoc[i][2]+' ['+AllVoc[i][3]+']'+': ')
		else write(AllVoc[i][2]+': ');
		
		readln(userTrans);
		userTrans := AnsiLowerCase(userTrans);
		TransCorrect := false;
		for j:= 1 to MultiTransSize do 
		begin
			if userTrans = AnsiLowerCase(MultiTrans[j]) then
			begin
				TransCorrect := true;
				break;
			end;
		end;

		if (not TransCorrect) and (AnsiLowerCase(AllVoc[i][1]) <> AnsiLowerCase(userTrans)) then writeln('Incorrect. Correct answer: '+AllVoc[i][1])
		else if TransCorrect then writeln('Correct. All solutions: '+AllVoc[i][1])
		else writeln('Correct.');
		writeln;
	end;
end;

procedure promptVocMixed(dictName: String; numVoc: Integer);
var 	userTrans: String;
	MultiTrans: TMultiTrans;
	TransCorrect: Boolean;
	j: Integer;
	i,a: Integer;
	AllVoc: TAllVoc;
begin
	AllVoc := getNVoc(dictName, numVoc);	
	for i:=1 to numVoc do 
	begin
		Randomize;
		a := Random(2);
		if a = 0 then
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
		end
		else 
		begin
			multitrans := getmultitrans(allvoc[i][1]);
			if AllVoc[i][3] <> '' then write(AllVoc[i][2]+' ['+AllVoc[i][3]+']'+': ')
			else write(AllVoc[i][2]+': ');
		
			readln(userTrans);
			userTrans := AnsiLowerCase(userTrans);
			TransCorrect := false;
			for j:= 1 to MultiTransSize do 
			begin
				if userTrans = AnsiLowerCase(MultiTrans[j]) then
				begin
					TransCorrect := true;
					break;
				end;
			end;

			if (not TransCorrect) and (AnsiLowerCase(AllVoc[i][1]) <> AnsiLowerCase(userTrans)) then writeln('Incorrect. Correct answer: '+AllVoc[i][1])
				else if TransCorrect then writeln('Correct. All solutions: '+AllVoc[i][1])
				else writeln('Correct.');
				writeln;
			end;
		end;
end;

begin
	CreateOrnament(ProgramName,'-',9);
	writeln;
	writeln('[E]nglish -> Japanese'+#9#9+'[J]apanese -> English'+#9#9+'[M]ixed');
	writeln;
	write('Query type: ');
	readln(userPromptDirection);
	if AnsiLowerCase(userPromptDirection) = 'e' then promptVocEnJa(dictName,DictSize)
	else if AnsiLowerCase(userPromptDirection) = 'j' then promptVocJaEn(dictName,DictSize)
	else if AnsiLowerCase(userPromptDirection) = 'm' then promptVocMixed(dictName,DictSize)
	else writeln('Unknown input. Use [E], [J] or [M].');
end.
