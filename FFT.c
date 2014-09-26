//FFT
// Jean-René Bédard
// 2002-03

#include "FFT.h"
#include <stdlib.h>
#include <stdio.h>
FILE *out;
FILE *file1;
FILE *clean;
FILE *cleaned;
FILE *com;

char buffx[10];
char buffy[10];

int compteur;
int recording;   // ca roule!
int swich,ok;
int control;

recording=0;	
compteur=0;
compute=0;


#define TWO_PI  6.283185

void CopyAudioData(unsigned char* pbAudioBuffer, PFLOAT pfRealBuffer,PFLOAT pfImagBuffer, int iNumBytes) 
{
 
 int i,n;
 short sAudioSample;	
  for (i=0,n=0; i< iNumBytes; i+=2,n++)
  { 
	// aligner les données, bon ordre
 	sAudioSample =  ((pbAudioBuffer[i+1] << 8) & 0xFF00)|
		             (pbAudioBuffer[i] & 0x00FF);
	pfImagBuffer[n] = 0;
	pfRealBuffer[n] = sAudioSample; // correct!!!! 16bit SampleValue
  }	
}


//------------------------------------------------------------
void DrawFFTBuffer(HDC hdc,RECT rect,RECT* del,POINT *apt ,PFLOAT pfPaintBuffer,int iStart,int iOffset,int iMaxArrayVal,int ifreq,int buffsize)
{
  
  HBRUSH hbrush;
 
  int retval;
  int i;
  char buff1[5],buff2[5],buff3[5],buff4[5];
 

  SetBkColor(hdc, RGB(0,0,0));
 
 //setting des valeurs de fréquences
 //delete zone ! effacer tout
  TextOut(hdc,(rect.right-rect.left)/2+3,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,"               ",10);
  TextOut(hdc,(rect.right-rect.left)/2-213,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,"               ",10);
  TextOut(hdc,(rect.right-rect.left)/2+180,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,"               ",10);
   
   
  // est-ce que ca enregistre damn it!!!?!?!?!??!
  TextOut(hdc,(rect.right-rect.left)/2+80,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+20,enrego,strlen(enrego));
  TextOut(hdc,(rect.right-rect.left)/2+80,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+40,computing,strlen(computing));
  TextOut(hdc,(rect.right-rect.left)/2-100,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+20,resultat,strlen(resultat));
  
  
  
  //delete du haut
  del = &rect;
  
  //retval = SetRect(del, 0, 0, 800, 300);
  hbrush = CreateSolidBrush(RGB(0, 0, 0));
  retval = FillRect(hdc, del, hbrush);
  retval = DeleteObject(hbrush);
  
  ifreq = (iOffset/2 + iStart)*(11025)/(buffsize/2); // freq du curseur principal
  
  itoa(ifreq,buff1,10);
  itoa(ifreq-1500,buff2,10);
  itoa(ifreq+1500,buff3,10);

  
  
 /////////////////////////////////////////////////////

  
   SetTextColor(hdc,RGB(0,255,0));
 
  

   for(i=0; i<iOffset ; i++)
   { 
    apt[i].x = (int)(rect.right-rect.left) * i / iOffset;
    
	if  (((i+iStart)<0) || ((i+iStart)>=iMaxArrayVal))
	apt[i].y = (int) ((rect.bottom-rect.top)-(rect.bottom-rect.top)/3); //il n'y a rien! 
	else
	{
		apt[i].y = (int) (((rect.bottom-rect.top)-(rect.bottom-rect.top)/3) * (1 - (pfPaintBuffer[i+iStart])/32768));   
	    
		
		if( recording == 1 && ((i+ iStart )*(11025)/(buffsize/2)) > 250 && ((i+ iStart )*(11025)/(buffsize/2)) < 3400)
		voice((i+ iStart )*(11025)/(buffsize/2),apt[i].y); /// BIG deAL
		
		
		
		if(apt[i].y<150)
		{
			itoa((i+ iStart )*(11025)/(buffsize/2),buff4,10);
		
			if(apt[i].y> 20)
			TextOut(hdc,apt[i].x,apt[i].y-10,buff4,strlen(buff4));
			else
			TextOut(hdc,apt[i].x,10,buff4,strlen(buff4));
		}
	}
   }
	
   if(recording==1) 
   {
	   voice(-375,-375); /// finit l'instant
	   compteur++;
   }	
	 
  
  Polyline (hdc, apt, iOffset) ;
  SelectObject(hdc, GetStockObject(WHITE_PEN)); // ligne verticale centrale
  MoveToEx(hdc,(rect.right-rect.left)/2 ,0,NULL);
  LineTo(hdc,(rect.right-rect.left)/2  ,(rect.bottom-rect.top));
	
  
  MoveToEx(hdc,((rect.right-rect.left)/2)-220 ,(rect.bottom-rect.top)/8,NULL);
  LineTo(hdc,((rect.right-rect.left)/2)+220  ,(rect.bottom-rect.top)/8);

  MoveToEx(hdc,((rect.right-rect.left)/2)-220 ,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8,NULL);
  LineTo(hdc,((rect.right-rect.left)/2)+220  ,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8);

  //vertical !
  MoveToEx(hdc,((rect.right-rect.left)/2)-220 ,(rect.bottom-rect.top)/8,NULL);
  LineTo(hdc,((rect.right-rect.left)/2)-220  ,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8);

  //vertical !
  MoveToEx(hdc,((rect.right-rect.left)/2)+220 ,(rect.bottom-rect.top)/8,NULL);
  LineTo(hdc,((rect.right-rect.left)/2)+220  ,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8);
	
  
  //valeur standards +1500 -1500 carte brouilleur
  SetTextColor(hdc,RGB(255,255,0));
  TextOut(hdc,(rect.right-rect.left)/2-213,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,buff2,strlen(buff2));
  TextOut(hdc,(rect.right-rect.left)/2+180,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,buff3,strlen(buff3));
  TextOut(hdc,(rect.right-rect.left)/2+3,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8-19,buff1,strlen(buff1));

  
  // ca enregistre ??!?!
  SetTextColor(hdc,RGB(255,0,0));
  TextOut(hdc,(rect.right-rect.left)/2+80,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+20,enrego,strlen(enrego));
  SetTextColor(hdc,RGB(0,255,0));
  TextOut(hdc,(rect.right-rect.left)/2+80,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+40,computing,strlen(computing));
  TextOut(hdc,(rect.right-rect.left)/2-100,(rect.bottom-rect.top)-(rect.bottom-rect.top)/8+40,resultat,strlen(resultat));
   
}


