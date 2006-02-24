zip -9 %1 COPYING *.cpp *.h *.c *.rc *.ds? *.sln *.vcproj unused/* zlib/*
kzip b%1 COPYING *.cpp *.h *.c *.rc *.ds? *.sln *.vcproj
zipmix %1 b%1
del b%1