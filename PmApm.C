/*************************************************************\
 * PmApm                                                     *
 * =====                                                     *
 * (c) 1998 C. Bockemuehl (cbockem@datacomm.ch)              *
 *                                                           *
 * Shutdownprogramm mit der Moeglichkeit, zwischen           *
 * verschiedenen Versionen zu waehlen: Standby, Suspend,     *
 * Poweroff.                                                 *
 * Die Durchfuehrung geschieht mit Hilfe des Programms       *
 * APM/2 v1.4 von Roman Stangl (rstangl@vnet.ibm.com)        *
 *                                                           *
 * Version 0.1 :  7. 9.98                                    *
\*************************************************************/

#define INCL_PM
#define INCL_DOSQUEUES
#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "PmApm.H"

/***************************************************************************\
  Definitionen
\***************************************************************************/

#define WM_WEITER     WM_USER
#define HF_STDOUT     1
#define PWRFF_NORM    0
#define PWRFF_PLUS    1
#define PWRFF_MIN     2
#define OPT_NSTANDBY  0x0001
#define OPT_NSUSPEND  0x0002
#define OPT_NPOWEROFF 0x0004
#define PRF_APPLICATION  "PmApm"
#define PRFKEY_LASTSEL   "LastSelected"
#define PRFKEY_PWRFFOPT  "PoweroffOption"
#define PRFKEY_XTRAOPT   "XtraOptions"
#define PRFKEY_DISABLE   "MethodDisabled"

/***************************************************************************\
  Globale Variablen
\***************************************************************************/

HAB hab;
LONG iSelect;
ULONG pwrffOption,rbDisable;
CHAR xtraOptions[512],langMsgfile[32],MessageString[4096];
ULONG idPwrffOpt[]={ID_RB_PWRFFNORM,ID_RB_PWRFFPLUS,ID_RB_PWRFFMIN};
PSZ iParam[]={"standby","suspend","poweroff"};
PSZ optPoweroff[]={"","+","-"};
ULONG idrb[]={ID_RB_STANDBY,ID_RB_SUSPEND,ID_RB_POWEROFF};
ULONG idNrb[]={ID_CB_NSTANDBY,ID_CB_NSUSPEND,ID_CB_NPOWEROFF};
ULONG optNrb[]={OPT_NSTANDBY,OPT_NSUSPEND,OPT_NPOWEROFF};

/***************************************************************************\
  Funktionsdeklarationen
\***************************************************************************/

