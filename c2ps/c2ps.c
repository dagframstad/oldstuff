/*
** 
** c2ps.c : a C to PostScript program. Does the following:
**     
** keywords 		=> bold times
** text 		=> bold courier
** comments 		=> italic times
** linenumbers 		=> on righthand side
** procedure names 	=> on righthand side and top of page
**
** The C program should compile and execute on most computers. Has been
** tested on VAX/VMS, VAX/ULTRIX, RISC/ULTRIX, SunOS and DEC OSF/1.
**
** The generated PostScript file is a PostScript level 1 file
** and should work on most printers. 
** 
** Dag Framstad
**
**
*/
 
 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#if defined(_unix) || defined(__unix)
#ifndef unix 
#define unix
#endif
#endif

#ifdef VMS
#include <time.h>
#include <jpidef.h>
#include <ssdef.h>
#include <types.h>
struct jpi_item_lst {
  short int length;
  short int code;
  int buf_adr;
  int len_adr;
} jpitem[2];

#else
#if defined(M_SYSV) || defined(_AIX) || defined (_WIN32) || defined(linux)
#include <time.h>
#else
#include <sys/time.h>
#endif /* SCO || AIX */
#endif /* VMS */

#ifdef unix
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
struct passwd *pwd;
#endif

#define LINEWIDTH	12
#define NORMFSIZE	10
#define SMALLFSIZE	8
#define BIGFSIZE	12
#define MAXCHARSINLINE	256
#define BOTTOM		72
#define LMARG		60
#define VERSION		"V1.10"
#define TRUE		1
#define FALSE		0

#define FONT_NORMAL     1
#define FONT_SMALL      2
#define FONT_VERYSMALL  3
#define FONT_MICRO      4

#define FONT_UNKNOWN    0
#define FONT_ORDTEXT    1
#define FONT_KEYWORD    2
#define FONT_TEXT       3
#define FONT_COMMENT    4
#define FONT_LINENO     5
#define FONT_BOTTOM     6
#define FONT_TOP        7
#define FONT_PROC       8

#define PSIZE_A4 1
#define PSIZE_A3 2
#define PSIZE_A  3
#define PSIZE_L  4
#define PSIZE_E  5

static char *ckeywords[] = {
#ifdef VMS
  "_align",
#endif /* VMS */
  "asm",
  "auto",
  "break",
  "case",
  "char",
  "const",
  "continue",
  "default",
  "do",
  "double",
  "else",
  "enum",
#ifdef OBSOLOLETE
  "entry",
#endif /* OBSOLETE */
  "extern",
  "float",
  "for",
  "fortran",
#ifdef VMS
  "globaldef",
  "globalref",
  "globalvalue",
#endif /* VMS */
  "goto",
  "if",
  "int",
  "long",
#ifdef VMS
  "noshare",
  "readonly",
#endif /* VMS */
  "register",
  "return",
  "short",
  "signed",
  "sizeof",
  "static",
  "struct",
  "switch",
  "typedef",
  "union",
  "unsigned",
  "void",
  "volatile",
  "while",
  0
};

static char *cpluspluskeywords[] = {
  "asm",
  "auto",
  "break",
  "case",
  "catch",
  "char",
  "class",
  "const",
  "continue",
  "default",
  "delete",
  "do",
  "double",
  "else",
  "enum",
  "extern",
  "float",
  "for",
  "friend",
  "goto",
  "if",
  "inline",
  "int",
  "long",
  "new",
  "operator",
  "private",
  "protected",
  "public",
  "register",
  "return",
  "short",
  "signed",
  "sizeof",
  "static",
  "struct",
  "switch",
  "template",
  "this",
  "throw",
  "try",
  "typedef",
  "union",
  "unsigned",
  "virtual",
  "void",
  "volatile",
  "while",
  0
};

static char *javakeywords[] = {
  "abstract",
  "boolean",
  "break",
  "byvalue",
  "byte",
  "case",
  "cast",
  "catch",
  "char",
  "class",
  "const",
  "continue",
  "default",
  "do",
  "double",
  "else",
  "extends",
  "final",
  "finally",
  "float",
  "for",
  "future",
  "generic",
  "goto",
  "if",
  "implements",
  "import",
  "inner",
  "instanceof",
  "int",
  "interface",
  "long",
  "native",
  "new",
  "null",
  "operator",
  "outer",
  "package",
  "private",
  "protected",
  "public",
  "rest",
  "return",
  "short",
  "static",
  "super",
  "switch",
  "syncronized",
  "this",
  "throw",
  "throws",
  "transient",
  "try",
  "var",
  "void",
  "volatile",
  "while",
  0
};

static char **keywords = ckeywords;
 
char *argv0,
  ifname[80],
  ofname[80],
  ibuffer[MAXCHARSINLINE],
  obuffer[MAXCHARSINLINE],
  tmpbuffer[MAXCHARSINLINE],
  timbuf[25],
  user[25] = "??unknown??",
  cword[50],
  curfuncs[120],
  funcname[50],
  txtchar = ' ';
 
int psize = PSIZE_A4,
  txtmode,
  moreonline,
  Commode,
  fontsize = FONT_NORMAL,
  normfsize,
  smallfsize,
  bigfsize,
  botline,
  linewidth,
  monospace = FALSE,
  landscape = FALSE,
  notc = FALSE,
  nooutfile = FALSE,
  lastwasbslash,
  dslashcomment = FALSE,
  dslashCommode = FALSE,
  appendps = FALSE;
 
