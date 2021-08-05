program jaVocTrainer;
{$MODE OBJFPC}

uses SysUtils, StrUtils;

const dictName = 'jadict';

var 	promptType: Integer;
	JaGe, GeJa, Mixd: String;	
	ornament, header: String;

procedure promptVocabulary(dictName: String; promptType: Integer);
var 	tfOut: TextFile;
	i: Integer;
	ALine: String;
	ger, ja: String;
	vocDelim: Char;
	trans: String;
	NumVocs, CorrectVocs: Integer;
	infoText, infoOrnament: String;


begin
	infoText := 'You have got x out of y words correct!';
	vocDelim := '-';
	AssignFile(tfOut,dictName);
	reset(tfOut);
	NumVocs := 0; CorrectVocs := 0;

	while not eof(tfOut) do
	begin
		readln(tfOut,ALine);
		{ - partitioning in ger und Ja - }
		ger := trim(copy(aline,1,pos(vocDelim,aline)-1));
		ja := trim(copy(aline, pos(vocDelim,aline)+1,length(aline)));
		
		{ - german -> japanese - }
		if promptType = 1 then
		begin
			write(ger+':');
			inc(NumVocs);
			readln(trans);
			trans:=trim(trans);
			if CompareText(trans,ja)=0 then 
			begin
				writeln('Correct!');
				inc(CorrectVocs);
			end
			else writeln('Wrong, correct answer: '+ja);
		end
			
		{ - japanisch -> deutsch - }
		else if promptType = 2 then
		begin
			write(ja+':');
			inc(NumVocs);
			readln(trans);
			trans := trim(trans);
			if CompareText(trans,ger)=0 then 
			begin
				writeln('Correct!');
				inc(CorrectVocs);
			end
			else writeln('Wrong, correct answer: '+ger);
		end
		
		{ - japanisch <-> deutsch - }
		else if promptType = 3 then
		begin
			Randomize;
		 	i := Random(2);

			if i = 0 then
			begin
				write(ger+':');
				inc(NumVocs);
				readln(trans);
				trans := trim(trans);
				if CompareText(trans,ja)=0 then 
				begin
					writeln('Correct!');
					inc(CorrectVocs);
				end
				else writeln('Wrong, correct answer: '+ja);
			end
			else if i = 1 then
			begin 
				write(ja+':');
				inc(NumVocs);
				readln(trans);
				trans := trim(trans);
				if CompareText(trans,ger)=0 then 
				begin 
					writeln('Correct!');
					inc(CorrectVocs);
				end
				else writeln('Wrong, correct answer: '+ger);
			end;
		end;
	end;
	close(tfOut);
	infoText[pos('x',infoText)] := IntToStr(CorrectVocs)[1];
	infoText[pos('y',infoText)] := IntToStr(NumVocs)[1];
	infoOrnament := DupeString('-',length(infoText)+4);

	writeln; writeln(infoOrnament);
	writeln('  '+infoText);
	writeln(infoOrnament);
end;
	
begin
	header := 'Japanese Vocabulary Trainer'; 	
	GeJa := 'German    -> Japanese';
	JaGe := 'Japanese  -> German';
	Mixd := 'Japanese <-> German';
	ornament := DupeString('-',length(header)+4);

	writeln; writeln(ornament);
	writeln('  '+header);
	writeln(ornament);
	writeln('[1] '+GeJa +#10#13 +'[2] '+JaGe +#10#13+ '[3] '+Mixd);
	readln(promptType);

	if promptType = 1 then
		begin
			writeln(GeJa + ' selected.');
			promptVocabulary(dictName, promptType);
		end
	else if promptType = 2 then
		begin
			writeln(JaGe + ' selected.');
			promptVocabulary(dictName, promptType);
		end
	else if promptType = 3 then
		begin
			writeln(Mixd + ' selected.');
			promptVocabulary(dictName, promptType);
		end
	else
		writeln('Unknown selection!');

	writeln;
end.
