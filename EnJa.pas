program EnJa;
{$MODE OBJFPC}

uses 	SysUtils, StrUtils;

const 	DictName = 'DictEnJa.txt';
	ProgramName = 'EnJa: The Terminal English-Japanese Vocabulary Trainer';
	MaxSize = 2000; { max number of vocabulary in dictionary, a triplet is considered to be one vocabulary }
	MultiTransSize = 5;
	MultiTransDelim = ';';

type 	TAllVoc = array [1..MaxSize,1..3] of String; { [x,y]: each x holds one vocabulary, i.e. english, kanji or hiragana and if still needed hiragana }
						      { [x,y]: each y holds one specific part of the vocabulary, i.e. y=1: english, y=2: kanji, y=3: hiragana }
	TMultiTrans = array [1..MultiTransSize] of String;

	TVocRec = record
		AllVoc: TAllVoc;
		VocNum: Integer;
	end;

var 	UserPromptDirection: Char;

Operator in (A: String; B: TMultiTrans): Boolean;
var 	i: Integer;
begin
	result := false;
	for i:=1 to MultiTransSize do 
		if A = B[i] then
		begin
			result:=true;
			break;
		end;
end;	

{ return multiple translations in array. If there is only one translation return garbage}
function GetMultiTrans(trans: String): TMultiTrans;
var 	MultiTrans: TMultiTrans;
	i: Integer;
	ContainsMultiTrans: Boolean;
begin
	i := 1;
	ContainsMultiTrans := pos(MultiTransDelim,trans) > 0;
	while pos(MultiTransDelim,trans) > 0 do 
	begin
		MultiTrans[i] := AnsiLowerCase(copy(trans,1,pos(MultiTransDelim,trans)-1));
		Delete(trans,1,pos(MultiTransDelim,trans));
		Inc(i);
	end;
	if ContainsMultiTrans then MultiTrans[i] := trans
	else MultiTrans[i] := '@!^'; { if there is only one translation, return garbage}
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
function GetNVoc(DictName: String): TVocRec;
var	Dict: TextFile;
	CurLine: String;
	i: Integer;
	
begin
	Assign(Dict,DictName);
	Reset(Dict);
	i:=1;
	repeat
		ReadLn(Dict,CurLine);
		while CurLine = '' do ReadLn(Dict,CurLine);
		Result.AllVoc[i][1] := CurLine; {en}
		ReadLn(Dict,CurLine);
		Result.AllVoc[i][2] := CurLine; {ja}
		ReadLn(Dict,CurLine);
		Result.AllVoc[i][3] := CurLine; {hira if present, else empty}
		Inc(i);
	until eof(Dict);
	Result.VocNum := i-1;
	close(Dict);
end;

{ prompt user for english -> japanese translations }
procedure PromptVocEnJa(DictName: String);
var 	UserTrans: String;
	i: Integer;
	VocRec: TVocRec;
begin
	VocRec := GetNVoc(DictName);	
	for i:=1 to VocRec.VocNum do 
	begin
		write(VocRec.AllVoc[i][1]+': ');
		readln(UserTrans);

		if (CompareText(UserTrans,VocRec.AllVoc[i][2]) <> 0) and (CompareText(UserTrans,VocRec.AllVoc[i][3]) <> 0) then { Kanji AND Hiragana are wrongly translated }
			if VocRec.AllVoc[i][3] <> '' then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']')
			else writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][2])
		else if CompareText(UserTrans,VocRec.AllVoc[i][2]) = 0 then writeln('Correct.')
		else if (CompareText(UserTrans,VocRec.AllVoc[i][3]) = 0) and (VocRec.AllVoc[i][3] <> '') then
			writeln('Correct. Kanji notation exists: '+VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']')
		else 	writeln('Oh oh, this statement should not have been reached!');

		writeln;
	end;
end;