int i = 0,
  x,
  urx,
  ury,
  top,
  topline,
  pageno,
  lineno,
  lbrace,
  ypos,
  rmarg,
  status,
  ibuffp,
  obuffp,
  cwordp;
 
FILE *infile,
  *outfile;
 
int IsKeyword(),
  IsItAFunc();
 
void Usage(),
  MakePaperSize(),
  MakeFontSize(),
  MakeProlog(),
  PrintPage(),
  MakeNewPage(),
  WriteBuffer(),
  WriteFont(),
  WriteLineNo(),
  PutCharInBuffer(),
  WhatToPutIn(),
  PutWordInBuffer(),
  WasKeyword(),
  WasNotKeyword(),
  WasAFunc(),
  PutCharInWord(),
  StopComMode(),
  InComMode(),
  StartComMode(),
  StopTxtMode(),
  InTxtMode(),
  StartTxtMode(),
  ParseCFile(),
  ParseFile(),
  MakeTrailer(),
  Init();
 
 
 
main(argc, argv)
int argc;
char **argv;
{
  char name[80], c;
  int dotpos;
#ifdef unix
  char *slashptr;

  slashptr = strrchr(argv[0], '/');
  if (slashptr == NULL)
    slashptr = argv[0];
  else
    slashptr++;
  if (strcmp(slashptr, "J2ps") == 0 || strcmp(slashptr, "j2ps") == 0) {
    keywords = javakeywords;
    dslashcomment = TRUE;
  } else if (strcmp(slashptr, "C2ps") == 0) {
    keywords = cpluspluskeywords;
    dslashcomment = TRUE;
  } 
#endif /* unix */

if (argc == 1)
	Usage();

  argv0 = argv[0];
 
#if defined (unix)
  while ((c = getopt(argc, argv, "CJLm34alensvutof")) != EOF) 
#else
  if (argv[1][0] == '-')
    c = argv[1][1];
  if (argv[1][2] != ' ' && argv[1][2] != '\0')
    Usage();
#endif
  switch (c) {
  case 'C':
    dslashcomment = TRUE;
    keywords = cpluspluskeywords;
    i++;
    break;
  case 'J':
    dslashcomment = TRUE;
    keywords = javakeywords;
    i++;
    break;
  case 'L':
    landscape = TRUE;
    i++;
    break;
  case 'f':
    appendps = TRUE;
    i++;
    break;
  case 'm':
    monospace = TRUE;
    i++;
    break;
  case '3':
    psize = PSIZE_A3;
    i++;
    break;
  case '4':
    psize = PSIZE_A4;
    i++;
    break;
  case 'a':
    psize = PSIZE_A;
    i++;
    break;
  case 'l':
    psize = PSIZE_L;
    i++;
    break;
  case 'e':
    psize = PSIZE_E;
    i++;
    break;
  case 'n':
    fontsize = FONT_NORMAL;
    i++;
    break;
  case 's':
    fontsize = FONT_SMALL;
    i++;
    break;
  case 'v':
    fontsize = FONT_VERYSMALL;
    i++;
    break;
  case 'u':
    fontsize = FONT_MICRO;
    i++;
    break;
  case 't':
    notc = TRUE;
    i++;
    break;
  case 'o':
    nooutfile = TRUE;
    i++;
    break;
  case 'h':
  case '?':
  default:
    Usage();
  } /* switch */
  
  if (i > 6)  /* too many options */ 
    Usage();
  i++;

  if (argc <= i)
    Usage();
    
/*
** find the user name 
*/
#ifdef unix
  ((pwd = getpwuid((uid_t) getuid())) != NULL) ? strcpy(user, pwd->pw_name) : strcpy(user, "??unknown??");
#endif /* unix */

#ifdef VMS
  jpitem[0].buf_adr = user;
  jpitem[0].code = JPI$_USERNAME;
  jpitem[0].length = 25;
  jpitem[1].len_adr = 0;
  if (sys$getjpiw(0, 0, 0, &jpitem, 0, 0, 0) != SS$_NORMAL)
    strcpy(user, "??unknown??");
#endif /* VMS */

  for (; i < argc; i++) {
    strcpy(name, argv[i]);
    if (strcmp(name, "-") == 0) {
      strcpy(ifname, "stdin");
    } else if ((strchr(name, '.')) == 0) {
      sprintf(ifname, "%s.c", argv[i]);
      sprintf(ofname, "%s.ps", argv[i]);
    } else {
      strcpy(ifname, argv[i]);
      dotpos = 0;
      while (name[dotpos] != '.')
	dotpos++;
      name[dotpos] = '\0';
      sprintf(ofname, "%s.ps", name);
    }
	if (appendps)
	  sprintf(ofname, "%s.ps", ifname);

    if (strcmp(name, "-") == 0) {
      infile = stdin;
    } else {
      if ((infile = fopen(ifname, "r")) == NULL) {
	strcpy(ifname, argv[i]);
	if ((infile = fopen(ifname, "r")) == NULL) {
	  fprintf(stderr, "%s : can't open '%s'\n", argv0, ifname);
	  exit(1);
	}
      }
    }
    
    
    if (nooutfile) {
      outfile = stdout;
    } else {
      if ((outfile = fopen(ofname, "w+")) == NULL) {
	fprintf(stderr, "%s : can't open '%s'\n", argv0, ofname);
	exit(1);
      }
    }
 
    if (!nooutfile)
      printf("%s ==> %s\n", ifname, ofname);
    Init();
    MakeFontSize(fontsize);
    MakePaperSize(psize);
    MakeProlog();
    if (notc)
      ParseFile();
    else
      ParseCFile();
    MakeTrailer();
  } /* for */
  return(0);
} /* main() */
 
 
/* 
** Checks if kword is a C reserved word 
*/
int IsKeyword(kword)
char *kword;
{
  int count;
  int iskey = FALSE;
 
 
  for (count = 0; (!iskey) && keywords[count] != NULL; count++) {
    iskey = (strcmp(keywords[count], kword) == 0) ? TRUE : FALSE;
  }
 
  if (obuffp > 0) {
    if (obuffer[obuffp - 1] == '#')
      iskey = FALSE;
  }
  return(iskey);
} /* IsKeyWord() */
 
 
 
