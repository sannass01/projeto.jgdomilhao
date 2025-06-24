BINNAME = MilhaoDosGames

SRC = *.c

all:
	gcc $(SRC) -o $(BINNAME) -I"C:\raylib\raylib\src" -L"C:\raylib\raylib\src" -lraylib -lopengl32 -lgdi32 -lwinmm

clean:
	rm -f $(BINNAME)

run:
	./$(BINNAME)

