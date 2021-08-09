program EnJa;
{$MODE OBJFPC}

uses 	SysUtils, StrUtils;

const 	DictName = 'DictEnJa.txt';
	ProgramName = 'EnJa: The Terminal English-Japanese Vocabulary Trainer';
	MaxSize = 2000; { max number of vocabulary in dictionary, a triplet is considered to be one vocabulary }
	MultiTransSize = 5; { max number of english words, that point to same japanese word }
	MultiTransDelim = ';'; { delimiter, if multiple english words are given }

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
		if (CompareText(A,B[i]) = 0) then
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
		MultiTrans[i] := copy(trans,1,pos(MultiTransDelim,trans)-1);
		Delete(trans,1,pos(MultiTransDelim,trans));
		Inc(i);
	end;
	if ContainsMultiTrans then MultiTrans[i] := trans
	else MultiTrans[i] := '@!^'; { if there is only one translation, return garbage}
	result := MultiTrans;
end;

{ create header of program }
procedure CreateHeader(Header: String; Ornament: Char; Padding: Integer);
var 	OrnamentLength: Integer;
	ExpandedOrnament: String;
begin
	OrnamentLength := length(Header)+Padding*2;
	ExpandedOrnament := DupeString(Ornament,OrnamentLength);
	writeln(#10#13+ExpandedOrnament);
	writeln(PadLeft(Header,length(Header)+Padding));
	writeln(#10#13+ExpandedOrnament);
end;

procedure CreateFooter(Ornament: Char; OrnamentLength: Integer);
var 	ExpandedOrnament: String;
begin
	ExpandedOrnament := DupeString(Ornament,OrnamentLength);
	writeln(ExpandedOrnament+#10#13);
end;


{ get all vocabulary up to line numVoc }
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
		result.AllVoc[i][1] := CurLine; { i.th voc, en}
		ReadLn(Dict,CurLine);
		result.AllVoc[i][2] := CurLine; { i.th voc, ja}
		ReadLn(Dict,CurLine);
		result.AllVoc[i][3] := CurLine; { i.th voc, hira if present, else empty}
		Inc(i);
	until eof(Dict);
	result.VocNum := i-1; 			{ number of vocabulary stored }
	close(Dict);
end;

{ prompt user for english -> japanese translations }
procedure PromptVocEnJa(DictName: String; VocIndex: Integer);
var 	UserTrans: String;
	VocRec: TVocRec;
begin
	VocRec := GetNVoc(DictName);	
	write(#10#13+VocRec.AllVoc[VocIndex][1]+': ');
	readln(UserTrans);
	if (UserTrans = '') and (VocRec.AllVoc[VocIndex][3] <> '') then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[VocIndex][2]+' ['+VocRec.AllVoc[VocIndex][3]+']')
	else if (UserTrans = '') and (VocRec.AllVoc[VocIndex][3] = '') then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[VocIndex][2])
	else
		if (CompareText(UserTrans,VocRec.AllVoc[VocIndex][2]) <> 0) and (CompareText(UserTrans,VocRec.AllVoc[VocIndex][3]) <> 0) then 
			if VocRec.AllVoc[VocIndex][3] <> '' then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[VocIndex][2]+' ['+VocRec.AllVoc[VocIndex][3]+']')
			else writeln('Incorrect. Correct answer: '+VocRec.AllVoc[VocIndex][2])
		else if CompareText(UserTrans,VocRec.AllVoc[VocIndex][2]) = 0 then writeln('Correct.')
		else if (CompareText(UserTrans,VocRec.AllVoc[VocIndex][3]) = 0) and (VocRec.AllVoc[VocIndex][3] <> '') then
			writeln('Correct. Kanji notation exists: '+VocRec.AllVoc[VocIndex][2]+' ['+VocRec.AllVoc[VocIndex][3]+']')
		else 	writeln('A wild unhandled statement appeard. Catch your current vocabulary and report it!');
end;

{ prompt user for japanese -> english translations }
{ need to add verification if more than one and less than all UserTrans are given, currently message is always 'Incorrect.' }
procedure PromptVocJaEn(DictName: String; VocIndex: Integer);
var 	UserTrans: String;
	VocRec: TVocRec;
	MultiTrans: TMultiTrans;
begin
	VocRec := GetNVoc(DictName);	
	MultiTrans := GetMultiTrans(VocRec.AllVoc[VocIndex][1]);
	if VocRec.AllVoc[VocIndex][3] <> '' then write(#10#13+VocRec.AllVoc[VocIndex][2]+' ['+VocRec.AllVoc[VocIndex][3]+']'+': ')
	else write(#10#13+VocRec.AllVoc[VocIndex][2]+': '); 
	
	readln(UserTrans);
	if (UserTrans = '') or ( not (UserTrans in MultiTrans) ) and ( CompareText(VocRec.AllVoc[VocIndex][1],UserTrans) <> 0 ) then writeln('Incorrect. Correct answer: '+VocRec.AllVoc[VocIndex][1])
	else if (UserTrans in MultiTrans) then writeln('Correct. All solutions: '+VocRec.AllVoc[VocIndex][1])
	else writeln('Correct.');
end;

procedure PromptVocMixed(DictName: String; VocIndex: Integer);
var 	a: Integer;
begin
	a := Random(2);
	if a > 0 then PromptVocEnJa(DictName, VocIndex)
	else PromptVocJaEn(DictName, VocIndex);
end;

procedure StartPrompt(PrompDirection: Char);
var 	VocRec: TVocRec;
	i: Integer;
begin
	VocRec := GetNVoc(DictName);
	case (AnsiLowerCase(PrompDirection)) of
		'e': for i:=1 to VocRec.VocNum do PromptVocEnJa(DictName,i);
		'j': for i:=1 to VocRec.VocNum do PromptVocJaEn(DictName,i);
		'm': for i:=1 to VocRec.VocNum do PromptVocMixed(DictName,i);
		'': writeln('Please choose a query type.');
	else writeln('Unknown input. Use [E], [J] or [M].');
	end;
end;

begin
	Randomize;
	CreateHeader(ProgramName,'-',9);
	writeln(#10#13+'[E]nglish -> Japanese'+#9#9+'[J]apanese -> English'+#9#9+'[M]ixed');
	try
		repeat
			write(#10#13+'Query type: ');
			readln(UserPromptDirection);
			StartPrompt(UserPromptDirection);
			writeln; 
		until UserPromptDirection in ['e','E','j','J','m','M'];
	except on E:Exception do 
		writeln('A wild '+E.ClassName+' has appeared.'+#13#10+'It was lured by '+E.Message+'.');
	end;
	CreateFooter('-',length(ProgramName)+9*2);	
end.