/*
** Prints usage help text when giving wrong command 
*/
void Usage() {
  fprintf(stderr, "usage: %s [-C|-J] [-L] [-o] [-t] [-m] [-n|-s|-v|-u] [-3|-4|-a|-e|-l] file(s)\n", argv0);
  fprintf(stderr, "where:\n");
  fprintf(stderr, "\t-C\tC++ mode\n");
  fprintf(stderr, "\t-J\tjava mode\n");
  fprintf(stderr, "\t-L\tpaper in landscape position\n");
  fprintf(stderr, "\t-o\tprint to standard out\n");
  fprintf(stderr, "\t-t\ttext mode, not program\n");
  fprintf(stderr, "\t-m\tuse only monospace fonts\n");
  fprintf(stderr, "\t-n\tnormal fontsize (10 pts)\n");
  fprintf(stderr, "\t-s\tsmall fontsize (8 pts)\n");
  fprintf(stderr, "\t-v\tvery small fontsize (6 pts)\n");
  fprintf(stderr, "\t-u\tmicro fontsize (4 pts)\n");
  fprintf(stderr, "\t-3\ta3 papersize (846x1188)\n");
  fprintf(stderr, "\t-4\ta4 papersize (596x846)\n");
  fprintf(stderr, "\t-a\ta papersize (612x792)\n");
  fprintf(stderr, "\t-e\texecutive papersize (540x756)\n");
  fprintf(stderr, "\t-l\tletter papersize (612x1000)\n");
  exit(1);
} /* Usage() */
 
 
/*
** Sets new font 
*/
void WriteFont(fn)
int fn;
{
/* 
**    FONT_ORDTEXT: Font for ordinary text	
**    FONT_KEYWORD: Font for keywords
**    FONT_TEXT:    Font for texts
**    FONT_COMMENT: Font for comments
**    FONT_LINENO:  Font for linenumbers
**    FONT_BOTTOM:  Font for bottomtext
**    FONT_TOP:     Font for toptext
**    FONT_PROC:    Font for procedures
**    default:	What font?
*/
  switch (fn) {
  case FONT_ORDTEXT: 
    fprintf(outfile, "ordfn ");
    break;
  case FONT_KEYWORD: 
    fprintf(outfile, "keyfn ");
    break;
  case FONT_TEXT: 
    fprintf(outfile, "txtfn ");
    break;
  case FONT_COMMENT: 
    fprintf(outfile, "comfn ");
    break;
  case FONT_LINENO: 
    fprintf(outfile, "linfn ");
    break;
  case FONT_BOTTOM: 
    fprintf(outfile, "botfn ");
    break;
  case FONT_TOP: 
    fprintf(outfile, "topfn ");
    break;
  case FONT_PROC: 
    fprintf(outfile, "prcfn ");
    break;
  case FONT_UNKNOWN:
  default: 
    if (txtmode) {
      WriteFont(FONT_TEXT);
    } else {
      if (Commode) {
	WriteFont(FONT_COMMENT);
      } else {
	WriteFont(FONT_ORDTEXT);
      }
    }
  } /* switch */
 
} /* WriteFont() */

 
/* 
** set font sizes 
*/
void MakeFontSize(size)
int size;
{
  switch (size) {
  case FONT_SMALL:
    normfsize = NORMFSIZE - 2;
    smallfsize = SMALLFSIZE - 2;
    bigfsize = BIGFSIZE - 2;
    linewidth = LINEWIDTH - 2;
    break;
  case FONT_VERYSMALL:
    normfsize = NORMFSIZE - 4;
    smallfsize = SMALLFSIZE - 4;
    bigfsize = BIGFSIZE - 4;
    linewidth = LINEWIDTH - 4;
    break;
  case FONT_MICRO:
    normfsize = NORMFSIZE - 6;
    smallfsize = SMALLFSIZE - 6;
    bigfsize = BIGFSIZE - 6;
    linewidth = LINEWIDTH - 6;
    break;
  case FONT_NORMAL:
  default:
    normfsize = NORMFSIZE;
    smallfsize = SMALLFSIZE;
    bigfsize = BIGFSIZE;
    linewidth = LINEWIDTH;
    break;
  } /* switch */
  botline = BOTTOM - (2 * linewidth);
} /* MakeFontSize() */