//------------------------------------------------------------
// Amplidude en log <-> Resultat
//------------------------------------------------------------

void GetResult(PFLOAT pfRealBuffer, PFLOAT pfImagBuffer, PFLOAT pfResBuffer,int iNum)
{
	// abs(z) = sqrt(Re(z)^2+Im(z)^2);
	// I needed fast scaling, so I placed a /10 in here
	// I will have a control for this plus log Scale
	
	int i;
	for (i=0;i< iNum;i++)
	{
		pfResBuffer[i] = (float)sqrt(pfRealBuffer[i]*pfRealBuffer[i]+
			                  pfImagBuffer[i]*pfImagBuffer[i]) / 10;
	}
}

//-------------------------------------------------------
// Bit Reversing 
//-------------------------------------------------------

unsigned int iBitReversing(unsigned int iValue, unsigned int iNumberOfBits)
{
   unsigned int i,iReversedValue=0;
    
   for (i=0; i<iNumberOfBits; i++)
   {
        iReversedValue <<= 1;  // shift iValue with one to the left
   	iReversedValue = (iReversedValue ) | (iValue & 1);  //Set lsb of iReversedValue with lsb of iValue
        iValue >>= 1;  // shift iValue
   }  
   return iReversedValue;
}

//-----------------------------------
// Nombre de bits pour une valeur
// ex. 8 a besoin de 3 bits (2^3=8)!; 
//---------------------------------------
unsigned int iGetNumberOfBits(unsigned int iValue)
{
   int i;
   // iValue ^2  toujours !!!!   
   for (i=1; i<=32; i++)
   {
   	if (iValue & (1<<i)) return i;
   }
   return 0;

}

