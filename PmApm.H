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

/* ID-Nummern in Dialogen und MenÅs */

#define ID_MAIN                   1000
#define ID_SECOND                 1100
#define ID_GB_METHOD              2000
#define ID_RB_STANDBY             2100
#define ID_RB_SUSPEND             2200
#define ID_RB_POWEROFF            2300
#define ID_ST_SHUTDOWN            2400
#define ID_ST_WAIT                2410
#define ID_SL_SHUTDOWN            2500
#define ID_DLG_OPTIONS            3000
#define ID_OPTIONS                3010
#define ID_GB_POWEROFF            3100
#define ID_RB_PWRFFNORM           3110
#define ID_RB_PWRFFPLUS           3120
#define ID_RB_PWRFFMIN            3130
#define ID_GB_XOPTIONS            3200
#define ID_EF_XOPTIONS            3210
#define ID_GB_NMETHOD             3300
#define ID_CB_NSTANDBY            3310
#define ID_CB_NSUSPEND            3320
#define ID_CB_NPOWEROFF           3330

/* Message-Nummern fÅr Message-Files */

#define MSG_STANDBY                  1
#define MSG_SUSPEND                  2
#define MSG_POWEROFF                 3
#define MSG_OK                       4
#define MSG_CANCEL                   5
#define MSG_HDR_MAIN                11
#define MSG_OPTIONS                 12
#define MSG_GB_METHOD               13
#define MSG_HDR_SECOND              21
#define MSG_ST_SHUTDOWN             22
#define MSG_ST_WAIT                 23
#define MSG_HDR_OPTIONS             31
#define MSG_GB_XOPTIONS             32
#define MSG_GB_NMETHOD              33
#define MSG_ERROR                   41
#define MSG_ERR_NOMETHOD            42
#define MSG_ERR_STARTNPOSSIBLE      43

