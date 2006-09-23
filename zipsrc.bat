zip -9 %1 COPYING Makefile *.cpp *.h *.c *.rc *.ds? *.sln *.vcproj unused/* zlib/*.c zlib/*.h
kzip b%1 COPYING Makefile *.cpp *.h *.c *.rc *.ds? *.sln *.vcproj unused/* zlib/*.c zlib/*.h
zipmix %1 b%1
del b%1