//---------------------------------------------------------
// FFT transformation
//---------------------------------------------------------
void FFT(PFLOAT pfReal, PFLOAT pfImag, unsigned int iNumOfSamples)
{
	unsigned int i,M;
	unsigned int iCol = 1; //start premiere colonne
	unsigned int iNoS2 = iNumOfSamples / 2;
    unsigned int iKnotNumber = 0, iKnotIndex;
    unsigned int iNumberOfBits =0;
    double  dExpValue,dTmpReal, dTmpImag, dCos, dSin, dArg;

	// Formule Xl(k) = X(l-1)(k) + W^P * X(l-1)(k + N/2^l)
	// index von k mit lambda bits = Spalten NxN Matrix mit N=2^lamda
	// i Col =  Column l
	// iKnot =Knot k
	// N = Number of Samples
	// Distance between two dual Knoten N/2^l   = N<<l 
	// p shift of M = (lambda-l) to the right the do BitReversing
	       
        iNumberOfBits = iGetNumberOfBits(iNumOfSamples);
        if(!iNumberOfBits) exit(-1);
        

        for (iCol=1;iCol<=iNumberOfBits;iCol++)
        {
            
            do {
        	for (i=1; i<= iNoS2; i++)
        	{
        		M = (iKnotNumber>>(iNumberOfBits - iCol));
        		dExpValue = (double) iBitReversing(M,iNumberOfBits); 
        		// ---> Nombres complexes calcul
        		  dArg = TWO_PI * dExpValue / iNumOfSamples;
        		  dCos = cos (dArg);
        		  dSin = sin (dArg);
        		  dTmpReal = pfReal[iKnotNumber+iNoS2]* dCos + pfImag[iKnotNumber+iNoS2] * dSin;
        		  dTmpImag = pfImag[iKnotNumber+iNoS2]* dCos - pfReal[iKnotNumber+iNoS2] * dSin;
        		  pfReal[iKnotNumber+iNoS2] = (float)(pfReal[iKnotNumber] - dTmpReal);
        		  pfImag[iKnotNumber+iNoS2] = (float)(pfImag[iKnotNumber] - dTmpImag);
        		  pfReal[iKnotNumber] = (float)(pfReal[iKnotNumber] + dTmpReal);
        		  pfImag[iKnotNumber] = (float)(pfImag[iKnotNumber] + dTmpImag);
        		// --> c'était les Nombres complexes!  
        		iKnotNumber++;
        	}
        	iKnotNumber += iNoS2;
            }while (iKnotNumber < iNumOfSamples);
            iNoS2/=2;
            iKnotNumber=0;
        }
        // BitReversing
        for (iKnotNumber=0;iKnotNumber<iNumOfSamples;iKnotNumber++)
        {
        	iKnotIndex = iBitReversing(iKnotNumber, iNumberOfBits);
        	if (iKnotIndex > iKnotNumber)
        	{
        		dTmpReal = pfReal[iKnotNumber];
        		dTmpImag = pfImag[iKnotNumber];
        		pfReal[iKnotNumber] = pfReal[iKnotIndex];
        		pfImag[iKnotNumber] = pfImag[iKnotIndex];
        		pfReal[iKnotIndex] = (float)dTmpReal;
        		pfImag[iKnotIndex] = (float)dTmpImag;
       		}
        }
}



// sortie de l'extrait vers fichier
void voice(long vx,long vy)
{
	//vy = vy/4;
	
	if(vx==258 && vy < 270 || ok==1)
	{
		swich=1;
		ok=1;
	}
	
	
	if(swich==1)
	{
		if(vy == -375)
		vy = -375;
		else if(vy<0)
		vy = -9;

		//vx=vx;
		

		ltoa(0-(vy-291),buffy,10);
		strcat(buffy,":");
		fputs(buffy, out);
	
		if(compteur >= 30)
		{
		compteur=0;
		demelage();
		}
	}
}



//controle de l'enregistrement 
void enregistrer(int begin,HDC hdc)
{    
	if(recording == 0)
	{
		out = fopen("voix.voi", "wb");
		recording=1;
		compute=1;

		strcpy(computing,"                   ");
		strcpy(resultat,"     ");
		strcpy(enrego,"ca enregistre!");
	}
}



// demeler le control de reconnaissance
void demelage()
{
	int wow;
	fclose (out);
	swich=0;
	ok=0;
	recording=0;
	strcpy(enrego,"                   ");
		
	if(compute==1)
	{
		com = fopen("com.txt","wb");
	
		wow=diction();
		itoa(wow,resultat,10);
		fputs(resultat,com);
	
		//strcpy(computing,"brouillage");
	
	
		fclose(com);
	}
	
	compute=0;

}




// écart entre le test et la normale   DICTIONNAIRE
int init_mot()
{
	long mots[10];
	long init[10];
	int word;

	mots[1] = mastervoice("voix.voi","brouillage.voi",1);
	mots[2] = mastervoice("voix.voi","agenda.voi",2);
	mots[3] = mastervoice("voix.voi","connection.voi",3);

	//mots[3] = mastervoice("voix.voi","moy3.voi");
	//mots[4] = mastervoice("voix.voi","moy4.voi");
	//mots[5] = mastervoice("voix.voi","moy5.voi");
	//mots[6] = mastervoice("voix.voi","moy6.voi");
	//mots[7] = mastervoice("voix.voi","moy7.voi");
	//mots[8] = mastervoice("voix.voi","moy8.voi");
	
	word = petitesse(mots[1],mots[2],mots[3],99999,99999,99999,99999,99999);

	return word;
}



// écart entre le test et la normale   DICTIONNAIRE
int diction()
{
	long mots[10];
	long init[10];
	int word;

	mots[1] = mastervoice("voix.voi","moy1.voi",1);
	mots[2] = mastervoice("voix.voi","moy2.voi",2);
	mots[3] = mastervoice("voix.voi","moy3.voi",3);
	mots[4] = mastervoice("voix.voi","moy4.voi",4);
	mots[5] = mastervoice("voix.voi","moy5.voi",5);
	mots[6] = mastervoice("voix.voi","moy6.voi",6);
	mots[7] = mastervoice("voix.voi","moy7.voi",7);
	mots[8] = mastervoice("voix.voi","moy8.voi",8);
	
	word = petitesse(mots[1],mots[2],mots[3],mots[4],mots[5],mots[6],mots[7],mots[8]);

	return word;
}





