/* ex: ts=2 */
/*	MakeNames  V1.0  -  Erstellt Namen aus Silbenliste */
/*	copyright 1997 Henning Peters  -  use for free in your programs */
/*      Modifications Copyright 1997 Alexander Schroeder */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

static char *kons="bdfghklmnprstvwz";
static char *end="nlrdst";
static char *vokal="aeiou";
static char *start="dgtskpvfr";
/* Dies anpassen, dann unten "n." nicht vergessen! */

char *makename() 
{
  int s,v,k,e,p=0,x=0,
    nk=16,ne=6,nv=5,ns=9;	/* Länge der Buchstaben-Strings */
  char *name="12345678901234567890";
  for (s=random()%3+2; /* 2-4 Silben */ s>0; s--) 
    {
      if (x==0) 
	{
	  k=random()%nk;
	  name[p]=kons[k]; 
	  p++;
	} 
      else 
	{
	  k=random()%ns;
	  name[p]=start[k]; 
	  p++;
	}
      v=random()%nv;
      name[p]=vokal[v]; 
      p++;
      if (random()%3==2 || s==0) 
	{
	  e=random()%ne;
	  name[p]=end[e]; 
	  p++; 
	  x=1;
	} 
      else
	x=0;
    }
  name[p] = '\0'; 
  name[0] = toupper(name[0]);
  return (char *)strdup(name);
}


/* Nur zum Testen... */
int main() 
{
  int x;
  srandom(time(0)); 
  for (x=0; x<50; x++) 
    printf("  \"%s\",\n", makename());
  return 0;
}