MRESULT EXPENTRY HauptDialogfunktion(HWND hwnd, ULONG msg,
                                     MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ZweitDialogfunktion(HWND hwnd, ULONG msg,
                                     MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY OptionenDialogfunktion(HWND hwnd, ULONG msg,
                                        MPARAM mp1, MPARAM mp2);
PSZ GetMessage(ULONG msgId);
void PutControlTexts(HWND hwnd, ULONG msgId[][2], ULONG num);
void CenterDialog(HWND hwnd, HWND hwndRef);
void EnableRadiobuttons(HWND hwnd);
void ErrorMessage(PSZ message);

/***************************************************************************\
  Hauptprogramm
\***************************************************************************/

int main(void)
{
  /* Initialisierung */
  hab=WinInitialize(0);

  if(hab!=NULLHANDLE)
  {
    HMQ hmq;

    /* Message-Queue: Sollte eigentlich nicht nîtig sein, aber ohne dies
       scheinen manchmal komische Effekte aufzutreten. */
    hmq=WinCreateMsgQueue(hab,0);

    if(hmq!=NULLHANDLE)
    {
      HWND hwndDlg;
      ULONG ulrc;

      /* Erstes Dialogfenster erzeugen */
      hwndDlg=WinLoadDlg(HWND_DESKTOP,NULLHANDLE,
                         HauptDialogfunktion,NULLHANDLE,
                         ID_MAIN,NULL);

      if(hwndDlg!=NULLHANDLE)
      {
        /* Eigentliche ProgrammausfÅhrung hier */
        ulrc=WinProcessDlg(hwndDlg);

        /* Der Dialog muss hier noch explizit wieder beendet werden! */
        WinDestroyWindow(hwndDlg);
      }

      /* Wenn wirklich Abschluss gewÅnscht: Weiter */
      if(DID_OK==ulrc)
      {
        /* Zweites Dialogfenster erzeugen */
        hwndDlg=WinLoadDlg(HWND_DESKTOP,NULLHANDLE,
                           ZweitDialogfunktion,NULLHANDLE,
                           ID_SECOND,NULL);

        if(hwndDlg!=NULLHANDLE)
        {
          /* Eigentliche ProgrammausfÅhrung hier */
          WinProcessDlg(hwndDlg);

          /* Der Dialog muss hier noch explizit wieder beendet werden! */
          WinDestroyWindow(hwndDlg);
        }
      }

      /* Message-Queue beenden */
      WinDestroyMsgQueue(hmq);
    }

    /* Und Schluss */
    WinTerminate(hab);
  }

  return(0);
}

/***************************************************************************\
  Hauptfenster-Funktion (Dialog)
\***************************************************************************/

MRESULT EXPENTRY HauptDialogfunktion(HWND hwnd, ULONG msg,
                                     MPARAM mp1, MPARAM mp2)
{
  static HWND hmenu=NULLHANDLE;

  switch(msg)
  {
    case WM_INITDLG:
    {
      HPOINTER hicon;
      PSZ psz;
      FILE *msgf;
      ULONG ctlMsg[][2]={{ID_GB_METHOD,  MSG_GB_METHOD},
                         {ID_RB_STANDBY, MSG_STANDBY  },
                         {ID_RB_SUSPEND, MSG_SUSPEND  },
                         {ID_RB_POWEROFF,MSG_POWEROFF },
                         {DID_OK,        MSG_OK       },
                         {DID_CANCEL,    MSG_CANCEL   }};

      /* Message-File in der richtigen Sprache suchen */
      psz=getenv("LANG");
      strcpy(langMsgfile,"PmApmEN.MSG");
      if(NULL!=psz)
        strncpy(&langMsgfile[5],psz,2);
      if(NULL!=(msgf=fopen(langMsgfile,"r")))
      {
        fclose(msgf);
      }
      else
      {
        strcpy(langMsgfile,"PmApmEN.MSG");
        if(NULL!=(msgf=fopen(langMsgfile,"r")))
        {
          fclose(msgf);
        }
        else
        {
          /* Only message that doesn't come from a message file !! */
          ErrorMessage("Could not find message file");
          WinSendMsg(hwnd,WM_COMMAND,MPFROM2SHORT(DID_CANCEL,0),(MPARAM)0);
        }
      }

      /* Fenstertitel */
      WinSetWindowText(hwnd,GetMessage(MSG_HDR_MAIN));

      /* Controls beschriften */
      PutControlTexts(hwnd,ctlMsg,sizeof(ctlMsg)/(2*sizeof(ULONG)));

      /* MenÅ erzeugen, falls vorhanden */
      hmenu=WinLoadMenu(hwnd,NULLHANDLE,ID_MAIN);
      WinSetMenuItemText(hmenu,ID_OPTIONS,GetMessage(MSG_OPTIONS));
      if(hmenu!=NULLHANDLE)
        WinSendMsg(hwnd,WM_UPDATEFRAME,MPFROMLONG(FID_MENU),MPVOID);

      /* Icon setzen, falls vorhanden */
      hicon=WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_MAIN);
      if(hicon!=NULLHANDLE)
        WinSendMsg(hwnd,WM_SETICON,MPFROMLONG(hicon),MPVOID);

      /* Dialog auf dem Bildschirm zentrieren */
      CenterDialog(hwnd,HWND_DESKTOP);

      /* Weitere Vorbereitungen erst spÑter */
      WinPostMsg(hwnd,WM_WEITER,(MPARAM)0,(MPARAM)0);
    } break;

    /* Controls erstellen */
    case WM_WEITER:
    {
      ULONG n;
      CHAR buf[32];

      /* Selection und Focus auf zuletzt gewÑhlten Radiobutton */
      PrfQueryProfileString(HINI_USERPROFILE,PRF_APPLICATION,PRFKEY_LASTSEL,
                            "2" /* Default=Standby */,buf,sizeof(buf)-1);
      sscanf(buf,"%d",&n);
      WinCheckButton(hwnd,idrb[n],TRUE);
      WinFocusChange(HWND_DESKTOP,WinWindowFromID(hwnd,idrb[n]),0);

      /* Globale Optionen einlesen */
      PrfQueryProfileString(HINI_USERPROFILE,PRF_APPLICATION,PRFKEY_PWRFFOPT,
                            "0",buf,sizeof(buf)-1);
      sscanf(buf,"%d",&pwrffOption);
      PrfQueryProfileString(HINI_USERPROFILE,PRF_APPLICATION,PRFKEY_XTRAOPT,
                            "",xtraOptions,sizeof(xtraOptions)-1);
      PrfQueryProfileString(HINI_USERPROFILE,PRF_APPLICATION,PRFKEY_DISABLE,
                            "0",buf,sizeof(buf)-1);
      sscanf(buf,"%d",&rbDisable);

      /* Ggf. Radiobuttons ausgrauen */
      EnableRadiobuttons(hwnd);
    } break;

    case WM_COMMAND:
    {
      switch(SHORT1FROMMP(mp1))
      {
        case ID_OPTIONS:
        {
          ULONG drc;

          /* Neue Optionen auswÑhlen */
          drc=WinDlgBox(HWND_DESKTOP,hwnd,(PFNWP)OptionenDialogfunktion,
                        NULLHANDLE,ID_DLG_OPTIONS,NULL);

          /* Ggf. Radiobuttons anpassen */
          if(DID_OK==drc)
            EnableRadiobuttons(hwnd);
        } break;

        case DID_OK:
        {
          ULONG n;
          CHAR buf[128];
          BOOL allDisabled;

          /* Wenn alle Radiobuttons disabled sind: Melden und nichts tun */
          allDisabled=TRUE;
          for(n=0;n<3;n++)
            if(WinIsControlEnabled(hwnd,idrb[n]))
              allDisabled=FALSE;
          if(allDisabled)
          {
            ErrorMessage(GetMessage(MSG_ERR_NOMETHOD));
            break;
          }

          /* Feststellen, was gewÑhlt wurde und fÅr nÑchstes Mal merken */
          for(n=0;n<3;n++)
          {
            if((BOOL)WinQueryButtonCheckstate(hwnd,idrb[n]))
              iSelect=n;
          }
          sprintf(buf,"%1d",iSelect);
          PrfWriteProfileString(HINI_USERPROFILE,PRF_APPLICATION,
                                PRFKEY_LASTSEL,buf);

          /* Diesen Dialog beenden */
          WinDismissDlg(hwnd,DID_OK);
        } break;

        case DID_CANCEL:
        {
          WinDismissDlg(hwnd,DID_CANCEL);
        } break;
      }
    } break;

    default:
    {
      return WinDefDlgProc(hwnd,msg,mp1,mp2);
    }
  }

  return (MRESULT)FALSE;
}

/***************************************************************************\
  Zweite Hauptfenster-Funktion (Dialog)
\***************************************************************************/

MRESULT EXPENTRY ZweitDialogfunktion(HWND hwnd, ULONG msg,
                                     MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_INITDLG:
    {
      HPOINTER hicon;
      ULONG actionMsg[]={MSG_STANDBY,MSG_SUSPEND,MSG_POWEROFF};
      ULONG ctlMsg[][2]={{ID_ST_SHUTDOWN,0          },
                         {ID_ST_WAIT,    MSG_ST_WAIT}};
      CHAR msg[128];

      /* Fenstertitel */
      WinSetWindowText(hwnd,GetMessage(MSG_HDR_SECOND));

      /* Controls beschriften */
      strcpy(msg,GetMessage(actionMsg[iSelect]));
      GetMessage(MSG_ST_SHUTDOWN);
      strcat(MessageString,msg);
      PutControlTexts(hwnd,ctlMsg,sizeof(ctlMsg)/(2*sizeof(ULONG)));

      /* Icon setzen, falls vorhanden */
      hicon=WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_MAIN);
      if(hicon!=NULLHANDLE)
        WinSendMsg(hwnd,WM_SETICON,MPFROMLONG(hicon),MPVOID);

      /* Dialog auf dem Bildschirm zentrieren */
      CenterDialog(hwnd,HWND_DESKTOP);

      /* APM.EXE aufrufen */
      WinPostMsg(hwnd,WM_WEITER,(MPARAM)0,(MPARAM)0);
    } break;

    /* Weitere Bearbeitung */
    case WM_WEITER:
    {
      CHAR ch,buf[256],objName[CCHMAXPATH],arg[1024];
      HPIPE hpR,hpW;
      ULONG n,cbRead,spCount,ptCount;
      HFILE hfSave=-1,hfNew=HF_STDOUT;
      RESULTCODES resCodes;
      APIRET arc;

      /* Filehandle von STDOUT sichern */
      DosDupHandle(HF_STDOUT,&hfSave);
      DosCreatePipe(&hpR,&hpW,256);        /* Creates pipe                      */
      DosDupHandle(hpW,&hfNew);            /* Duplicates standard output handle */

      /* APM2 mit dem entsprechenden Parameter starten */
      sprintf(arg,"%s%c/%s%s %s%c%c",
              "APM.EXE",0,
              iParam[iSelect],(iSelect==0)?(optPoweroff[pwrffOption]):"",
              xtraOptions,0,0);
      arc=DosExecPgm(objName,sizeof(objName),EXEC_ASYNC,arg,NULL,
                     &resCodes,"APM.EXE");

      /* Ende, wenn Programmstart nicht gelungen ist */
      /* Anmerkung: Keine Fehlermeldung... */
      if(NO_ERROR!=arc)
      {
        ErrorMessage(GetMessage(MSG_ERR_STARTNPOSSIBLE));
        WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
        break;
      }

      DosClose(hpW);                       /* Closes write handle to ensure     */
                                           /* Notification at child termination */
      DosDupHandle(hfSave,&hfNew);         /* Brings stdout back                */

      /* Einlesen und Text ausgeben */
      spCount=ptCount=0;
      do
      {
        DosRead(hpR,&ch,1,&cbRead);
        switch(ch)
        {
          case ' ':
          {
            spCount++;
          } break;

          case '.':
          {
            if(10==spCount)
              WinSendMsg(WinWindowFromID(hwnd,ID_SL_SHUTDOWN),
                         SLM_SETSLIDERINFO,
                         MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                      SMA_INCREMENTVALUE),
                         (MPARAM)(++ptCount));

          } break;

          default:
          {
            spCount=0;
          }
        }
      }
      while(cbRead);

      /* Folgender Output wird von APM2 erzeugt (korrekter Programmlauf!):
      ------------------------------------------------------------------------------
      APM/2 V1.40 - Advanced Power Management spec. 1.1/1.2 interface for OS/2
      Copyright (C) by Roman Stangl, 1997, 1998 (rstangl@vnet.ibm.com)
                       http://www.geocities.com/SiliconValley/Pines/7885/

      APM0059I: GerÑtetreiber APM$ wurde geîffnet, APM wird in 10 Sekunden aktiviert
                ..........

      APM0065I: Angeforderte APM "Funktion" wird 10 Sekunden durchgefÅhrt.
                ..........

      APM0068I: Die (asynchron bearbeitete) APM Funktion wird in KÅrze durchgefÅhrt.
      ------------------------------------------------------------------------------
      Die 2 x 10 Punkte werden als ZÑhler fÅr die Sekunden ausgegeben. PMAPM
      filtert sie heraus und benutzt sie zur Steuerung seiner Fortschrittsanzeige.
      Wenn kein Output mehr kommt, beendet sich das Programm.
      */

      /* Programm beenden */
      WinPostMsg(hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
    }

    default:
    {
      return WinDefDlgProc(hwnd,msg,mp1,mp2);
    }
  }

  return (MRESULT)FALSE;
}