/*
** Defines the size of the PostScript BoundingBox depending on the papersize 
*/
void MakePaperSize(size)
int size;
{
  int tmp;
 
/* 
** a4 = 0 0 594 846
** a3 = 0 0 846 1188 
** a = 0 0 612 792
** executive = 0 0 540 756
** legal = 0 0 612 1008 
*/
 
  switch (size) {
  case PSIZE_A3:
    urx = 846;
    ury = 1188;
    break;
  case PSIZE_A4:
    urx = 594;
    ury = 846;
    break;
  case PSIZE_A:
    urx = 612;
    ury = 792;
    break;
  case PSIZE_E:
    urx = 540;
    ury = 756;
    break;
  case PSIZE_L:
    urx = 612;
    ury = 1008;
    break;
  } /* switch */
  
  if (landscape) {
    tmp = urx;
    urx = ury;
    ury = tmp;
  }
 
  ypos = top = ury - normfsize - 70;
  topline = top + linewidth;
  rmarg = urx - 36;
} /* MakePaperSize() */
 
 
 
/*
** Makes the PostScript Prolog 
*/
void MakeProlog() {
  time_t bt;
  struct tm *lt;
 
  static char *month[12] = {
    "January", 
    "February", 
    "Mars", 
    "April",
    "May", 
    "June", 
    "July", 
    "August",
    "September", 
    "October", 
    "November", 
    "December"
    };
  static char *wday[7] = {
    "Sunday", 
    "Monday", 
    "Tuesday", 
    "Wednesday",
    "Thursday", 
    "Friday", 
    "Saturday"
    };
 
  time(&bt);
  lt = localtime(&bt);
  sprintf(timbuf, "%s %d %02d:%02d 19%d", month[lt -> tm_mon], lt -> tm_mday,
	   lt -> tm_hour, lt -> tm_min, lt -> tm_year);
 
  fprintf(outfile, "%%!PS-Adobe-3.0\n");
  fprintf(outfile, "%%%%BoundingBox: 0 0 %d %d\n", urx, ury);
  fprintf(outfile, "%%%%LanguageLevel: 1\n");
  if (monospace) 
    fprintf(outfile, "%%%%DocumentFonts: Times-Roman Times-Bold Times-Italic Courier Courier-Bold Courier-Oblique Courier-BoldOblique\n");
  else 
    fprintf(outfile, "%%%%DocumentFonts: Times-Roman Times-Bold Times-Italic Courier-Bold\n");
  fprintf(outfile, "%%%%Title: %s\n", ofname);
  fprintf(outfile, "%%%%Creator: %s\n", argv0);
  fprintf(outfile, "%%%%Version: %s\n", VERSION);
  fprintf(outfile, "%%%%CreationDate: %s %s %d %02d:%02d:%02d 19%d\n",
	   wday[lt -> tm_wday], month[lt -> tm_mon], lt -> tm_mday,
	   lt -> tm_hour, lt -> tm_min, lt -> tm_sec, lt -> tm_year);
  fprintf(outfile, "%%%%For: %s\n", user);
  fprintf(outfile, "%%%%Pages: (atend)\n");
  fprintf(outfile, "%%%%EndComments\n");
  fprintf(outfile, "%%%%BeginProlog\n");
 
/* 
** define the newfont procedure, stack: fontsize font 
*/
  fprintf(outfile, "/nf {findfont exch scalefont setfont} def\n");

/*
** use ISOLatin1Encoding for comments and text 
*/
  fprintf( outfile, "%% define the ISOLatin1Encoding vector, if needed.\n");
  fprintf( outfile, "%% Copyright © 1986, Digital Equipment Corporation All Rights Reserved \n");
  fprintf( outfile, "mark/ISOLatin1Encoding 8#000 1 8#054{StandardEncoding exch get}for/minus\n");
  fprintf( outfile, "8#056 1 8#217{StandardEncoding exch get}for/dotlessi 8#301 1\n");
  fprintf( outfile, "8#317{StandardEncoding exch\n");
  fprintf( outfile, "get}for/space/exclamdown/cent/sterling/currency/yen/brokenbar/section\n");
  fprintf( outfile, "/dieresis/copyright/ordfeminine/guillemotleft/logicalnot/hyphen/registered\n");
  fprintf( outfile, "/macron/degree/plusminus/twosuperior/threesuperior/acute/mu/paragraph\n");
  fprintf( outfile, "/periodcentered/cedilla/onesuperior/ordmasculine/guillemotright/onequarter\n");
  fprintf( outfile, "/onehalf/threequarters/questiondown/Agrave/Aacute/Acircumflex/Atilde\n");
  fprintf( outfile, "/Adieresis/Aring/AE/Ccedilla/Egrave/Eacute/Ecircumflex/Edieresis/Igrave\n");
  fprintf( outfile, "/Iacute/Icircumflex/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde\n");
  fprintf( outfile, "/Odieresis/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute/Thorn\n");
  fprintf( outfile, "/germandbls/agrave/aacute/acircumflex/atilde/adieresis/aring/ae/ccedilla\n");
  fprintf( outfile, "/egrave/eacute/ecircumflex/edieresis/igrave/iacute/icircumflex/idieresis\n");
  fprintf( outfile, "/eth/ntilde/ograve/oacute/ocircumflex/otilde/odieresis/divide/oslash/ugrave\n");
  fprintf( outfile, "/uacute/ucircumflex/udieresis/yacute/thorn/ydieresis/ISOLatin1Encoding\n");
  fprintf( outfile, "where not{256 array astore def}if cleartomark\n");
  fprintf( outfile, "/encodefont { findfont dup maxlength dict begin { 1 index /FID ne \n");
  fprintf( outfile, "{ def } { pop pop } ifelse } forall /Encoding exch def dup /FontName exch def\n");
  fprintf( outfile, "currentdict definefont end } bind def\n");
  if (monospace) {
    fprintf( outfile, "/Courier-BoldObliqueISOLatin1 ISOLatin1Encoding /Courier-BoldOblique encodefont pop\n");
    fprintf( outfile, "/Courier-ObliqueISOLatin1 ISOLatin1Encoding /Courier-Oblique encodefont pop\n");
    fprintf( outfile, "/CourierISOLatin1 ISOLatin1Encoding /Courier encodefont pop\n");
    fprintf( outfile, "/Courier-BoldISOLatin1 ISOLatin1Encoding /Courier-Bold encodefont pop\n");
    fprintf( outfile, "/Times-ItalicISOLatin1 ISOLatin1Encoding /Times-Italic encodefont pop\n");
    fprintf( outfile, "/Times-RomanISOLatin1 ISOLatin1Encoding /Times-Roman encodefont pop\n");
    fprintf( outfile, "/Times-BoldISOLatin1 ISOLatin1Encoding /Times-Bold encodefont pop\n");
  } else {
    fprintf( outfile, "/Times-ItalicISOLatin1 ISOLatin1Encoding /Times-Italic encodefont pop\n");
    fprintf( outfile, "/Courier-BoldISOLatin1 ISOLatin1Encoding /Courier-Bold encodefont pop\n");
    fprintf( outfile, "/Times-RomanISOLatin1 ISOLatin1Encoding /Times-Roman encodefont pop\n");
    fprintf( outfile, "/Times-BoldISOLatin1 ISOLatin1Encoding /Times-Bold encodefont pop\n");
  }
/* 
** define other the fonts procedures 
*/
  if (monospace) {
    fprintf(outfile, "/txtfn {%d /Courier-BoldObliqueISOLatin1 nf } def\n", normfsize);
    fprintf(outfile, "/keyfn {%d /Courier-BoldISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/ordfn {%d /CourierISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/comfn {%d /Courier-ObliqueISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/linfn {%d /Times-RomanISOLatin1 nf} def\n", smallfsize);
    fprintf(outfile, "/botfn {%d /Times-ItalicISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/topfn {%d /Times-BoldISOLatin1 nf} def \n", bigfsize);
    fprintf(outfile, "/prcfn {%d /Times-RomanISOLatin1 nf} def \n", bigfsize);
  } else {
    fprintf(outfile, "/txtfn {%d /Courier-BoldISOLatin1 nf } def\n", normfsize);
    fprintf(outfile, "/keyfn {%d /Times-BoldISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/ordfn {%d /Times-RomanISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/comfn {%d /Times-ItalicISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/linfn {%d /Times-RomanISOLatin1 nf} def\n", smallfsize);
    fprintf(outfile, "/botfn {%d /Times-ItalicISOLatin1 nf} def\n", normfsize);
    fprintf(outfile, "/topfn {%d /Times-BoldISOLatin1 nf} def \n", bigfsize);
    fprintf(outfile, "/prcfn {%d /Times-RomanISOLatin1 nf} def \n", bigfsize);
  }
/* 
** define the show procedure 
*/
  fprintf(outfile, "/s /show load def\n");

/* 
** define the rightshow procedure, stack: string 
*/
  fprintf(outfile, "/rs {dup stringwidth pop neg 0 rmoveto s} def\n");

/*
** define the moveto procedure 
*/
  fprintf(outfile, "/m /moveto load def\n");
  fprintf(outfile, "%%%%EndProlog\n");
  fprintf(outfile, "%%%%BeginSetup\n");
  WriteFont(FONT_ORDTEXT);
  fprintf(outfile, "\n%%%%EndSetup\n");

} /* MakeProlog() */
 
 
 
