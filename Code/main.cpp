#include <stdio.h>
#include <string.h>
#include "naglow.h"

extern "C" 
{
	#include"./sdl-2.0.7/include/SDL.h"
	#include"./sdl-2.0.7/include/SDL_main.h"
}



//POCZATEK UDOSTEPNIONEGO SZABLONU
//
//Narysowanie napisu txt na powierzchni screen, zaczynajac od punktu (x, y)
//Charset to bitmapa 128x128 zawierajaca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) 
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


//Narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
//(x, y) to punkt srodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int rozmiarPlanszy, int i)
{
	int marginesPoziomy = 24 + ((SCREEN_WIDTH - ((rozmiarPlanszy - 1) * WIELKOSCPOLA)) / 2);
	int marginesPionowy = 24 + ((SCREEN_HEIGHT - (rozmiarPlanszy * WIELKOSCPOLA)) / 2);
	int x, y;
	y = (i / rozmiarPlanszy) + 1;
	x = i - ((y - 1) * rozmiarPlanszy);
	y = (y * WIELKOSCPOLA) + marginesPionowy;
	x = (x * WIELKOSCPOLA) + marginesPoziomy;

	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


//Rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) 
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
};


//Rysowanie linii o dlugosci 'l' w pionie (gdy dx = 0, dy = 1) 
//Badz poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) 
{
	for (int i = 0; i < l; i++) 
	{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

//Rysowanie prostokata o dlugosci bokow 'l' i 'k'
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) 
{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};
//
//KONIEC UDOSTEPNIONEGO SZABLONU



void liczCzasIFPS(int *t1, int *t2, double *delta, double *worldTime, double *fps, int *frames, double *fpsTimer)
{
	*t2 = SDL_GetTicks();
	*delta = (*t2 - *t1) * 0.001;
	*t1 = *t2;
	*worldTime += *delta;

	*fpsTimer += *delta;
	if (*fpsTimer > 0.5)
	{
		*fps = *frames * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
	}
}


//Inizjalizacja(w szablonie w "main")
void inicjalizacja(int *quit, SDL_Surface **screen, SDL_Texture **scrtex, SDL_Window **window, SDL_Renderer **renderer)
{
	int rc;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		*quit = TAK;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		window, renderer);
	if (rc != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		*quit = TAK;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(*window, "Sokoban Daniel Kulas 168813");

	*screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0, 0, 0, 0);

	*scrtex = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	//Wylaczenie kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
}


void wczytanieGry(int *quit, int *wygrana, double *worldTime, plansza_t **plansza, int *rozmiarPlanszy)
{
	*worldTime = 0;
	*wygrana = NIE;
	FILE *wczytPlik = fopen("plansza.txt", "r");
	if (wczytPlik == NULL)
		*quit = TAK;

	fscanf(wczytPlik, "%d", rozmiarPlanszy);
	zwolnijPamiec(plansza);
	przydzielPamiec(quit, plansza, rozmiarPlanszy);

	//Wczytanie pol
	fscanf(wczytPlik, "\n");
	for (int i = 0; i < (*rozmiarPlanszy) * (*rozmiarPlanszy); i++)
		fscanf(wczytPlik, "%1d", &((*plansza + i)->obiekt));

	//Wczytanie pol docelowych
	fscanf(wczytPlik, "\n");
	for (int i = 0; i < (*rozmiarPlanszy) * (*rozmiarPlanszy); i++)
		fscanf(wczytPlik, "%1d", &((*plansza + i)->czyDocelowe));

	fclose(wczytPlik);
}


void zwolnijPowierzchnie(int *quit, SDL_Surface **screen, SDL_Surface **charset, SDL_Texture **scrtex, SDL_Window **window, SDL_Renderer **renderer)
{
	SDL_FreeSurface(*charset);
	SDL_FreeSurface(*screen);
	SDL_DestroyTexture(*scrtex);
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);
	*quit = TAK;
	SDL_Quit();
}


void przydzielPamiec(int *quit, plansza_t **plansza, int *rozmiarPlanszy)
{
	*plansza = (plansza_t*)malloc((*rozmiarPlanszy) * (*rozmiarPlanszy) * sizeof(plansza_t));
	if (plansza == NULL)
	{
		*quit = TAK;
	}
};


void zwolnijPamiec(plansza_t **plansza)
{
	free(*plansza);
	plansza = NULL;
};