/***************************************************************************\
  Optionen-Dialogfunktion
\***************************************************************************/

MRESULT EXPENTRY OptionenDialogfunktion(HWND hwnd, ULONG msg,
                                        MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_INITDLG:
    {
      ULONG n;
      ULONG idCtl[]={ID_RB_PWRFFNORM,ID_RB_PWRFFPLUS,ID_RB_PWRFFMIN};
      ULONG ctlMsg[][2]={{ID_GB_POWEROFF, MSG_POWEROFF    },
                         {ID_GB_XOPTIONS, MSG_GB_XOPTIONS },
                         {ID_GB_NMETHOD,  MSG_GB_NMETHOD  },
                         {ID_CB_NSTANDBY, MSG_STANDBY     },
                         {ID_CB_NSUSPEND, MSG_SUSPEND     },
                         {ID_CB_NPOWEROFF,MSG_POWEROFF    },
                         {DID_OK,         MSG_OK          },
                         {DID_CANCEL,     MSG_CANCEL      }};

      /* Fenstertitel */
      WinSetWindowText(hwnd,GetMessage(MSG_HDR_OPTIONS));

      for(n=0;n<3;n++)
      {
        sprintf(MessageString,"/%s%s",iParam[2],optPoweroff[n]);
        WinSetWindowText(WinWindowFromID(hwnd,idCtl[n]),MessageString);
      }

      /* Controls beschriften */
      PutControlTexts(hwnd,ctlMsg,sizeof(ctlMsg)/(2*sizeof(ULONG)));

      /* Fenster zentrieren */
      CenterDialog(hwnd,HWND_DESKTOP);

      /* Controls initialisieren */
      WinPostMsg(hwnd,WM_WEITER,(MPARAM)0,(MPARAM)0);
    } break;

    case WM_WEITER:
    {
      ULONG n;
      HWND hwndef;

      /* Radiobuttons Poweroff-Methode initialisieren */
      WinCheckButton(hwnd,idPwrffOpt[pwrffOption],TRUE);

      /* Eingabefeld Extra-Optionen initialisieren */
      hwndef=WinWindowFromID(hwnd,ID_EF_XOPTIONS);
      WinSendMsg(hwndef,EM_SETTEXTLIMIT,MPFROM2SHORT(128,0),(MPARAM)0);
      WinSetWindowText(hwndef,xtraOptions);

      /* Checkbuttons Methoden ausblenden initialisieren */
      for(n=0;n<3;n++)
      {
        WinCheckButton(hwnd,idNrb[n],0!=(optNrb[n]&rbDisable));
      }
    } break;

    case WM_COMMAND:
    {
      switch(SHORT1FROMMP(mp1))
      {
        case DID_OK:
        {
          ULONG n;
          CHAR buf[512];

          /* Poweroff-Methode lesen und merken */
          for(n=0;n<3;n++)
          {
            if((BOOL)WinQueryButtonCheckstate(hwnd,idPwrffOpt[n]))
              pwrffOption=n;
          }
          sprintf(buf,"%1d",pwrffOption);
          PrfWriteProfileString(HINI_USERPROFILE,PRF_APPLICATION,
                                PRFKEY_PWRFFOPT,buf);

          /* Extra-Optionen lesen und merken */
          WinQueryWindowText(WinWindowFromID(hwnd,ID_EF_XOPTIONS),
                             sizeof(xtraOptions)-1,xtraOptions);
          PrfWriteProfileString(HINI_USERPROFILE,PRF_APPLICATION,
                                PRFKEY_XTRAOPT,xtraOptions);

          /* Methoden ausblenden lesen und merken */
          rbDisable=0;
          for(n=0;n<3;n++)
          {
            if((BOOL)WinQueryButtonCheckstate(hwnd,idNrb[n]))
              rbDisable|=optNrb[n];
          }
          sprintf(buf,"%1d",rbDisable);
          PrfWriteProfileString(HINI_USERPROFILE,PRF_APPLICATION,
                                PRFKEY_DISABLE,buf);

          /* Dialog-Aenderungen ok */
          WinDismissDlg(hwnd,DID_OK);
        } break;

        case DID_CANCEL:
        {
          /* Dialog abbrechen */
          WinDismissDlg(hwnd,DID_CANCEL);
        } break;
      }
    } break;

    default:
    {
      return WinDefDlgProc(hwnd,msg,mp1,mp2);
    }
  }
  return (MRESULT)FALSE;
}

