BINNAME_MAIN = MilhaoDosGames

SRC_MAIN = main.c csv_handler.c pergunta.c

RAYLIB_FLAGS = -I"C:\raylib\raylib\src" -L"C:\raylib\raylib\src" -lraylib -lopengl32 -lgdi32 -lwinmm

all: $(BINNAME_MAIN)

$(BINNAME_MAIN):
	gcc $(SRC_MAIN) -o $(BINNAME_MAIN) $(RAYLIB_FLAGS)

clean:
	rm -f $(BINNAME_MAIN)

run:
	./$(BINNAME_MAIN)


