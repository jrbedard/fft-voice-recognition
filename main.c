
//main.c
//callback windows général
//instructions des transfo


//Jean-René Bédard. aka deslock.....03-2002


#include "FFT.h"

#define QUANTIZATION 16
#define WM_FREQ 1234     


//6 entrée mic possibles
#define WM_DEV  150
#define WM_DEV1 150
#define WM_DEV2 151
#define WM_DEV3 152
#define WM_DEV4 153
#define WM_DEV5 154
#define WM_DEV6 155



LRESULT CALLBACK MainWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    CALLBACK SettingsDlgProc (HWND, UINT, WPARAM, LPARAM) ;
TCHAR szAppName []  = TEXT ("Transformation Fourier, P8") ;
TCHAR szOpenError[] = TEXT ("Erreur Ouverture AudioDevice!");
TCHAR szMemError [] = TEXT ("Ne peut allouer la mémoire!") ;

typedef struct 
{
	int iSamplesPerSecond,
	    iNumOfSamples,
		iInputBufferSize;
} *PFFT_DATA;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;
		
	

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;  // Scrollbars
     wndclass.lpfnWndProc   = MainWndProc ;              // Proc
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_FFT));  
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ; // Curseur win
     wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH) ; 
     wndclass.lpszMenuName  = MAKEINTRESOURCE(IDM_FFT) ;  
     wndclass.lpszClassName = szAppName ;

     if (!RegisterClass (&wndclass))
     {    
          MessageBox (NULL, TEXT ("Initialization impossible"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }

      hwnd = CreateWindow (szAppName,                 // classe win
                  szAppName,                          // app name
                  WS_OVERLAPPEDWINDOW |
				  WS_MINIMIZEBOX      |
				  WS_HSCROLL          |
				  WS_MAXIMIZEBOX      |
				  WS_BORDER,
				  CW_USEDEFAULT,                      // Position x 
                  0,                                  // Position y
                  800,                                // largeur fenetre
                  500,                                // largeur fenetre
                  NULL,                               
                  NULL,                               // Menu
                  hInstance,                           
                  NULL) ;                             

     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}


LRESULT CALLBACK MainWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	 static BOOL         bRecording,    // enregistrer
                         bEnding,       // finir d'enregistrer
						 bTerminating ; // Quit	                 
     static HWAVEIN      hWaveIn  ;     // WaveIn DeviceHandle
	 static PWORD        pwTemp; 
	 static PBYTE       pBuffer1,				// Device Buffer 1
		                 pBuffer2,				// Device Buffer 2
						 pTempBuffer;			// Record  Buffer
     static PWAVEHDR     pWaveHdr1, pWaveHdr2 ; // WaveheadrStructure
     static WAVEFORMATEX waveform ;             
     static int   iDevSel,			// Menu Device Selection
		          iViewSel=0,		// Menu View Selection
		          iTmpRange=0,		// dernier -> Window Range
				  iStartPos=0,		// left side -> First Array
				  iRange = 256;		// afficher ->Window Range Array
	 static PFFT_DATA pFFT; 
     HDC         hdc ;               // Device Context
	 static HINSTANCE hInstance;     // HandleInstance
     int         i,                
		         iFreq,              // Frequence à afficher 
				 iNumDev;            // Number of Devices
				 
     static POINT       *apt;        //FFT Points 
     static float   *pfRealBuf,      // Buffer réel
	                    *pfImagBuf,  // Buffer imaginaire
	                    *pfResBuf;   // ResultBuffer
     static RECT rect,*del;          // client rect
	 
	 
	 WAVEINCAPS DevCaps;             // DriverCaps
     TCHAR buffer[30];               // WindowTextBuffer

     switch (message)
     {
	 
	case WM_KEYDOWN:
	{
			switch(wParam)
			{
			case VK_ESCAPE:
				enregistrer(1,hdc);
				break;
			}
	}
	

	 
	 case WM_COMMAND:
		   switch (LOWORD (wParam))
           {
              
		 
		       case IDM_SETTINGS:
                 DialogBoxParam (hInstance, 
					             MAKEINTRESOURCE(IDD_SETTINGS),
							     hwnd, 
							     SettingsDlgProc,
								 (long)pFFT) ;
				
                SendMessage(hwnd, WM_COMMAND,(WPARAM)IDM100,(LPARAM)NULL); // zoom à 100%
				// effacer écran
				InvalidateRect(hwnd,&rect,TRUE);
		      return 0;
              case IDM_EXIT:
               SendMessage (hwnd, WM_CLOSE, 0, 0) ;
              return 0 ;
			 
			  case IDM100:   // 100% zoom
                CheckMenuItem(GetMenu(hwnd),iViewSel,MF_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd),IDM100,MF_CHECKED);
				iViewSel = IDM100;
                iRange = pFFT->iNumOfSamples/2; 
				iStartPos =  0;
			    SetScrollRange (hwnd, SB_HORZ, 0,iRange, FALSE); // SetNewRange, barre non-updaté
                SetScrollPos   (hwnd, SB_HORZ, iRange/2, TRUE);  // SetNewPos, barre updaté 
                SendMessage(hwnd, WM_FREQ,0,0);
              return 0 ;
			  
			 
			  case WM_DEV1:
              case WM_DEV2:
			  case WM_DEV3:
			  case WM_DEV4:
			  case WM_DEV5:
			  case WM_DEV6:
				 bEnding = TRUE ;
                 waveInReset (hWaveIn) ; // reset WaveDevice
				 EnableMenuItem(GetMenu(hwnd),IDM_RSTOP,MF_GRAYED);        //Disable boutton Stop
			     EnableMenuItem(GetMenu(hwnd),IDM_RSTART,MF_ENABLED);      //enable boutton Start
			     SetWindowText(hwnd, TEXT("FFT analyseur de fréquence"));  // Update WindowName title
				 CheckMenuItem(GetMenu(hwnd),iDevSel+WM_DEV,MF_UNCHECKED); // uncheck old device
			     iDevSel = LOWORD(wParam)-WM_DEV;  
				 CheckMenuItem(GetMenu(hwnd),LOWORD(wParam),MF_CHECKED);   // coche 
			  return 0;
              // début enregistrement
			  case IDM_RSTART:
			   
			   EnableMenuItem(GetMenu(hwnd),IDM_SETTINGS,MF_GRAYED);

			   
			  /////////DÉCLARATION des Buffer's sizes///////////////////////////////////////
		      apt = malloc(pFFT->iInputBufferSize*sizeof(POINT));
		       
			  pTempBuffer = malloc(pFFT->iInputBufferSize);   // Recording Buffer:
		      
			  pfRealBuf= malloc(pFFT->iInputBufferSize*sizeof(float)); // Buffer des valeurs réelles FFT
		      
		      pfImagBuf= malloc(pFFT->iInputBufferSize*sizeof(float)); // Buffer des valeurs imaginaires FFT
		     
		      pfResBuf = malloc(pFFT->iInputBufferSize*sizeof(float)); // Resultat -> amplitudes (result)
			   
			  pBuffer1 = malloc (pFFT->iInputBufferSize) ; //AudioDevices
              pBuffer2 = malloc (pFFT->iInputBufferSize) ;
              ////////////////////////////////////////////////////////////////////////////// 

		       if(!pfRealBuf || 
			      !pfImagBuf || 
			      !pfResBuf  ||
			      !apt       ||
			      !pBuffer1  || 
			      !pBuffer2 )
			   {   MessageBeep (MB_ICONEXCLAMATION) ;
                   MessageBox (hwnd, szMemError, szAppName, MB_ICONEXCLAMATION | MB_OK) ;
			       return 0;
			   }     
               
			   //Ouvrir AudioDevice -> Enregistremnt
               waveform.wFormatTag      = WAVE_FORMAT_PCM ;
               waveform.nChannels       = 1 ;
               waveform.nSamplesPerSec  = pFFT->iSamplesPerSecond ;
               waveform.nAvgBytesPerSec = pFFT->iSamplesPerSecond*2;//16bit=2Byte*8kHz
               waveform.nBlockAlign     = QUANTIZATION/8 ; //16bit/8=2Byte
               waveform.wBitsPerSample  = QUANTIZATION ;   //16 bit par Sample
               waveform.cbSize          = 0 ;
			   
               // si AudioDevice non-disponible
               if (waveInOpen (&hWaveIn, iDevSel, &waveform, (DWORD) hwnd, 0, CALLBACK_WINDOW))
               {
                    free (pBuffer1);
                    free (pBuffer2);
                    MessageBeep (MB_ICONEXCLAMATION);
                    MessageBox (hwnd, szOpenError, szAppName, MB_ICONEXCLAMATION | MB_OK);
               }
               else
			   {
               // init et preparation WaveHeader1;
			   pWaveHdr1->lpData          = pBuffer1 ;
               pWaveHdr1->dwBufferLength  = pFFT->iInputBufferSize ;
               pWaveHdr1->dwBytesRecorded = 0 ;
               pWaveHdr1->dwUser          = 0 ;
               pWaveHdr1->dwFlags         = 0 ;
               pWaveHdr1->dwLoops         = 1 ;
               pWaveHdr1->lpNext          = NULL ;
               pWaveHdr1->reserved        = 0 ;
               
			   waveInPrepareHeader (hWaveIn, pWaveHdr1, sizeof (WAVEHDR)) ;
               
			   // init et preparation WaveHeader2;
               pWaveHdr2->lpData          = pBuffer2 ;
               pWaveHdr2->dwBufferLength  = pFFT->iInputBufferSize ;
               pWaveHdr2->dwBytesRecorded = 0 ;
               pWaveHdr2->dwUser          = 0 ;
               pWaveHdr2->dwFlags         = 0 ;
               pWaveHdr2->dwLoops         = 1 ;
               pWaveHdr2->lpNext          = NULL ;
               pWaveHdr2->reserved        = 0 ;
               
			   waveInPrepareHeader (hWaveIn, pWaveHdr2, sizeof (WAVEHDR)) ;
			   
			   SendMessage(hwnd, WM_FREQ,0,0);
			   EnableMenuItem(GetMenu(hwnd),IDM_RSTART,MF_GRAYED); //disable Start
			   EnableMenuItem(GetMenu(hwnd),IDM_RSTOP,MF_ENABLED); //enable Stop
			   }
			   return 0;
			  case IDM_RSTOP:
			   // Arreter l'enregistrement et reset AudioDevice 
               bEnding = TRUE ;
               waveInReset (hWaveIn) ; 
			   SetWindowText(hwnd, TEXT("FFT Frequency Analyser")); //reload WindowText
			   EnableMenuItem(GetMenu(hwnd),IDM_RSTOP,MF_GRAYED);   // disable stop
			   EnableMenuItem(GetMenu(hwnd),IDM_RSTART,MF_ENABLED); // enable Start
			   EnableMenuItem(GetMenu(hwnd),IDM_SETTINGS,MF_GRAYED);
			   EnableMenuItem(GetMenu(hwnd),IDM_SETTINGS,MF_ENABLED);
			   return 0;
		   } 
		   break;
	 case WM_CREATE:     
           hInstance = ((LPCREATESTRUCT) lParam)->hInstance ;
		   pFFT = malloc(sizeof(PFFT_DATA));
		   pFFT->iInputBufferSize   =1024;
		   pFFT->iNumOfSamples      = pFFT->iInputBufferSize/2;
		   pFFT->iSamplesPerSecond  = 11025;
		   
		   //-----------------------------------
           //RecordingDevice dans le Menu config
		   //-----------------------------------
           
           iNumDev = waveInGetNumDevs(); //nombre de waveIn du système
		   
		   for (i=0;i<iNumDev;i++)		 //append waveIn dans le menu
		   {
             waveInGetDevCaps(i, &DevCaps,sizeof(DevCaps));
             AppendMenu(GetSubMenu(GetMenu(hwnd),1), MF_STRING, i+WM_DEV, DevCaps.szPname) ;
		   }
		   
		   // Pre-Selectionner Device 0 (Windows WaveMapper);
           iDevSel = 0;
           CheckMenuItem(GetMenu(hwnd),WM_DEV,MF_CHECKED);
		   EnableMenuItem(GetMenu(hwnd),IDM_RSTOP,MF_GRAYED);
		   
		   //-------------------------------------
		   // Allouer Memoire WAVEHDR
           //-------------------------------------
           pWaveHdr1 = malloc (sizeof (WAVEHDR)) ;
           pWaveHdr2 = malloc (sizeof (WAVEHDR)) ;
		  
          hpen_paint   = CreatePen(PS_SOLID,1,RGB(0,200,240));
          hpen_del     = CreatePen(PS_SOLID,1,RGB(0,0,0));
          
          SendMessage(hwnd, WM_COMMAND,(WPARAM)IDM100,(LPARAM)NULL); // range barre au départ
		  return 0;
     
	 case WM_SIZE:
		  GetClientRect(hwnd, &rect);  // rect->Hdc
          return 0 ;
	 
     case WM_FREQ:
		 // Afficher la fréquence actuelle TITLE
		  iFreq = (iRange/2 + iStartPos)*pFFT->iSamplesPerSecond/pFFT->iNumOfSamples;
          wsprintf(buffer, TEXT("Frequence: %d Hz "), iFreq);
          SetWindowText(hwnd, buffer);
		  return 0;
     
	 case WM_HSCROLL:
          // Scroll Handling
          switch (LOWORD (wParam))
          {
          case SB_LINEUP:         iStartPos  -= 1 ;  break ;
          case SB_LINEDOWN:       iStartPos  += 1 ;  break ;
          case SB_PAGEUP:         iStartPos  -= 1 ;  break ;
          case SB_PAGEDOWN:       iStartPos  += 1 ;  break ;
		  break;
          case SB_THUMBTRACK:
               iStartPos =  HIWORD (wParam)-iRange/2 ;
              break ;
          default:
               return 0 ;
          }
		  if (iStartPos <-iRange/2) //max
			      iStartPos = -iRange/3;
		  else if (iStartPos > (int)(pFFT->iNumOfSamples-iRange)/2)
			  iStartPos= (int)(pFFT->iNumOfSamples-iRange)/2;
		  
		  SendMessage(hwnd, WM_FREQ,0,0); //update frequence
          SetScrollPos (hwnd, SB_HORZ, iRange/2 +iStartPos, TRUE) ;
          return 0 ;

     case MM_WIM_OPEN: // AudioDevice est ouvert  
		  // Ajouter les 2 Buffers 
          waveInAddBuffer (hWaveIn, pWaveHdr1, sizeof (WAVEHDR)) ;
          waveInAddBuffer (hWaveIn, pWaveHdr2, sizeof (WAVEHDR)) ;
          // Enregistrer
          bRecording = TRUE ; // Enregistre
          bEnding = FALSE ;    
          waveInStart (hWaveIn); // Start recording
          return TRUE ;
          
     case MM_WIM_DATA: // WaveformAudio.data  handling!
		  // Copy recording.data -> tempBuffer 
          CopyMemory (pTempBuffer, ((PWAVEHDR) lParam)->lpData,
                         ((PWAVEHDR) lParam)->dwBytesRecorded);

          if (bEnding)  
          {
               waveInClose (hWaveIn) ;
               return TRUE ;
          }
          
          // Ajouter nouveau buffer à AudioDevice
          waveInAddBuffer (hWaveIn, (PWAVEHDR) lParam, sizeof (WAVEHDR)) ;
		  

		  //-------------------------------------
		  // FFT Instructions:
		  //-------------------------------------
		  
		  // Copy AudioData -> RealBuffer et -> ImagBuffer=0, aligner les données
		  CopyAudioData(pTempBuffer, pfRealBuf, pfImagBuf, pFFT->iInputBufferSize); 
          
		  // Transformation FFT
          FFT(pfRealBuf, pfImagBuf, pFFT->iNumOfSamples);
          
		  // abs(z) = sqrt(re(z)^2+Im(z)^2) ->  resultBuffer
		  GetResult(pfRealBuf, pfImagBuf,pfResBuf,pFFT->iNumOfSamples);
          
		  hdc = GetDC(hwnd);		       
		  SelectObject(hdc,hpen_del);      // selection deleting Pen
          Polyline (hdc, apt, iTmpRange);  // effacer le dernier sampling
          
		  SelectObject(hdc,hpen_paint);// selection paint color
		  
		  // max Range = NumOfSamples/2 
          DrawFFTBuffer(hdc,rect,del,apt ,pfResBuf,iStartPos,iRange,pFFT->iNumOfSamples/2,iFreq,pFFT->iInputBufferSize);
		  iTmpRange = iRange;   // save Range
		  ReleaseDC(hwnd, hdc);
          return TRUE ;
          
     case MM_WIM_CLOSE:  // Fermer AudioDevice
          // fermer tout
          waveInUnprepareHeader (hWaveIn, pWaveHdr1, sizeof (WAVEHDR)) ;
          waveInUnprepareHeader (hWaveIn, pWaveHdr2, sizeof (WAVEHDR)) ;
          if (pBuffer1) free (pBuffer1) ;
          if (pBuffer2) free (pBuffer2) ;
		  if (apt) free(apt);
		  if (pTempBuffer) free (pTempBuffer) ;
		  if (pfResBuf) free (pfResBuf);
		  if (pfRealBuf) free (pfRealBuf) ;
		  if (pfImagBuf) free (pfImagBuf)  ;
          bRecording = FALSE ; // not recording
		  
          if (bTerminating) //  
               SendMessage (hwnd, WM_DESTROY, 0, 0) ;
          return TRUE ;
          
          case WM_CLOSE:
			  if (bRecording)  // recording?
               {
                    bTerminating = TRUE ; 
                    bEnding = TRUE ;
                    waveInReset (hWaveIn) ; //Reset device
					return 0;  // ne pas fermer!!!!!!!!!!!!!!!!!!! attendre WIM_DATA
               }
			   SendMessage (hwnd, WM_DESTROY, 0, 0) ;
          return 0 ;
          case WM_DESTROY:
		       DeleteObject(hpen_paint); 
			   DeleteObject(hpen_del);   
               if (pWaveHdr1) free (pWaveHdr1) ;
               if (pWaveHdr2) free (pWaveHdr2) ;
               PostQuitMessage (0) ;
		  return 0;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}



// Configuration box !
BOOL CALLBACK SettingsDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{
	static PFFT_DATA pFFTData ; 
	static int iValueSampleRate[] = {8000,11025,22050,44100},
		       iValueBufferSize[] = {1024,2048,4096,8192};
	static unsigned char cSampleRateNr=0,  
		                 cBufferSizeNr=0,
						 n;                
	switch (message)
     {
     case WM_INITDIALOG :
          pFFTData = (PFFT_DATA) lParam;   
		  CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO4, IDC_RADIO1+cBufferSizeNr);
         
          return TRUE ;
          
     case WM_COMMAND :
          switch (LOWORD (wParam))
          {
          case IDOK :
		   
			//buffer size
           for (n=0;n<4;n++)
		   {
		     if (IsDlgButtonChecked (hDlg, IDC_RADIO1+n))
			 {
				 pFFTData->iInputBufferSize  = iValueBufferSize[n];
				 pFFTData->iNumOfSamples     = iValueBufferSize[n]/2;
				 cBufferSizeNr=n;
				 break;
			 }
		   }
		   
          case IDCANCEL :
               EndDialog (hDlg, 0) ;
               return TRUE ;
          } 
          break ;
     }
     return FALSE ;
}

