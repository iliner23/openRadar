# openRadar
This is open source reimplementation of Radar program.
Project staying in early stage of implementation.
# Current features in experimental branch:
1) It can read repertories and keys
3) There is navigation for repertory chapters and subchapters
4) It supports read information about symptoms, that include remedies and his authors
5) It allows use synonym links for jump into interesting symptom in repertory
6) There is search remedies by synonyms, words and roots
# Peculiarities
Original Radar program work with custom implementation of c-tree database, than modern c-tree can't read (raw data be able read with DrTree from c-tree software)
and elden graphics library which not compatible with modern operation systems like Windows 10.
I decide create a open source realization of this program with using Qt framework for GUI. OpenCtree is custom library that can read c-tree indexes with raw data. 
This data process in program modules placed into commonfunction directiory. 
Original program worked with ascii local codecs so my program decode this in unicode for working with Qt.
# TODO list for nearlier future:
1) Will finish with functionality of remedies research window
2) Make better code in window for choose chapters and subchapters, and in MDI main window
3) In Qt6 QTextCodec deprecated, thus need in future use anoter library for decode ascii in unicode.
4) Redraw original icon for prevent eventual author pretensions
# Compile and Launch
This implementation of course need original Radar files, especially places in "data" and "system" directories. Without it program don't be able to start.
Place directories upper one catalogue than executable file was placed. This code have tested on Qt 6.2 with Qmake and MSVS 2019 on Windows 10. 
In theory can work on Qt 5, but I don't often testing on tihs configuration. 
If you are testing this sourse code on another compilators/Qt version/operation system 
tell about bugs or problems that you catched in issues section or my email yaponomat23@gmail.com.