/* 
** Prints the page 
*/
void PrintPage() {
  fprintf(outfile, "%d %d m ", LMARG, botline);
  WriteFont(FONT_BOTTOM);
  fprintf(outfile, "(%s)s %d %d m (Page %d of %s)rs\n",
	   timbuf, rmarg, botline, pageno, ifname);
  fprintf(outfile, "%d %d m ", rmarg, topline);
  WriteFont(FONT_TOP);
  fprintf(outfile, "(%s \\(%s\\))rs\n", curfuncs, ifname);
  curfuncs[0] = '\0';
  fprintf(outfile, "showpage\n");
} /* PrintPage() */
 
 
 
/*
** Sets up a new page 
*/
void MakeNewPage() {
  pageno++;
  fprintf(outfile, "%%%%Page: %d %d\n", pageno, pageno);
  WriteFont(FONT_UNKNOWN);
  
  if (landscape)
    fprintf(outfile, "90 rotate 0 -%d translate\n", ury);
      
} /* MakeNewPage() */
 
 
 
/* 
** Writes the buffer to the file 
*/
void WriteBuffer() {
  obuffer[obuffp] = '\0';
  fprintf(outfile, "(%s)s\n", obuffer);
  obuffp = 0;
} /* WriteBuffer() */
 
 
 