{ prompt user for japanese -> english translations }
{ need to add verification if more than one and less than all UserTrans are given, currently message is always 'Incorrect.' }
procedure PromptVocJaEn(DictName: String);
var 	UserTrans: String;
	i: Integer;
	VocRec: TVocRec;
	MultiTrans: TMultiTrans;
begin
	VocRec := GetNVoc(DictName);	
	for i:=1 to VocRec.VocNum do 
	begin
		MultiTrans := GetMultiTrans(VocRec.AllVoc[i][1]);
		if VocRec.AllVoc[i][3] <> '' then write(VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']'+': ')
		else write(VocRec.AllVoc[i][2]+': ');
		
		readln(UserTrans);
		UserTrans := AnsiLowerCase(UserTrans);

		if not (UserTrans in MultiTrans) and (AnsiLowerCase(VocRec.AllVoc[i][1]) <> AnsiLowerCase(UserTrans)) then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][1])
		else if (UserTrans in MultiTrans) then writeln('Correct. All solutions: '+VocRec.AllVoc[i][1])
		else writeln('Correct.');
		writeln;
	end;
end;

procedure PromptVocMixed(DictName: String);
var 	UserTrans: String;
	MultiTrans: TMultiTrans;
	i,a: Integer;
	VocRec: TVocRec;
begin
	VocRec := GetNVoc(DictName);	
	for i:=1 to VocRec.VocNum do 
	begin
		Randomize;
		a := Random(2);
		if a = 0 then
		begin
			write(VocRec.AllVoc[i][1]+': ');
			readln(UserTrans);
	
			if (CompareText(UserTrans,VocRec.AllVoc[i][2]) <> 0) and (CompareText(UserTrans,VocRec.AllVoc[i][3]) <> 0) then 
				if VocRec.AllVoc[i][3] <> '' then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']')
				else writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][2])
			else if CompareText(UserTrans,VocRec.AllVoc[i][2]) = 0 then writeln('Correct.')
			else if (CompareText(UserTrans,VocRec.AllVoc[i][3]) = 0) and (VocRec.AllVoc[i][3] <> '') then
				writeln('Correct. Kanji notation exists: '+VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']')
			else 	writeln('A wild unhandled statement appeard. Catch your current vocabulary and report it!');
			writeln;
		end
		else 
		begin
			MultiTrans := GetMultiTrans(VocRec.AllVoc[i][1]);
			if VocRec.AllVoc[i][3] <> '' then write(VocRec.AllVoc[i][2]+' ['+VocRec.AllVoc[i][3]+']'+': ')
			else write(VocRec.AllVoc[i][2]+': ');
		
			readln(UserTrans);
			UserTrans := AnsiLowerCase(UserTrans);

			if not (UserTrans in MultiTrans) and (AnsiLowerCase(VocRec.AllVoc[i][1]) <> AnsiLowerCase(UserTrans)) then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[i][1])
				else if (UserTrans in MultiTrans) then writeln('Correct. All solutions: '+VocRec.AllVoc[i][1])
				else writeln('Correct.');
				writeln;
			end;
		end;
end;

begin
	CreateOrnament(ProgramName,'-',9);
	writeln;
	writeln('[E]nglish -> Japanese'+#9#9+'[J]apanese -> English'+#9#9+'[M]ixed');
	try
		repeat
			writeln;
			write('Query type: ');
			readln(UserPromptDirection);
			writeln;
			if CompareText(UserPromptDirection,'e')=0 then PromptVocEnJa(DictName)
			else if CompareText(UserPromptDirection,'j')=0 then PromptVocJaEn(DictName)
			else if CompareText(UserPromptDirection,'m')=0 then PromptVocMixed(DictName)
			else writeln('Unknown input. Use [E], [J] or [M].');
		until UserPromptDirection in ['e','E','j','J','m','M'];
	except on E:Exception do 
		writeln('A wild '+E.ClassName+' has appeared.'+#13#10+'It was lured by '+E.Message+'.');
	end;
	
end.
