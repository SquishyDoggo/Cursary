program jaVocTrainer;
{$MODE OBJFPC}

uses SysUtils, StrUtils;

const dictName = 'jadict';

var 	promptType: Integer;
	JaEn, EnJa, Mixd: String;	
	ornament, header: String;

procedure promptVocabulary(dictName: String; promptType: Integer);
var 	tfOut: TextFile;
	i: Integer;
	ALine: String;
	lang1, lang2: String;
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
		if promptType = 1 then 
		begin
			lang1 := trim(copy(aline,1,pos(vocDelim,aline)-1)); { en }
			lang2 := trim(copy(aline, pos(vocDelim,aline)+1,length(aline))); { ja }
		end

		else if promptType = 2 then 
		begin
			lang1 := trim(copy(aline, pos(vocDelim,aline)+1,length(aline))); { ja }
			lang2 := trim(copy(aline,1,pos(vocDelim,aline)-1)); { en }
		end

		else if promptType = 3 then
		begin
			Randomize;
		 	i := Random(2);

			if i = 0 then
			begin
				lang1 := trim(copy(aline,1,pos(vocDelim,aline)-1)); { en }
				lang2 := trim(copy(aline, pos(vocDelim,aline)+1,length(aline))); { ja }
			end

			else
			begin
				lang1 := trim(copy(aline, pos(vocDelim,aline)+1,length(aline))); { ja }
				lang2 := trim(copy(aline,1,pos(vocDelim,aline)-1)); { en }
			end;
		end;

		write(lang1+': '); inc(NumVocs);
		readln(trans);
		trans:=trim(trans);
		if CompareText(trans,lang2)=0 then 
		begin
			writeln('Correct!');
			inc(CorrectVocs);
		end
		else writeln('Wrong, correct answer: '+lang2);
	end;
		close(tfOut);
		infoText[pos('x',infoText)] := IntToStr(CorrectVocs)[1];
		infoText[pos('y',infoText)] := IntToStr(NumVocs)[1];
		infoOrnament := DupeString('-',length(infoText)+4);
	
		writeln(infoOrnament);
		writeln('  '+infoText);
		writeln(infoOrnament);
end;
	
begin
	header := 'EnJa: The Japanese Vocabulary Trainer'; 	
	EnJa := 'English  -> Japanese';
	JaEn := 'English  <- Japanese';
	Mixd := 'English <-> Japanese';
	ornament := DupeString('-',length(header)+4);

	writeln; writeln(ornament);
	writeln('   '+header+'  ');
	writeln(ornament);
	while true do
	begin
		writeln('[1] '+EnJa +#13#10 +'[2] '+JaEn +#13#10+ '[3] '+Mixd);

		try
			readln(promptType);
			if (promptType in [1..3]) then promptVocabulary(dictName,promptType)
			else writeln('Option ' + IntToStr(promptType) + ' does not exist!');
		
		except
			on E: Exception do writeln('A wild '+E.Classname+' appeared!'+#13#10+'Try preventing '+E.Message+' next time.');
		end;	
		writeln;
	end;
end.