/***************************************************************************\
  Hilfsfunktionen
\***************************************************************************/

PSZ GetMessage(ULONG msgId)
{
  ULONG len,ulrc;

  /* String in globalen Speicherbereich kopieren */
  ulrc=(ULONG)DosGetMessage(NULL,0,MessageString,sizeof(MessageString)-1,
                            msgId,langMsgfile,&len);
  if(NO_ERROR==ulrc)
    MessageString[len]=0;
  else
    MessageString[0]=0;

  /* Immer Zeiger auf globalen Message-String zurÅckgeben */
  return MessageString;
}

void PutControlTexts(HWND hwnd, ULONG msgId[][2], ULONG num)
{
  ULONG n;

  for(n=0;n<num;n++)
    if(0==msgId[n][1])
      WinSetWindowText(WinWindowFromID(hwnd,msgId[n][0]),
                       MessageString);
    else
      WinSetWindowText(WinWindowFromID(hwnd,msgId[n][0]),
                       GetMessage(msgId[n][1]));
}

void CenterDialog(HWND hwnd, HWND hwndRef)
{
  SWP swp,swpRef;

  WinQueryWindowPos(hwnd,&swp);
  WinQueryWindowPos(hwndRef,&swpRef);

  swp.x=swpRef.x+(swpRef.cx-swp.cx)/2;
  swp.y=swpRef.y+(swpRef.cy-swp.cy)/2;

  WinSetWindowPos(hwnd,HWND_TOP,swp.x,swp.y,0,0,SWP_MOVE);
}

void EnableRadiobuttons(HWND hwnd)
{
  ULONG n;

  for(n=0;n<3;n++)
  {
    WinEnableControl(hwnd,idrb[n],0==(optNrb[n]&rbDisable));
  }

  for(n=0;n<4;n++)
  {
    if((0!=(optNrb[n%3]&rbDisable))&&
       ((BOOL)WinQueryButtonCheckstate(hwnd,idrb[n%3])))
      WinCheckButton(hwnd,idrb[(n+1)%3],TRUE);
  }
}

void ErrorMessage(PSZ message)
{
  CHAR msg[512];

  strcpy(msg,message);
  WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,msg,GetMessage(MSG_ERROR),
                0,MB_OK|MB_ERROR);
}

