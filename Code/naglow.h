#ifndef NAGLOW_H
#endif
#pragma once


#define SCREEN_WIDTH 512	
#define SCREEN_HEIGHT 576
#define WIELKOSCPOLA 48
#define ROZMIARPODSTAWOWY 9
#define POLE_PUSTE 0
#define POLE_SCIANA 1
#define POLE_SKRZYNKA 2
#define POLE_POSTAC 3
#define TAK 1
#define NIE 0
#define WLEWO -1
#define WPRAWO 1
#define WGORE -1
#define WDOL 1
#define POLE_DOCELOWE 1


struct plansza_t
{
	int obiekt;
	int czyDocelowe;
};

void przydzielPamiec(int *quit, plansza_t **plansza, int *rozmiarPlanszy);
void zwolnijPamiec(plansza_t **plansza);
int pozycjaGracza(int rozmiarPlanszy, plansza_t *plansza);
int jakiObiektWPolu(int rozmiarPlanszy, plansza_t *plansza, int a);
