#include <windows.h>
#include "resource.h"
#include <math.h>

HPEN hpen_paint,hpen_del;

void CopyAudioData(unsigned char* pbAudioBuffer, PFLOAT pfRealBuffer,PFLOAT pfImagBuffer, int iNumBytes); 
void GetResult(PFLOAT pfRealBuffer, PFLOAT pfImagBuffer, PFLOAT pfResBuffer,int iNum);
void FFT(PFLOAT pfReal, PFLOAT pfImag, unsigned int iSamples);
void DrawFFTBuffer(HDC hdc,RECT rect,RECT* del,POINT *apt ,PFLOAT pfPaintBuffer,int iStart,int iOffset,int iMaxArrayVal,int iFreq,int);




void voice(long,long);
void enregistrer(int,HDC);


char enrego[20];
char computing[40];
char resultat[30];



int* buffet(char*);


void moyenne(int[],int[]);


long wichone(int[],int[]);

long mastervoice(char*,char*,int);

int diction();

void intarray(int*);

int petitesse(long,long,long,long,long,long,long,long);


void demelage();