/* 
** Writes the linenumber on the right side 
*/
void WriteLineNo(ln)
int ln;
{
  fprintf(outfile, "%d %d m ", rmarg, ypos);
  WriteFont(FONT_LINENO);
  fprintf(outfile, "(%d)rs\n", ln);
  WriteFont(FONT_UNKNOWN);
} /* WriteLineNo() */
 
 
/* 
** Puts a character in the buffer 
*/
void PutCharInBuffer(c)
char c;
{
  if (c != '\0' && c != '\n' && c != '\f' && c != '\r')
    obuffer[obuffp++] = c;
}
 
 
 
/* 
** Finds out which charaters to put in the buffer 
*/
void WhatToPutIn(c)
char c;
{
  int tmpi, tmpj;
  char eightbit[8]; 
  unsigned int i8;
  i8 = c & 0377; /* 0xff */
  switch (c) {
  case '\\': 
  case '(': 
  case ')': 
    PutCharInBuffer('\\');
    PutCharInBuffer(c);
    x++;
    break;
  case '\t': 
    tmpi = ((x / 8) + 1) * 8 - x;
    for (tmpj = tmpi; tmpj > 0; tmpj--)
      PutCharInBuffer(' ');
    x += tmpi;
    break;
  case '\0':
    break;
  default:
    if (i8 >= 128) {
      sprintf(eightbit, "%3o", i8);
      PutCharInBuffer('\\');
      PutCharInBuffer(eightbit[0]);
      PutCharInBuffer(eightbit[1]);
      PutCharInBuffer(eightbit[2]);
    } else {
      PutCharInBuffer(c);
    }
    x++;
    break;
  } /* switch */
} /* WhatToPutIn() */
 
 
 
/* 
** Puts the word in the buffer 
*/
void PutWordInBuffer() {
  obuffer[obuffp] = '\0';
  strcat(obuffer, cword);
  obuffp += strlen(cword);
  x += strlen(cword);
  cwordp = 0;
} /* PutWordInBuffer() */
 
 
 
/* 
** The last word was a keyword 
*/
void WasKeyword() {
  WriteBuffer();
  WriteFont(FONT_KEYWORD);
  fprintf(outfile, "(%s)s ", cword);
  WriteFont(FONT_ORDTEXT);
  fprintf(outfile, "\n");
/*  WhatToPutIn(ibuffer[ibuffp]); */
  cwordp = 0;
} /* WasKeyword() */
 
 
/* 
** Checks if it is a function 
*/
int IsItAFunc(comment, tmp, par, seen)
int comment, tmp, par, seen;
{
  long ptrpos;
 
  if (ibuffer[0] == '#')
    return(FALSE);
  while (tmp < MAXCHARSINLINE) {
    if (comment) {
      switch (tmpbuffer[tmp]) {
      case '*': 
	if (tmpbuffer[tmp + 1] == '/' && !dslashCommode) {
	  comment = FALSE;
	  tmp++;
	}
	break;
      case '\n': 
      case '\f': 
      case '\r': 
      case '\0':
	if (dslashCommode) {
	  comment = FALSE;
	  tmp++;
	  break;
	}
	ptrpos = ftell(infile);
	if ((fgets(tmpbuffer, MAXCHARSINLINE, infile)) == NULL)
	  perror(argv0);
	if (IsItAFunc(comment, 0, par, seen)) {
	  status = fseek(infile, ptrpos, 0);
	  if (status == EOF)
	    perror(argv0);
	  return(TRUE);
	} else {
	  status = fseek(infile, ptrpos, 0);
	  if (status == EOF)
	    perror(argv0);
	  return(FALSE);
	}
      } /* switch */ 
    } else {
	switch (tmpbuffer[tmp]) {
	case '/': 
	  if (tmpbuffer[tmp + 1] == '*') {
	    comment = TRUE;
	    tmp++;
	  } else if (dslashcomment && tmpbuffer[tmp + 1] == '/') {
	    dslashCommode = comment = TRUE;
	    tmp++;
	  } else {
	    return(FALSE);
	  }
	  break;
	case '(': 
	  seen = TRUE;
	  par++;
	  break;
	case ')': 
	  par--;
	  break;
	case ';': 
	case ',': 
	  if (par == 0)
	    return(FALSE);
	  break;
	case '\n': 
	case '\f': 
	case '\r': 
	case '\0': 
	  ptrpos = ftell(infile);
	  if ((fgets(tmpbuffer, MAXCHARSINLINE, infile)) == NULL)
	    perror(argv0);
	  if (IsItAFunc(comment, 0, par, seen)) {
	    status = fseek(infile, ptrpos, 0);
	    if (status == EOF)
	      perror(argv0);
	    return(TRUE);
	  } else {
	    status = fseek(infile, ptrpos, 0);
	    if (status == EOF)
	      perror(argv0);
	    return(FALSE);
	  }
	case ' ': 
	case '\t': 
	  break;
	default: 
	  if (!seen) {
	    return(FALSE);
	  } else {
	    if (par == 0 && seen)
	      return(TRUE);
	  }
	  break;
	} /* switch */
      } /* else */
    tmp++;
  } /* while */
} /* IsItAFunc() */
 
 
/* 
** Writes the function name on the right-hand side 
*/
void WasAFunc() {
  strcpy(funcname, cword);
  if (curfuncs[0] == '\0') {
    strcpy(curfuncs, funcname);
  } else {
    if (strlen(curfuncs) < 80) {
      strcat(curfuncs, " ");
      strcat(curfuncs, funcname);    
    }
  }
} /* WasInFunc() */
 
 
 