// controle de la reconnaissance vocale
long mastervoice(char*er,char*rt,int num) 
{
	int* megaint;
	int* otherint;
	long result;
	int jungle[889];
	int boogie[889];
	int r;

	megaint = buffet(er);
	for(r=0;r< 730;r++)
	{
		jungle[r]= *(megaint++);
	}
	
	otherint = buffet(rt);
	for(r=0;r< 730;r++)
	{
		boogie[r]= *(otherint++);
	}

	result = wichone(jungle,boogie);
		
	if(num==7)
	moyenne(jungle,boogie);
	
	return result;
}



//convertir le fichier en tableau de caractères
int* buffet(char* filename)
{
	long lSize;
	char* buffer; 
    int block[1000];
	int* tempbuff;
	char* token;
	char seps[] = ":";
	int cur_token = 0;
	
	file1 = fopen(filename, "r");    // fichier temporaire  référence ou dernier rec
	
	if (file1 !=NULL)
	{
		fseek (file1 , 0 , SEEK_END);
		lSize = ftell (file1);
		rewind (file1);

		buffer = (char*) malloc(lSize);
		fread(buffer,1,lSize-4,file1);
 
		fclose (file1);
		
	
		if(lSize > 2000)					 // il faut au moins 1/25 sec de son
		{
			tempbuff = (int*)malloc(lSize);
			token = (char*)malloc(lSize);

			token = strtok(buffer, seps);
	
			while(cur_token < 730)//token != NULL)  // regarder le size du buffer de cars  :( bug area!
			{
   				block[cur_token] = atoi(token);
				cur_token++;
				token = strtok(NULL, seps);  
			}
		}
	}
	tempbuff = block;
	return tempbuff;
}




// choisir le bon son, retourner la différence des amplitudes
long wichone(int son[], int dic[])
{
	int q;
	long diff;
	long semi;
	int premier, deuxieme;

	diff=0;
	for(q=0;q<730;q++)				
	{
		premier = dic[q];
		deuxieme = son[q];
		
		semi = abs(premier - deuxieme); 
		diff += semi;
	}
	return diff;
}



// faire la moyenne de fichier d'un son
void moyenne(int son[], int dic[])  
{
	int q;
	int semi;
	int premier, deuxieme;
	char buffc[5];
	out = fopen("moyun1.voi", "wb");
	
	for(q=0;q<730;q++)				
	{
		premier = dic[q];
		deuxieme = son[q];
		
		semi = (premier + deuxieme)/2;
	
		itoa(semi,buffc,10);
		strcat(buffc,":");
		fputs(buffc, out);
	}
}




// trouver le plus petit, détection!!!!!!!!!!!!!!!!!!!!
int petitesse(long n1,long n2,long n3,long n4,long n5,long n6,long n7,long n8)
{
	long small1,small2,small3, ultimatesmall;
	int petit1,petit2,petit3, pluspetit;
	
	if (n1 < n2)			// premier round
	{
		small1 = n1;
		petit1=1;
	}
	else
	{
    	small1 = n2;
		petit1=2;
	}
	
	if (small1 < n3)
	{
        small1 = small1;
		petit1 = petit1;
	}	
	else
    {
		small1 = n3;
		petit1 = 3;
	}
	
	if (n4 < n5)           // deuxime round
    {
		small2 = n4;
		petit2 = 4;
    }
	else
    {
		small2 = n5;
		petit2 = 5;
    }

	if (small2 < n6)
	{
		small2 = small2;
		petit2 = petit2;
	}
	else
    {
		small2 = n6;
		petit2 = 6;
	}
	
	if (n7 < n8)           // troisieme round
    {	
		small3 = n7;
		petit3 = 7;
    }	
	else
	{
		small3 = n8;
		petit3 = 8;
    }

	if (small1 < small2)	// round final      
    {	
		ultimatesmall = small1;
		pluspetit = petit1;
    }	
	else
    {
		ultimatesmall = small2;
		pluspetit = petit2;
    }
	if (ultimatesmall < small3)
	{
		ultimatesmall = ultimatesmall;
		pluspetit = pluspetit;
    }
	else
    {
		ultimatesmall = small3;            // KNOCK-OUT !!!!!!!!!!!!!!!
		pluspetit = petit3;
	}


 return pluspetit;
}

// ca fucking work!!! et c'est pas le hasard!!!!!