void poruszanie(int left, int right, int up, int down, int rozmiarPlanszy, plansza_t *plansza)
{
	int pozycjaPostaci, poleNastepne, poleDrugieNastepne;
	if (left == WLEWO || right == WPRAWO)
	{
		pozycjaPostaci = pozycjaGracza(rozmiarPlanszy, plansza);
		poleNastepne = jakiObiektWPolu(rozmiarPlanszy, plansza, pozycjaPostaci + left + right);
		poleDrugieNastepne = jakiObiektWPolu(rozmiarPlanszy, plansza, pozycjaPostaci + (2 * left) + (2 * right));
		if (!(((pozycjaPostaci - 1) % rozmiarPlanszy == 0 && left == WLEWO) || ((pozycjaPostaci + 2) % rozmiarPlanszy == 0 && right == WPRAWO)))
		{
			if (poleNastepne == POLE_PUSTE)
			{
				(plansza + pozycjaPostaci + left + right)->obiekt = POLE_POSTAC;
				(plansza + pozycjaPostaci)->obiekt = POLE_PUSTE;
			}
			else if (poleNastepne == POLE_SKRZYNKA && poleDrugieNastepne == POLE_PUSTE)
			{
				(plansza + pozycjaPostaci + (2 * left) + (2 * right))->obiekt = POLE_SKRZYNKA;
				(plansza + pozycjaPostaci + left + right)->obiekt = POLE_POSTAC;
				(plansza + pozycjaPostaci)->obiekt = POLE_PUSTE;
			}
		}
	}

	if (up == WGORE || down == WDOL)
	{
		pozycjaPostaci = pozycjaGracza(rozmiarPlanszy, plansza);
		poleNastepne = jakiObiektWPolu(rozmiarPlanszy, plansza, pozycjaPostaci + (rozmiarPlanszy * up) + (rozmiarPlanszy * down));
		poleDrugieNastepne = jakiObiektWPolu(rozmiarPlanszy, plansza, pozycjaPostaci + (rozmiarPlanszy * up * 2) + (rozmiarPlanszy * down * 2));
		if (!((pozycjaPostaci < (2 * rozmiarPlanszy) && up == WGORE) || (pozycjaPostaci > (((rozmiarPlanszy * rozmiarPlanszy)-1)-(2 * rozmiarPlanszy)) && down == WDOL)))
		{
			if (poleNastepne == POLE_PUSTE)
			{
				(plansza + pozycjaPostaci + (rozmiarPlanszy * up) + (rozmiarPlanszy * down))->obiekt = POLE_POSTAC;
				(plansza + pozycjaPostaci)->obiekt = POLE_PUSTE;
			}
			else if (poleNastepne == POLE_SKRZYNKA && poleDrugieNastepne == POLE_PUSTE)
			{
				(plansza + pozycjaPostaci + (rozmiarPlanszy * up * 2) + (rozmiarPlanszy * down * 2))->obiekt = POLE_SKRZYNKA;
				(plansza + pozycjaPostaci + (rozmiarPlanszy * up) + (rozmiarPlanszy * down))->obiekt = POLE_POSTAC;
				(plansza + pozycjaPostaci)->obiekt = POLE_PUSTE;
			}
		}
	}
}


void wyswietlanie(SDL_Surface **screen, SDL_Surface **wall, SDL_Surface **box, SDL_Surface **boxc, SDL_Surface **texture, SDL_Surface **texturec, SDL_Surface **character, 
			plansza_t *plansza, int rozmiarPlanszy)
{
	//Rysuj sciany
	for (int i = 0; i < rozmiarPlanszy * rozmiarPlanszy; i++)
	{
		if ((plansza + i)->obiekt == POLE_SCIANA)
		{
			DrawSurface(*screen, *wall, rozmiarPlanszy, i);
		}
		else if ((plansza + i)->obiekt == POLE_PUSTE)
		{
			if ((plansza + i)->czyDocelowe == TAK)
			{
				DrawSurface(*screen, *texturec, rozmiarPlanszy, i);
			}
			else
			{
				DrawSurface(*screen, *texture, rozmiarPlanszy, i);
			}
		}
		else if ((plansza + i)->obiekt == POLE_SKRZYNKA)
		{
			if ((plansza + i)->czyDocelowe == TAK)
			{
				DrawSurface(*screen, *boxc, rozmiarPlanszy, i);
			}
			else
			{
				DrawSurface(*screen, *box, rozmiarPlanszy, i);
			}
		}
		else if ((plansza + i)->obiekt == POLE_DOCELOWE)
		{
			DrawSurface(*screen, *texturec, rozmiarPlanszy, i);
		}
		else if ((plansza + i)->obiekt == POLE_POSTAC)
		{
			DrawSurface(*screen, *character, rozmiarPlanszy, i);
		}
	}
}


//Funkcja zwraca pole na ktorym jest gracz(jednowymiarowej tablicy)
int pozycjaGracza(int rozmiarPlanszy, plansza_t *plansza)
{
	for (int a = 0; a < rozmiarPlanszy * rozmiarPlanszy; a++)
	{
		if ((plansza + a)->obiekt == POLE_POSTAC)
		{
			return a;
			break;
		}
	}
	return -1;
}