/* 
** The last word wasn't a keyword 
*/
void WasNotKeyword() {
  if (lbrace == 0) {
    strcpy(tmpbuffer, ibuffer);
    if (IsItAFunc(FALSE, ibuffp, 0, FALSE))
      WasAFunc();
  }
  PutWordInBuffer();
/*  WhatToPutIn(ibuffer[ibuffp]); */
}
 
 
 
 
/* 
** Puts a character in the word 
*/
void PutCharInWord() {
  cword[cwordp++] = ibuffer[ibuffp];
  cword[cwordp] = '\0';
} /* PutCharInWord() */
 
 
 
/* 
** Stops Comment mode 
*/
void StopComMode() {
  WhatToPutIn(ibuffer[ibuffp++]);
  if (!dslashCommode)
    WhatToPutIn(ibuffer[ibuffp]);
  WriteBuffer();
  WriteFont(FONT_ORDTEXT);
  dslashCommode = Commode = FALSE;
} /* StopComMode() */
 
 
 
/* 
** Parses file in Comment mode 
*/
void InComMode() {
  /* we accept doubleslash as comment */
  if (dslashCommode) {
    switch (ibuffer[ibuffp]) {
    case '\f': 
      WhatToPutIn(ibuffer[ibuffp]);
      ypos = 0;
    case '\r': 
    case '\n':
      StopComMode();
      if (ibuffp > 0)
	WriteBuffer();
      moreonline = FALSE;
      break;
    default:
      WhatToPutIn(ibuffer[ibuffp]);
      break;
    } /* switch */
    return;
  } 

  /* ordinary comment (like this) */
  switch (ibuffer[ibuffp]) {
  case '*': 
    if (ibuffer[ibuffp + 1] == '/')
      StopComMode();
    else
      WhatToPutIn(ibuffer[ibuffp]);
    break;
  case '\f': 
  case '\r': 
  case '\n': 
  case '\0': 
    if (ibuffp > 0)
      WriteBuffer();
    moreonline = FALSE;
    if (ibuffer[ibuffp] == '\f')
      ypos = 0;
    break;
  default: 
    WhatToPutIn(ibuffer[ibuffp]);
    break;
  } /* switch */
} /* InComMode() */
 
 
 
/* 
** Starts Comment mode 
*/
void StartComMode() {
  Commode = TRUE;
  WriteBuffer();
  WriteFont(FONT_COMMENT);
  WhatToPutIn(ibuffer[ibuffp]);
} /* StartComMode() */
 
 
 
/* 
** Stops Text mode 
*/
void StopTxtMode() {
  WhatToPutIn(ibuffer[ibuffp]);
  WriteBuffer();
  WriteFont(FONT_ORDTEXT);
  txtmode = FALSE;
} /* StopTxtMode() */
 
 
 
/* 
** Parses file in Text mode 
*/
void InTxtMode() {
  switch (ibuffer[ibuffp]) {
 
  case '\'': 
  case '\"': 
    if (ibuffer[ibuffp] == txtchar && ibuffp > 0 && !lastwasbslash)
      StopTxtMode();
    else
      WhatToPutIn(ibuffer[ibuffp]);
    lastwasbslash = FALSE;
    break;
  case '\f': 
  case '\r': 
  case '\n': 
  case '\0': 
    if (ibuffp > 0)
      WriteBuffer();
    moreonline = FALSE;
    if (ibuffer[ibuffp] == '\f')
      ypos = 0;
    lastwasbslash = FALSE;
    break;
  default: 
    if (!lastwasbslash && ibuffer[ibuffp] == '\\')
      lastwasbslash = TRUE;
    else
      lastwasbslash = FALSE;
    WhatToPutIn(ibuffer[ibuffp]);
  } /* switch */
} /* InTxtMode() */
 
 
 
/* 
** Starts Text mode 
*/
void StartTxtMode(){
  txtmode = TRUE;
  txtchar = ibuffer[ibuffp];
  WriteBuffer();
  WriteFont(FONT_TEXT);
  WhatToPutIn(ibuffer[ibuffp]);
 
} /* StartTxtMode() */
 
 
 
