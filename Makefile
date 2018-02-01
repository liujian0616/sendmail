CC = gcc 
CCC = g++
INC = -I/usr/local/include
CFLAGS = -g
LDFLAGS = -lssl

APPNAME = sendmail
OBJ = $(patsubst %.c, obj/%.o, $(wildcard *.c)) $(patsubst %.cpp, obj/%.o, $(wildcard *.cpp))

${APPNAME} : ${OBJ}
	${CCC} ${OBJ} ${LDFLAGS} -o ${APPNAME}

obj/%.o :%.c
	${CC} ${CFLAGS} -c -o $@ $< ${INC}

obj/%.o :%.cpp
	${CCC} ${CFLAGS} -c -o $@ $< ${INC}

clean:
	rm -rf ${OBJ}