//Funckja zwraca rodzaj obiektu jaki jest we wskazanym polu
int jakiObiektWPolu(int rozmiarPlanszy, plansza_t *plansza, int a)
{
	int swap;
	swap = (plansza + a)->obiekt;
	return swap;
}


//Funkcja zwraca 1(TAK), gdy wykryto wygrana, przeciwnie 0(NIE)
int czyWygrana(plansza_t *plansza, int rozmiarPlanszy)
{
	int wygrana = TAK;
	for (int i = 0; i < rozmiarPlanszy * rozmiarPlanszy; i++)
	{
		if ((plansza + i)->obiekt == POLE_SKRZYNKA)
		{
			if ((plansza + i)->czyDocelowe == NIE)
				wygrana = NIE;
		}
	}
	return wygrana;
}



#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv)
{
	//Inicjalizacje
	int quit = 0, rozmiarPlanszy = ROZMIARPODSTAWOWY, left = 0, right = 0, up = 0, down = 0, wygrana = 0, frames = 0, t1, t2;
	double delta, worldTime = 0, fpsTimer = 0, fps = 0;
	struct plansza_t *plansza = (plansza_t*)malloc(rozmiarPlanszy * rozmiarPlanszy * sizeof(plansza_t));

	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *wall, *box, *boxc, *texture, *texturec, *character;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	
	inicjalizacja(&quit, &screen, &scrtex, &window, &renderer);
	wczytanieGry(&quit, &wygrana, &worldTime, &plansza, &rozmiarPlanszy);

	//Wczytanie obrazkow
	charset = SDL_LoadBMP("images/cs8x8.bmp");
	SDL_SetColorKey(charset, true, 0x000000);
	wall = SDL_LoadBMP("images/wall.bmp");
	box = SDL_LoadBMP("images/box.bmp");
	boxc = SDL_LoadBMP("images/boxc.bmp");
	texture = SDL_LoadBMP("images/texture.bmp");
	texturec = SDL_LoadBMP("images/texturec.bmp");
	character = SDL_LoadBMP("images/character.bmp");

	//Jeœli wczytanie ktoregos z obrazkow nie powiodlo sie:
	if (wall == NULL || box == NULL || boxc == NULL || texture == NULL || texturec == NULL || character == NULL)
		zwolnijPowierzchnie(&quit, &screen, &charset, &scrtex, &window, &renderer);
	if(quit == 1)
		SDL_Quit();

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	int szary = SDL_MapRGB(screen->format, 128, 128, 128);
	int czerwony = SDL_MapRGB(screen->format, 255, 0, 0);
	int zolty = SDL_MapRGB(screen->format, 210, 210, 110);

	t1 = SDL_GetTicks();
	while (quit != 1)
	{
		//Wyswielanie planszy i akcja
		SDL_FillRect(screen, NULL, zolty);
		wyswietlanie(&screen, &wall, &box, &boxc, &texture, &texturec, &character, plansza, rozmiarPlanszy);
		if(czyWygrana(plansza, rozmiarPlanszy) == TAK)
		{
			//Jesli wykryto wygrana:
			wygrana = 1;
			DrawRectangle(screen, 4, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 8, 100, bialy, czerwony);
			sprintf(text, "Wygrales!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20 + SCREEN_HEIGHT / 2, text, charset);
			left = 0, right = 0, up = 0, down = 0;
		}

		//Wyswietlanie legendy
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 72, bialy, szary);
			sprintf(text, "Sokoban, Daniel Kulas 168813");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		if (wygrana != 1)
		{
			sprintf(text, "Czas trwania: %.1lf s   %.0lf klatek/s", worldTime, fps);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
			sprintf(text, "Esc - wyjscie, Strzalki: poruszanie postacia, N - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
		}

		liczCzasIFPS(&t1, &t2, &delta, &worldTime, &fps, &frames, &fpsTimer);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		//Obsluga zdarzen, akcja
		poruszanie(left, right, up, down, rozmiarPlanszy, plansza);
		left = 0, right = 0, up = 0, down = 0;
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = TAK;
					if (event.key.keysym.sym == SDLK_n) wczytanieGry(&quit, &wygrana, &worldTime, &plansza, &rozmiarPlanszy);
					if (event.key.keysym.sym == SDLK_LEFT) left = WLEWO;
					if (event.key.keysym.sym == SDLK_RIGHT) right = WPRAWO;
					if (event.key.keysym.sym == SDLK_UP) up = WGORE;
					if (event.key.keysym.sym == SDLK_DOWN) down = WDOL;
					break;
				case SDL_QUIT:
					quit = TAK;
					break;
			};
		};
		frames++;
	};

	//Zwolnienie powierzchni i wyjscie
	zwolnijPowierzchnie(&quit, &screen, &charset, &scrtex, &window, &renderer);
	SDL_Quit();
	return 0;
};
