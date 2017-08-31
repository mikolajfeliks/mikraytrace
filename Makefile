# . Makefile for mrtp

PNGVER  = 0.2.9
CFLAGS  = -Ipng++/png++-${PNGVER} -W -Wall -pedantic  # -O2 
CC      = g++

OBJ = main.o actors.o vector.o world.o camera.o color.o  \
    buffer.o parser.o parserentry.o parsertables.o  \
    texture.o utils.o


default: mrtp

mrtp: ${OBJ}
	$(CC) $(CFLAGS) ${OBJ} -lpng -lm -o mrtp

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

actors.o: actors.cpp actors.hpp
	$(CC) $(CFLAGS) -c actors.cpp -o actors.o

vector.o: vector.cpp vector.hpp
	$(CC) $(CFLAGS) -c vector.cpp -o vector.o

world.o: world.cpp world.hpp
	$(CC) $(CFLAGS) -c world.cpp -o world.o

camera.o: camera.cpp camera.hpp
	$(CC) $(CFLAGS) -c camera.cpp -o camera.o

color.o: color.cpp color.hpp
	$(CC) $(CFLAGS) -c color.cpp -o color.o

buffer.o: buffer.cpp buffer.hpp
	$(CC) $(CFLAGS) -c buffer.cpp -o buffer.o

parser.o: parser.cpp parser.hpp
	$(CC) $(CFLAGS) -c parser.cpp -o parser.o

parserentry.o: parserentry.cpp parser.hpp
	$(CC) $(CFLAGS) -c parserentry.cpp -o parserentry.o

parsertables.o: parsertables.cpp parser.hpp
	$(CC) $(CFLAGS) -c parsertables.cpp -o parsertables.o

texture.o: texture.cpp texture.hpp
	$(CC) $(CFLAGS) -c texture.cpp -o texture.o

utils.o: utils.cpp utils.hpp
	$(CC) $(CFLAGS) -c utils.cpp -o utils.o

clean:
	if [ -e mrtp ] ; then rm -v mrtp ; fi
	if [ -e  main.o         ]  ;  then  rm -v  main.o         ;  fi
	if [ -e  actors.o       ]  ;  then  rm -v  actors.o       ;  fi
	if [ -e  vector.o       ]  ;  then  rm -v  vector.o       ;  fi
	if [ -e  world.o        ]  ;  then  rm -v  world.o        ;  fi
	if [ -e  camera.o       ]  ;  then  rm -v  camera.o       ;  fi
	if [ -e  color.o        ]  ;  then  rm -v  color.o        ;  fi
	if [ -e  buffer.o       ]  ;  then  rm -v  buffer.o       ;  fi
	if [ -e  parser.o       ]  ;  then  rm -v  parser.o       ;  fi
	if [ -e  parserentry.o  ]  ;  then  rm -v  parserentry.o  ;  fi
	if [ -e  parsertables.o ]  ;  then  rm -v  parsertables.o ;  fi
	if [ -e  texture.o      ]  ;  then  rm -v  texture.o      ;  fi
	if [ -e  utils.o        ]  ;  then  rm -v  utils.o        ;  fi
