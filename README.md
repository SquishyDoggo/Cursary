# 📔Cursary
Your friendly neighborhood ncurses vocabulary trainer for english and japanese.

## 💻Installation
Clone the repository and run `make` inside the project directory.

## Usage ##
__Cursary__ scans _dicts/enja.txt_ for vocabulary, where words may be added in accordance with the [notation](#dictionary-file) used in the sample file.
In the options menu the user is presented 3 query types:
1. __English -> Japanese__: Print english word and query the corresponding japanese word.
2. __Japanese -> English__: Print japanese word and query the corresponding english word.
3. __English <-> Japanese__: A random mix of 1 and 2.

## Showcase ##
![Cursary](demo/cursary.gif)

## 📁 Dictionary File
Vocabulary inside the dictionary file is stored in 3-tuples and follows a preset structure.\
The first line contains the english word. If multiple english words point to the same japanese word they may be separated by semicolons.\
The japanese counterpart is stored in the line below. If it features kanji-characters, then the third line contains furigana.
If it exists only of kana, the third line is left empty.