/* 
** Parses the C program 
*/
void ParseCFile() {
 
  while (fgets(ibuffer, MAXCHARSINLINE, infile) != NULL) {
 
/* 
** check if this line is empty or not 
*/
    if (ibuffer[0] != '\n') {
	    
/* 
** not empty line 
*/
 
      if (pageno == 0)	/* the first page */
	MakeNewPage();
      
      if (ypos < BOTTOM) {
/* 
** not enough space on this page
** print it and make a new one 
*/
	PrintPage();
	MakeNewPage();
	ypos = top;
      }
 
/* 
** move to the right position 
*/
      fprintf(outfile, "%d %d m ", LMARG, ypos);
    } else {
/* 
** empty line 
*/
      moreonline = FALSE;
    } 
 
/* 
** examine each charater if the line is not empty 
*/
    for (cwordp = ibuffp = obuffp = x = 0, moreonline = TRUE; moreonline; ibuffp++) {
      if (txtmode) {	/* we are printing text */
	InTxtMode();
      } else {		/* we are printing comments */
	if (Commode) {
	  InComMode();
	} else {		/* other text */
	  switch (ibuffer[ibuffp]) {
	  case '\n': 
	  case '\0': 
	  case '\r': 
	  case '\f': 
	    if (ibuffp > 0) {
	      if (cwordp > 0) {
		if (IsKeyword(cword))
		  WasKeyword();
		else
		  WasNotKeyword();
		WhatToPutIn(ibuffer[ibuffp]);
	      }
	    }
	    moreonline = FALSE;
	    if (ibuffer[ibuffp] == '\f')
	      ypos = 0;
	    break;
	  case '\"': 
	  case '\'': 
	    StartTxtMode();
	    break;
/*	  case '/': 
	    if (ibuffer[ibuffp + 1] == '*') {
	      StartComMode();
	      break;
	    }
	    if (ibuffer[ibuffp + 1] == '/') {
	      dslashCommode = TRUE;
	      StartComMode();
	      break;
	    }
*/ 
	  default: 
	    if (ibuffer[ibuffp] == '{') {
	      lbrace++;
	    } else {
	      if (ibuffer[ibuffp] == '}')
		lbrace--;
	    }
	    if (isalnum(ibuffer[ibuffp]) == 0 && ibuffer[ibuffp] != '_') {
/* 
** not an alphanum(or _), can't be part of a keyword or function 
*/
	      if (cwordp == 0) {
		if (ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '*') {
		  StartComMode();
		} else if (dslashcomment && ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '/') {
		  dslashCommode = TRUE;
		  StartComMode();
		} else {
		  WhatToPutIn(ibuffer[ibuffp]);
		}
	      } else {
		if (IsKeyword(cword))
		  WasKeyword();
		else
		  WasNotKeyword();
		
		if (ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '*') {
		  StartComMode();
		} else if (dslashcomment && ibuffer[ibuffp] == '/' && ibuffer[ibuffp + 1] == '/') {
		  dslashCommode = TRUE;
		  StartComMode();
		} else {
		  WhatToPutIn(ibuffer[ibuffp]);
		}
	      }
	    } else { /* could be a keyword or a function */
	      PutCharInWord();
	    }
	  } /* switch */
	} /* else */
      } /* else */
    } /* for */
    
/* 
** write what we got till now 
*/
    if (obuffp > 0)
      WriteBuffer();
    if (funcname[0] != '\0') {/* we have a funtion on this line */
      fprintf(outfile, "%d %d m ", rmarg, ypos);
      WriteFont(FONT_PROC);
      fprintf(outfile, "(%s)rs ", funcname);
      funcname[0] = '\0';
      WriteFont(FONT_UNKNOWN);
    } else {
      if (lineno % 10 == 0)
	WriteLineNo(lineno);
    }
    lineno++;
    ypos -= linewidth;
  } /* while */
  PrintPage();
} /* ParseCFile */


/* 
** Parses the file (not C)
*/
void ParseFile() {
 
  while (fgets(ibuffer, MAXCHARSINLINE, infile) != NULL) {
 
/* 
** check if this line is empty or not 
*/
    if (ibuffer[0] != '\n') {
	    
/* 
** not empty line 
*/
 
      if (pageno == 0)	/* the first page */
	MakeNewPage();
      
      if (ypos < BOTTOM) {
/* 
** not enough space on this page
** print it and make a new one 
*/
	PrintPage();
	MakeNewPage();
	ypos = top;
      }
 
/* 
** move to the right position 
*/
      fprintf(outfile, "%d %d m ", LMARG, ypos);
    } else {
/* 
** empty line 
*/
      moreonline = FALSE;
    } 
 
/* 
** examine each charater if the line is not empty 
*/
    for (cwordp = ibuffp = obuffp = x = 0, moreonline = TRUE; moreonline; ibuffp++) {
      switch (ibuffer[ibuffp]) {
      case '\n': 
      case '\0': 
      case '\r': 
      case '\f': 
	if (ibuffp > 0) {
	  if (cwordp > 0) {
	    WasNotKeyword();
	    WhatToPutIn(ibuffer[ibuffp]);
	  }
	}
	moreonline = FALSE;
	if (ibuffer[ibuffp] == '\f')
	  ypos = 0;
	break;
      default: 
	WhatToPutIn(ibuffer[ibuffp]);
	/*PutCharInWord();*/
      } /* switch */
    } /* for */
 
/* 
** write what we got till now 
*/
    if (obuffp > 0)
      WriteBuffer();
      if (lineno % 10 == 0)
	WriteLineNo(lineno);
    lineno++;
    ypos -= linewidth;
  } /* while */
  PrintPage();
} /* ParseFile */
 
 
/* 
** Makes the PostScript Trailer 
*/
void MakeTrailer() {
  fprintf(outfile, "%%%%Trailer\n");
  fprintf(outfile, "%%%%Pages: %d\n", pageno);
  fprintf(outfile, "%%%%EOF\n");
  if (!nooutfile) {
    status = fclose(outfile);
    if (status == -1)
      perror(argv0);
  } /* if */
  status = fclose(infile);
  if (status == -1)
    perror(argv0);
} /* MakeTrailer() */
 
/* 
** Initialize 
*/
void Init() {
  txtmode = FALSE;
  moreonline = TRUE;
  Commode = FALSE;
  lastwasbslash = FALSE;
  pageno = 0;
  lineno = 1;
  lbrace = 0;
} /* Init() */
