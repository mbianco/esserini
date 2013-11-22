/*
    Esserini, a life simulation program
    Copyright (C) 2011-...  Mauro Bianco

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact information: mauro d o t bianco a t gmail d o t com
*/

#include "datastructures.h"
//#include "physicandstats.h"
#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include <algorithm>
//#include <math.h>

#include "instructions_impl.h"

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((b)>(a))?(a):(b))
#define rotatex(x,y) a11*x+a12*y
#define rotatey(x,y) a21*x+a22*y

extern int CODELENGTH;
extern int HISTORYLENGHT;
extern float MASSENERGYFACTOR;
extern float INITIALWEIGHT;
extern float INITIALENERGY;
extern float MAXFOOD;
extern float MAXHORMONS;
extern float PTF;
extern float PTM;
extern float MANURE_PROB;
extern float GROWTH_RATE;
extern float EAT_AMOUNT;

extern random_gererator rndgen;
std::ptrdiff_t myrandom (std::ptrdiff_t i) { return boost::random::uniform_int_distribution<>(0,i-1)(rndgen);}
std::ptrdiff_t (*p_myrandom)(std::ptrdiff_t) = myrandom;

// Il mondo e` una griglia n x n (mod n). Ogni cella contiene:
// 1) E` presente l'esserino monster_id
// 2) E` presente cibo in quantita` 0, 1, 2, 3
// 3) Il terreno e` concimato

// Il cibo ha una durata poi diventa concime
// La cacca diventa concime (non so se metterla)
// I cadaveri diventano concime

// Ci vuole un principio di conservazione della massa, in modo che
// l'energia totale sia conservata. Ad esempio, il cibo mangiato
// diventa massa (energia) per gli esserini e cacca. Il concime di
// traduce in materia da cui nasce il cibo, magari distribuito su piu`
// celle. In piu` esiste una massa contenuta nel mondo che puo` essere
// riassorbita dal mondo stesso o ceduta al cibo in modo in modo
// casuale.

// Il mondo ha un elenco di esserini con su scritto in che cella sono
// e come sono orientati (in modo assoluto (l'orientamento visto
// dall'esserino e` locale all'esserino stesso)). Data la posizione e
// l'orientamento il mondo vede cosa c'e` nelle celle vicine (relative
// alle regioni percepite dagli esserini) e ne informa l'esserino
// stesso.


int TMondo::get_time() const {
  return time;
}


TMondo::TMondo(int n, int m, int tepOut)
  :dim(n)
  ,maxmonsters(dim*dim)
  ,surface(dim*dim)
  ,work(dim*dim>10?dim*dim:10)
  ,being(maxmonsters)
{
  int i;

  // printf("Initialization of world\n");

  stepOut=tepOut;
  timestepstorun=-1;
  time=0;

  for (i=0; i<dim*dim; i++) {
    surface[i].monster_ID=-1;
    surface[i].manure=(boost::bernoulli_distribution<>(MANURE_PROB)(rndgen))?boost::random::uniform_real_distribution<>(0.0,MAXFOOD/4)(rndgen):0.0;
    surface[i].timestamp=0; // per sapere quando Ë nato il cibo nella cella... FIXME: RIGHT?
    surface[i].food=0.0;//(int)genunf(0.0,MAXFOOD/2);
    surface[i].index=-1; // index within array being...
    surface[i].monster=NULL;

  }

  for (i=0; i<maxmonsters; i++) {
    being[i].IDs=-1;
    being[i].orientation=-1;
    being[i].x=-1;
    being[i].y=-1; // posizione
    being[i].monster=NULL;
  }

  m=(m>dim*dim)?dim*dim:m;

  for (i=0; i<maxmonsters; i++) {
    IDstack.push(maxmonsters-i);
  }

  for (i=0; i<dim*dim; i++)
    work[i]=i;
  std::random_shuffle(work.begin(), work.end(), p_myrandom);

  // FIXME: questo commento va bene?? numeri da 0 a dim*dim-1 in work. FUNZIONA SOLO IN SISTEMI IN CUI
  // sizeof(int)==sizeof(long int)...

  for (i=0; i<m; i++) {
    being[i].IDs=IDstack.top();
    IDstack.pop();
    being[i].orientation=boost::uniform_smallint<>(0,3)(rndgen);  
    if (surface[work[i]].monster!=NULL) {
      printf("There is someone else... %d, %s\n", __LINE__, __FILE__);
      exit(1);
    }
    being[i].x=(work[i] / dim);
    being[i].y=(work[i] % dim);
    printf("%d -> NEW BEING\n", being[i].IDs);

    being[i].monster = new TMonster(being[i].IDs, this); // Gli esserini non hanno
    // senso dell'orientamento
    surface[being[i].x*dim+being[i].y].monster_ID=being[i].IDs;
    surface[being[i].x*dim+being[i].y].monster=being[i].monster;
    surface[being[i].x*dim+being[i].y].index;
  }
  n_monsters=m;

  nfree=dim*dim-m;

  initial_invariant=0.0;
  C_BORNS=0;
  C_DEADS=0;
  C_FIGHTS=0;
  C_MATES=0;
  T_MATES=0; // tries to mate
  T_FIGHTS=0; // tries to fight
  C_NAT_DEADS=0;
}


int TMondo::get_monster(int id) {
  int monster=0;


  while (being[monster].IDs!=id) {
    monster++;
    if (monster>maxmonsters) {
      return -1;
    }
  }

  return monster;
}


/*
  Regions:
  |----|------------------------|----|
  |    |  4 |  5 |  6 |  7 |  8 |    |
  |    |----|----|----|----|----|    |
  |    |    |  1 |  2 |  3 |    |    |
  | 14 |    -----|----|-----    | 15 |
  |    |    9    |  0 |    10   |    |
  |    |------------------------|    |
  |    |    11   | 13 |    12   |    |
  |    |         |    |         |    |
  ------------------------------------
*/
int TMondo::beignsaround(int monsterID) {
  int monster=get_monster(monsterID);
  int x=being[monster].x;
  int y=being[monster].y;
  x = modulo(x-3,dim);
  y = modulo(y-2,dim);
  int r=0;
  for (int i=0; i<7; i++) {
    for (int j=0; j<5; j++) {
      if (surface[(modulo(x+i,dim))*dim+(modulo(y+j,dim))].monster_ID!=-1)
        r++;
    }
  }
  return r;
}

int TMondo::evalue(int id, Tinstr instr) {
  int r;
  int i,j;
  int monster, totalbeings;
  float totalfood;
  int a11=1, a12=0, a21=0, a22=1; // this init values are only to silent warnings. They will be surely overwritten later
  int male=0, female=0, heat=0, big=0, small=0, heatsamesex=0, heatothersex=0; //counters
  int samesex, othersex;

  // Prima bisogna trovare la regione
  if ((r=get_region(instr))==-1) {
    printf("ERROR: Region=-1 %d, %s\n", __LINE__, __FILE__);
    exit(-1);
  }

  if ((monster=get_monster(id))==-1) {
    // id contains the index of monster with the being array
    printf("monster id appears to be not a valid id %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }


  switch (being[monster].orientation) {
  case NORTH:
    a11=1; a12=0;
    a21=0; a22=1;
    break;
  case EAST:
    a11=0; a12=-1;
    a21=-1; a22=0;
    break;
  case SOUTH:
    a11=-1; a12=0;
    a21=0; a22=-1;
    break;
  case WEST:
    a11=0; a12=1;
    a21=1; a22=0;
    break;
  default :
    assert(0);
  }

  switch (r) {
  case 0:
    work[0]=modulo((being[monster].x),dim); //+rotatex(0,0);
    work[1]=modulo((being[monster].y),dim); //+rotatey(0,0);
    i=1;
    break;
  case 1:
    work[0]=modulo((being[monster].x+rotatex(1,-1)),dim);
    work[1]=modulo((being[monster].y+rotatey(1,-1)),dim);
    i=1;
    break;
  case 2:
    work[0]=modulo((being[monster].x+rotatex(1,0)),dim);
    work[1]=modulo((being[monster].y+rotatey(1,0)),dim);
    i=1;
    break;
  case 3:
    work[0]=modulo((being[monster].x+rotatex(1,1)),dim);
    work[1]=modulo((being[monster].y+rotatey(1,1)),dim);
    i=1;
    break;
  case 4:
    work[0]=modulo((being[monster].x+rotatex(2,-2)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,-2)),dim);
    i=1;
    break;
  case 5:
    work[0]=modulo((being[monster].x+rotatex(2,-1)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,-1)),dim);
    i=1;
    break;
  case 6:
    work[0]=modulo((being[monster].x+rotatex(2,0)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,0)),dim);
    i=1;
    break;
  case 7:
    work[0]=modulo((being[monster].x+rotatex(2,1)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,1)),dim);
    i=1;
    break;
  case 8:
    work[0]=modulo((being[monster].x+rotatex(2,2)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,2)),dim);
    i=1;
    break;
  case 13:
    work[0]=modulo((being[monster].x+rotatex(-1,0)),dim);
    work[1]=modulo((being[monster].y+rotatey(-1,0)),dim);
    work[2]=modulo((being[monster].x+rotatex(-2,0)),dim);
    work[3]=modulo((being[monster].y+rotatey(-2,0)),dim);
    i=1;
    break;
  case 9:
    work[0]=modulo((being[monster].x+rotatex(1,-2)),dim);
    work[1]=modulo((being[monster].y+rotatey(1,-2)),dim);
    work[2]=modulo((being[monster].x+rotatex(0,-2)),dim);
    work[3]=modulo((being[monster].y+rotatey(0,-2)),dim);
    work[4]=modulo((being[monster].x+rotatex(0,-1)),dim);
    work[5]=modulo((being[monster].y+rotatey(0,-1)),dim);
    i=3;
    break;
  case 10:
    work[0]=modulo((being[monster].x+rotatex(1,2)),dim);
    work[1]=modulo((being[monster].y+rotatey(1,2)),dim);
    work[2]=modulo((being[monster].x+rotatex(0,1)),dim);
    work[3]=modulo((being[monster].y+rotatey(0,1)),dim);
    work[4]=modulo((being[monster].x+rotatex(0,2)),dim);
    work[5]=modulo((being[monster].y+rotatey(0,2)),dim);
    i=3;
    break;
  case 11:
    work[0]=modulo((being[monster].x+rotatex(-1,-2)),dim);
    work[1]=modulo((being[monster].y+rotatey(-1,-2)),dim);
    work[2]=modulo((being[monster].x+rotatex(-1,-1)),dim);
    work[3]=modulo((being[monster].y+rotatey(-1,-1)),dim);
    work[4]=modulo((being[monster].x+rotatex(-2,-2)),dim);
    work[5]=modulo((being[monster].y+rotatey(-2,-2)),dim);
    work[6]=modulo((being[monster].x+rotatex(-2,-1)),dim);
    work[7]=modulo((being[monster].y+rotatey(-2,-1)),dim);
    i=4;
    break;
  case 12:
    work[0]=modulo((being[monster].x+rotatex(-1,1)),dim);
    work[1]=modulo((being[monster].y+rotatey(-1,1)),dim);
    work[2]=modulo((being[monster].x+rotatex(-1,2)),dim);
    work[3]=modulo((being[monster].y+rotatey(-1,2)),dim);
    work[4]=modulo((being[monster].x+rotatex(-2,1)),dim);
    work[5]=modulo((being[monster].y+rotatey(-2,1)),dim);
    work[6]=modulo((being[monster].x+rotatex(-2,2)),dim);
    work[7]=modulo((being[monster].y+rotatey(-2,2)),dim);
    i=4;
    break;
  case 14:
    work[0]=modulo((being[monster].x+rotatex(2,-3)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,-3)),dim);
    work[2]=modulo((being[monster].x+rotatex(1,-3)),dim);
    work[3]=modulo((being[monster].y+rotatey(1,-3)),dim);
    work[4]=modulo((being[monster].x+rotatex(0,-3)),dim);
    work[5]=modulo((being[monster].y+rotatey(0,-3)),dim);
    work[6]=modulo((being[monster].x+rotatex(-1,-3)),dim);
    work[7]=modulo((being[monster].y+rotatey(-1,-3)),dim);
    work[8]=modulo((being[monster].x+rotatex(-2,-3)),dim);
    work[9]=modulo((being[monster].y+rotatey(-2,-3)),dim);
    i=5;
    break;
  case 15:
    work[0]=modulo((being[monster].x+rotatex(2,3)),dim);
    work[1]=modulo((being[monster].y+rotatey(2,3)),dim);
    work[2]=modulo((being[monster].x+rotatex(1,3)),dim);
    work[3]=modulo((being[monster].y+rotatey(1,3)),dim);
    work[4]=modulo((being[monster].x+rotatex(0,3)),dim);
    work[5]=modulo((being[monster].y+rotatey(0,3)),dim);
    work[6]=modulo((being[monster].x+rotatex(-1,3)),dim);
    work[7]=modulo((being[monster].y+rotatey(-1,3)),dim);
    work[8]=modulo((being[monster].x+rotatex(-2,3)),dim);
    work[9]=modulo((being[monster].y+rotatey(-2,3)),dim);
    i=5;
    break;
  }

  if (is_food_instr(instr)) {
    totalfood=0;
    for (j=0; j<2*i; j=j+2) {
      totalfood += surface[work[j]*dim+work[j+1]].food;
    }
    
    switch (get_op2(instr)) {
    case EQ: // NOT USED
      return (totalfood==get_threshold(instr));
      break;
    case NE: // NOT USED
      return (totalfood!=get_threshold(instr));
      break;
    case LT:
      return (totalfood<=get_threshold(instr));
      break;
    case GT:
      return (totalfood>=get_threshold(instr));
      break;
    default:
      printf("OP2 not recognized (value=%d), [%d, %s]\n", 
             get_op2(instr), __LINE__, __FILE__);
    }

  } else {

    totalbeings=0;
    for (j=0; j<2*i; j=j+2) {
      if (surface[work[j]*dim+work[j+1]].monster != NULL)
        switch (surface[work[j]*dim+work[j+1]].monster->getsex()) {
        case MALE:
          male++;
          break;
        case FEMALE:
          female++;
          break;
        }

      if (surface[work[j]*dim+work[j+1]].monster != NULL) {
        if (surface[work[j]*dim+work[j+1]].monster->isheat()) {
          if (surface[work[j]*dim+work[j+1]].monster->getsex()==
              being[monster].monster->getsex()) {
            heatsamesex++;
          } else {
            heatothersex++;
          }
        }
        if (surface[work[j]*dim+work[j+1]].monster->getsize()>=MAXSIZE/2) {
          big++;
        } else {
          small++;
        }
      }
    }
    if (being[monster].monster->getsex()==MALE) {
      samesex=male;
      othersex=female;
    } else {
      samesex=female;
      othersex=male;
    }

    heat=heatsamesex+heatothersex;

    if (samesex==0 && othersex==0) // 0
      totalbeings=NOBODY;
    if (samesex==1 && othersex==0 && heat==0 && big==0)  // 1
      totalbeings=SMALLSAMESEX;
    if (samesex==1 && othersex==0 && heat==0 && big==1)  // 2
      totalbeings=BIGSAMESEX;
    if (samesex==0 && othersex==1 && heat==0 && big==0)  // 3
      totalbeings=SMALLOTHERSEX;
    if (samesex==0 && othersex==1 && heat==0 && big==1)  // 4
      totalbeings=BIGOTHERSEX;
    if (samesex==1 && othersex==0 && heat>0)  // 5
      totalbeings=HEATSAMESEX;
    if (samesex==0 && othersex==1 && heat>0)  // 6
      totalbeings=HEATOTHERSEX;
    if (samesex>0 && othersex==0 && heat==0)  // 7
      totalbeings=SAMESEXES;
    if (samesex==0 && othersex>0 && heat==0)  // 8
      totalbeings=OTHERSEXES;
    if (samesex>0 && othersex==0 && heat>0)  // 9
      totalbeings=SAMEHEATS;
    if (samesex==0 && othersex>0 && heat>0)  // 10
      totalbeings=SAMEHEATS;
    if (samesex>0 && othersex>0 && heat==0)  // 11
      totalbeings=MIX;
    if (samesex>0 && othersex>0 && heatsamesex==0 && heatothersex >0)  // 12
      totalbeings=MIXOTHERHEAT;
    if (samesex>0 && othersex>0 && heatsamesex>0 && heatothersex==0)  // 13
      totalbeings=MIXSAMEHEAT;
    if (samesex>0 && othersex>0 && heatsamesex>0 && heatothersex>0)  // 14
      totalbeings=MIXBOTHHEAT;

  }

  if (get_value(instr)==NOBODY || get_value(instr)==NOBODY2)
    if (totalbeings==NOBODY)
      return 1;

  return (get_value(instr)==totalbeings);

}

void TMondo::updatemanurewithdeath(int monster) {
  int ox, oy, x,y, region, orientation, i;
  float addmanure;
  static const int nregioni=8;
  static const int regioni[nregioni]={0, 1, 2, 3, 4, 5, 6, 7};
  float add;

  //addmanure=max(MAXFOOD,(int)being[monster].monster->getsize());//*10.0/9.0;
  assert(check_invariant());

  addmanure=being[monster].monster->getsize()+being[monster].monster->getenergy()/MASSENERGYFACTOR;
  being[monster].monster->annichil();
  x=being[monster].x;
  y=being[monster].y;
  ox=x;
  oy=y;

  assert(addmanure>=0);

  orientation=being[monster].orientation;
  i=0;
  while (addmanure>0) {
    add=min(2.0, addmanure);
    region=regioni[i];
    i=(i+1)%nregioni;
    get_coordinates(region, orientation, x,y);
    //surface[x*dim+y].manure=(surface[x*dim+y].manure+1>MAXFOOD)?MAXFOOD:surface[x*dim+y].manure+1;//addmanure;
    surface[x*dim+y].manure+=add;//addmanure;
    addmanure-=add;
    x=ox;
    y=oy;
  }
  assert(check_invariant());
}


void TMondo::updatewithshit(float shit, int ID) {
  int ox, oy, x,y, region, orientation, i;
  float addmanure;
  static const int nregioni=8;
  static const int regioni[nregioni]={0, 1, 2, 3, 4, 5, 6, 7};
  int monster;
  float add;

  monster=get_monster(ID);
  assert(shit>=0);

  addmanure=shit;//*10.0/9.0;
  x=being[monster].x;
  y=being[monster].y;
  ox=x;
  oy=y;

  orientation=being[monster].orientation;
  i=0;
  while (addmanure>0) {
    add=min(1.0, addmanure);
    region=regioni[i];
    i=(i+1)%nregioni;
    get_coordinates(region, orientation, x,y);
    //surface[x*dim+y].manure=(surface[x*dim+y].manure+1>MAXFOOD)?MAXFOOD:surface[x*dim+y].manure+1;//addmanure;
    surface[x*dim+y].manure+=add;//addmanure;
    addmanure-=add;
    x=ox;
    y=oy;
  }
  assert(check_invariant());
}


void TMondo::foodevolution(int i) {
  float swap;

  if (-0.1*surface[i].food == 0.3*surface[i].manure) {
    swap=boost::random::uniform_real_distribution<>((-GROWTH_RATE*0.25)*surface[i].food, GROWTH_RATE*surface[i].manure+0.01)(rndgen);
  } else {
    swap=boost::random::uniform_real_distribution<>((-GROWTH_RATE*0.25)*surface[i].food, GROWTH_RATE*surface[i].manure)(rndgen);
  }

  if (swap > 0) { //remove from manure
    if (swap > surface[i].manure) {
      swap = surface[i].manure;
    }
    surface[i].manure-=swap;
    surface[i].food+=swap;
  } else { // remove from food
    swap = -swap; // difficulties to reason about negative numbers :'-(
    if (swap > surface[i].food) {
      swap = surface[i].food;
    }
    surface[i].manure+=swap;
    surface[i].food-=swap;
  }

  assert(check_invariant());
}

void TMondo::setup() {
  int i;

  for (i=0; i<dim*dim; i++) {
    surface[i].food=0;
    surface[i].manure=INITIALMANURE;

    // nascita del cibo 1 manure = 1 food
    foodevolution(i);

  }
}

/*
  Regions:
  |----|------------------------|----|
  |    |  4 |  5 |  6 |  7 |  8 |    |
  |    |----|----|----|----|----|    |
  |    |    |  1 |  2 |  3 |    |    |
  | 14 |    -----|----|-----    | 15 |
  |    |    9    |  0 |    10   |    |
  |    |------------------------|    |
  |    |    11   | 13 |    12   |    |
  |    |         |    |         |    |
  ------------------------------------

  Movements:
  |-----||-----||-----|
  |  5^ ||  ^  || ^7  |
  | <-| || 6|  || |-> |
  |  4  ||     ||  8  |
  |-----||-----||-----|
  |     ||  ^0 || 9   |
  |  <- || < >1|| ->  |
  |  15 || 3v2 ||     |
  |-----||-----||-----|
  | <-| ||  |  || |-> |
  | 14v ||  v  || v10 |
  |  13 || 12  ||11   |
  |-----||-----||-----|

  Absolute coordinates:
  North is: increment x
  West is: increment y

  Relative coordinates (w.r.t. to being)
  ahead: +x
  right: +y

*/

void TMondo::get_coordinates(int region, int orientation, int &x, int &y) {
  int a11=1, a12=0, a21=0, a22=1; // init values to kill warnings
  int oldx, oldy, newx=0, newy=0; // init values to kill warnings

  switch (orientation) {
  case NORTH:
    a11=1; a12=0;
    a21=0; a22=1;
    break;
  case EAST:
    a11=0; a12=-1;
    a21=-1; a22=0;
    break;
  case SOUTH:
    a11=-1; a12=0;
    a21=0; a22=-1;
    break;
  case WEST:
    a11=0; a12=1;
    a21=1; a22=0;
    break;
  default:
    assert(0);
  }

  oldx=x;
  oldy=y;

  switch (region) {
  case 0:
    newx=(oldx);//%dim; //+rotatex(0,0);
    newy=(oldy);//%dim; //+rotatey(0,0);
    break;
  case 1:
  case 4:
  case 5:
    newx=modulo((oldx+rotatex(1,-1)),dim);
    newy=modulo((oldy+rotatey(1,-1)),dim);
    break;
  case 2:
  case 6:
    newx=modulo((oldx+rotatex(1,0)),dim);
    newy=modulo((oldy+rotatey(1,0)),dim);
    break;
  case 3:
  case 7:
  case 8:
    newx=modulo((oldx+rotatex(1,1)),dim);
    newy=modulo((oldy+rotatey(1,1)),dim);
    break;
  case 9:
  case 14:
    newx=modulo((oldx+rotatex(0,-1)),dim);
    newy=modulo((oldy+rotatey(0,-1)),dim);
    break;
  case 10:
  case 15:
    newx=modulo((oldx+rotatex(1,0)),dim);
    newy=modulo((oldy+rotatey(1,0)),dim);
    break;
  case 11:
    newx=modulo((oldx+rotatex(-1,-1)),dim);
    newy=modulo((oldy+rotatey(-1,-1)),dim);
    break;
  case 12:
    newx=modulo((oldx+rotatex(-1,1)),dim);
    newy=modulo((oldy+rotatey(-1,1)),dim);
    break;
  case 13:
    newx=modulo((oldx+rotatex(-1,0)),dim);
    newy=modulo((oldy+rotatey(-1,0)),dim);
    break;
  default:
    assert(0);
  }

  // TODO (Mauro Bianco#1#): Added this to correct % operator as in another function...

  // if (newx<0)
  //   newx=dim+newx;
  // if (newy<0)
  //   newy=dim+newy;

  x=newx;
  y=newy;
}

int TMondo::moveto(int monster) {

  // Movement is accomplished only if destination cell is empty (return == 1)!

  int completeaction, region, id;
  int x,y;

  // TODO (Mauro Bianco#1#): adesso monster arriva da parametro
  //	printf("time=%d, Monster's id appears to be not a valid id(%d) %d, %s\n", time, id, __LINE__, __FILE__);
  //	exit(1);
  //  }


  completeaction=being[monster].monster->getnextaction();

  region=allaction2region(completeaction);

  if (being[monster].monster==NULL) {
    printf("time=%d, monster of index %d is not defined, line %d, file %s\n",
           time, monster, __LINE__, __FILE__);
    exit(23);
  }

  id=being[monster].monster->get_ID();

  if (region==0) {
    // TODO (Mauro Bianco#1#): Added this control to avoid a being to mate with itself, since before if the region was '0' the being results in a no move since the place were occupied
    // the being make an empty move since region 0 is my region
    return 1;
  }

  x=being[monster].x;
  y=being[monster].y;

  get_coordinates(region, being[monster].orientation, x, y);

  if (surface[x*dim+y].monster==NULL) {

    switch (region) {
    case 0:
    case 2:
    case 5:
    case 6:
    case 7:
      // SAME ORIENTATION
      //being[monster].oritntation=being[monster].oritntation;
      break;
    case 1:
    case 4:
    case 9:
    case 14:
      // TURN LEFT
      being[monster].orientation=(being[monster].orientation+3)%4; // IT WAS 3
      break;
    case 11:
    case 12:
    case 13:
      // TURN BEHIND
      being[monster].orientation=(being[monster].orientation+2)%4;
      break;
    case 3:
    case 8:
    case 10:
    case 15:
      // TURN RIGHT
      being[monster].orientation=(being[monster].orientation+1)%4;
      break;
    default :
      printf("There's something wrong... %d, %s\n", __LINE__, __FILE__);
      exit(1);
    }

    surface[being[monster].x*dim+being[monster].y].monster_ID=-1;
    surface[being[monster].x*dim+being[monster].y].index=0;
    surface[being[monster].x*dim+being[monster].y].monster=NULL;

    being[monster].x=x;
    being[monster].y=y;

    surface[x*dim+y].monster_ID=id;
    surface[x*dim+y].monster=being[monster].monster;
    surface[being[monster].x*dim+being[monster].y].index=monster;
#ifdef _DBG
    printf("%d -> Moving\n", being[monster].IDs);
#endif
    return 1;

  } else {
#ifdef _DBG
    printf("%d -> NOT moving\n", being[monster].IDs);
#endif
    return 0;
  }
}

void TMondo::random_move (int monster) {

  int i, tmpid;
  // the best is choose a random permutation...
  for (i=0; i<dim*dim; i++)
  	work[i]=i;

  std::random_shuffle(work.begin(), work.end(), p_myrandom);

  i=0;
  while (surface[work[i]].monster!=NULL) {
    i++;
    if (i>maxmonsters) {
      printf("The world if full... %d, %s\n", __LINE__, __FILE__);
      exit(1);
    }
  }

  tmpid=surface[being[monster].x*dim+being[monster].y].monster_ID;
  surface[being[monster].x*dim+being[monster].y].monster_ID=-1;
  surface[being[monster].x*dim+being[monster].y].index=0;
  surface[being[monster].x*dim+being[monster].y].monster=NULL;

  being[i].orientation=boost::uniform_smallint<>(0,3)(rndgen);
  if (surface[(work[i] / dim)*dim+(work[i] % dim)].monster!=NULL) {
    printf("There is someone else... %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }


  being[monster].x=(work[i] / dim);
  being[monster].y=(work[i] % dim);

  surface[being[monster].x*dim+being[monster].y].monster_ID=tmpid;
  surface[being[monster].x*dim+being[monster].y].monster=being[monster].monster;
  surface[being[monster].x*dim+being[monster].y].index=monster;

}

void TMondo::eat(int id) {
  int x,y,monster;
  float food;

  if ((monster=get_monster(id))==-1) {
    printf("Monster's id appears to be not a valid id %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }

  x=being[monster].x;
  y=being[monster].y;

  food=(surface[x*dim+y].food<=EAT_AMOUNT)?surface[x*dim+y].food:EAT_AMOUNT;

  surface[x*dim+y].food-=food;

  being[monster].monster->update(food, 0, 0);

  assert(check_invariant());

}

void TMondo::fight(int index1, int index2) {
  // index1 and index2 are indexes of the two figthing beings. The
  // result of the fighting may are these: 1) one of them dies 2) both
  // fight, loose energy, and the looser will be moved away in another
  // cell

  // index1 IS THE ACTING BEING!!!!!

  // During the fight each being loose energy. The loss is
  // proportional to the inverse of the weight. Since there's must be
  // a winner, which remains alive (the other may die or not, but the
  // winner lives for sure), we need an appropriate statistic. The
  // winner of a fight is the one with the highest energy at the end
  // of the fight.

  // The enrgy of a fight is proportional to the sum of the weight of
  // the beings involved.

  float fightenergy,energy1, energy2;
  float fightenergy1, fightenergy2;

  // TODO (questa affermazione#1#): non mi piace
  C_FIGHTS++;

  fightenergy=(being[index1].monster->getenergy()
               +being[index2].monster->getenergy())/6;
  fightenergy=boost::random::normal_distribution<>(fightenergy, fightenergy/5)(rndgen);


  energy1=being[index1].monster->getenergy();
  energy2=being[index2].monster->getenergy();

  fightenergy=(fightenergy>max(energy1,energy2))?min(energy1,energy2):
    fightenergy;

#ifdef _DBG
  printf("%d, %d -> FIGHT!\n", being[index1].IDs, being[index2].IDs);
#endif
  fightenergy1=fightenergy;
  fightenergy2=fightenergy;

  energy1-=fightenergy1;
  energy2-=fightenergy2;

  if ((energy1>0.0)&&(energy2>0.0))
    if (energy1<energy2) {
      // begin 1 is moved away since it is the looser
      // random_move(index1);
    } else {
      // begin 2 is moved away since it is the looser
      random_move(index2);
      moveto(index1);
    }
  else
    if (energy1>0.0) {
      // being 2 is death, it is moved away and the update function is
      // called, so any strtegy to update lives and deaths will
      // work...
      random_move(index2);
      moveto(index1);
    } else {
      if (energy2>0.0) {
        // being 1 is death, it is moved away and the update function is
        // called, so any strtegy to update lives and deaths will
        // work...
        random_move(index1);
      } else {
        // they are both death: this is not possible! One of the two
        // must survive with a little amount of energy proportional to
        // the original energy (1/10)
        if (boost::random::uniform_01<>()(rndgen)<0.5) {
          fightenergy1=70*being[index1].monster->getenergy()/100;
          fightenergy2=being[index2].monster->getenergy();
          random_move(index1);
          //random_move(index2);
        } else {
          fightenergy2=70*being[index2].monster->getenergy()/100;
          fightenergy1=being[index1].monster->getenergy();
          random_move(index2);
          moveto(index1);
        }
      }
    }
  being[index1].monster->update(0, 0, -fightenergy1);
  being[index2].monster->update(0, 0, -fightenergy2);
  assert(check_invariant());

}

void TMondo::mate(int index1, int index2) {
  // a and b are indexes of the two figthing beings. The result of the
  // fighting may are these: 1) one of them dies 2) both fight, loose
  // energy, and the looser will be moved away in another cell

  // index1 and index2 are indexes of the two figthing beings. The result
  // of the fighting may are these: 1) one of them dies 2) both fight, loose
  // energy, and the looser will be moved away in another cell
  // The mating process involves many aspects:
  // 1) The number of children is a random number (binomial) but
  //    it may take into account the amount of manure in world
  //    (or in a neighbohrhood of the beings) to include a more
  //    global conservation law.
  // 2) The sons need to be placed somewhere, so the number of children
  //    have to be less than the number of available cells. Moreover, a
  //    way to find a free cell should be implemented.
  // 3) The unique being's ID needs to be assotiated to each child, an
  //    algorithm to do this is needed. In this sense I think that a
  //    stack (or a queue) made of IDs may help.

  int n,i,j,k; // number of children


  C_MATES++;
  n=boost::random::binomial_distribution<>(10,0.3)(rndgen);
  n=(n>(nfree-1))?nfree-1:n;
  n=((n_monsters+n)>maxmonsters)?maxmonsters-n_monsters:n;

  for (i=0; i<dim*dim; i++)
  	work[i]=i;
  std::random_shuffle(work.begin(), work.end(), p_myrandom);

  i=0;
  k=0;
#ifdef _DBG
  printf("%d, %d -> Generate %d children\n", being[index1].IDs, being[index2].IDs, n);
#endif
  for (j=0; j<n; j++) {
  	// there are at least n free cells...
    while (surface[work[i]].monster!=NULL) {
      i++;
#ifndef NDEBUG
      if (i>=dim*dim) {
        printf("No space left...\n");
        break;
      }
#endif
    }

    // Collect energy and mass across the world...
    // energy and mass came from manure!
    // if there is not enough energy around, the children is not spawned
    float needed=INITIALWEIGHT+INITIALENERGY/MASSENERGYFACTOR;
    float found=0.0,collected=0.0;//, totalfound;
    //while (collected<needed && found>=0) {
    do {
      //	while (collected<needed && totalfound>0) {
      // totalfound=0.0;
      for (int kk=0; kk<dim*dim; kk++) {
        found=min(1.0, surface[work[kk]].manure);
        surface[work[kk]].manure-=found;
        //if (found>0) break;
        //totalfound+=found;
        collected+=found;
        if (collected>=needed)
          break;
      }
    } while (collected<needed && found>0);

    if (collected==0.0) {
      break;
    }

    // there are at least n free beings...
    while (being[k].monster!=NULL)
      k++;

    being[k].IDs=IDstack.top();
    IDstack.pop();
    being[k].orientation=boost::uniform_smallint<>(0,3)(rndgen);
    //	if (surface[(work[i] / dim)*dim+(work[i] % dim)].monster!=NULL) {
    if (surface[work[i]].monster!=NULL) {
      printf("There is someone else... %d, %s\n", __LINE__, __FILE__);
      exit(1);
    }
    being[k].x=(work[i] / dim);
    being[k].y=(work[i] % dim);
#ifdef _DBG
    printf("%d -> NEW BEING\n", being[k].IDs);
#endif
    nfree--;
    n_monsters++;

    float initialweight=collected/2;
    float initialenergy=(collected-collected/2)*MASSENERGYFACTOR;

    C_BORNS++;
    being[k].monster = new TMonster(being[k].IDs, being[index1].monster, being[index2].monster,this, initialenergy, initialweight);
    surface[being[k].x*dim+being[k].y].monster_ID=being[k].IDs;
    surface[being[k].x*dim+being[k].y].monster=being[k].monster;
    surface[being[k].x*dim+being[k].y].index=k;

  }

  assert(check_invariant());

}

void TMondo::nothing(int id) {
  int monster;

#ifndef NDEBUG
  if ((monster=get_monster(id))==-1) {
    printf("Monster's id appears to be not a valid id %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }
#else
  monster=get_monster(id);
#endif

#ifdef _DBG
  printf("%d -> Doing nothing\n", id);
#endif
  being[monster].monster->update(0, 0, 0);

  assert(check_invariant());

}


void TMondo::try2mate(int id) {
  int completeaction, x, y, region, monster;
  TMonster *m1, *m2;

#ifndef NDEBUG
  if ((monster=get_monster(id))==-1) {
    printf("Monster's id appears to be not a valid id %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }
#else
  monster=get_monster(id);
#endif

  T_MATES++;

  if (!moveto(monster)) {
    // This means that there is some one...
    // The problem is: if there is no a being of the right sex...????

    x=being[monster].x;
    y=being[monster].y;

    completeaction=being[monster].monster->getnextaction();
    region=allaction2region(completeaction);
    get_coordinates(region, being[monster].orientation, x, y);

    m1=surface[x*dim+y].monster;
    m2=being[monster].monster;

    if (m1->getsex() != m2->getsex()) {
      // Mating may happen
      /*if ((genunf(0,MAXHORMONS)/PTM<m1->get_hormons())&&\
        (genunf(0,MAXHORMONS)/PTM<m2->get_hormons())) {*/
      if ( ( (beignsaround(id)/35.0) * MAXHORMONS/PTM<m1->get_hormons() )&&
           ( (beignsaround(id)/35.0) * MAXHORMONS/PTM<m2->get_hormons() ) ) { //non è preciso... lo so
        // Probability to mating is proportional to hormons level on
        // each being involved

        mate(get_monster(surface[x*dim+y].monster_ID), monster);

        // The update is made here
        m1->update(0, 1, 0);
        m2->update(0, 1, 0);


      } else {
        //try2fight(being[monster].IDs);
      }
    } else {
      try2fight(being[monster].IDs);
      // Mating cannot happen
    }
  }
  assert(check_invariant());
}

void TMondo::try2fight(int id) {
  // Fight occours with a certain probability. If there is no fight,
  // one of the two being should escape (this is not implemented yet)

  int completeaction, x, y, region, monster;
  int n1,n2;
  TMonster *m1, *m2;
  int idwantfight, otherwantfight;

  if ((monster=get_monster(id))==-1) {
    printf("Monster's id appears to be not a valid id %d, %s\n", __LINE__, __FILE__);
    exit(1);
  }


  T_FIGHTS++;

  if (!moveto(monster)) {
    // This means that there is some one...
    //	x=being[monster].x;
    //y=being[monster].y;

    completeaction=being[monster].monster->getnextaction();
    region=allaction2region(completeaction);

    // TODO (Mauro Bianco#1#): Removed the following condition, which I don't understand
    //	if (region>3) {
    x=being[monster].x;
    y=being[monster].y;
    get_coordinates(region, being[monster].orientation, x, y);

    if ((surface[x*dim+y].monster==NULL)) {
      printf("There is no monster there x=%d y=%d r=%d linre=%d, %s\n", 
             x, y, region, __LINE__, __FILE__);
      // TODO (Mauro Bianco#1#): Before the following "return" was an exit, but in a multi-cell region the being may jump into the wrong place
      return;
    }

    m1=surface[x*dim+y].monster;
    m2=being[monster].monster;

    idwantfight=(PTF<m2->get_propension2fight()/
                 (m1->get_propension2fight()+m2->get_propension2fight()))?0:1;

    otherwantfight=(PTF<m1->get_propension2fight()/
                    (m1->get_propension2fight()+m2->get_propension2fight()))?0:1;


	  if (otherwantfight) {
		n1=id;
		n2=surface[x*dim+y].monster_ID;

		fight(get_monster(n1),get_monster(n2));
		// Now it is not possible to kwon how to update
		// the monsters, so the update need to be done
		// into fight function
	  } else
		if (!otherwantfight) {
		  // other retires
		  if (surface[x*dim+y].monster_ID==-1)
			printf("cazzo");
		  random_move(get_monster(surface[x*dim+y].monster_ID));
		  being[monster].monster->update(0, 0, 0);

		}
	  //	}
  } else
    being[monster].monster->update(0, 0, 0);

  assert(check_invariant());
}


void TMondo::dead(int ID, bool natural /*=false*/) {
  assert(check_invariant());
  int monster=get_monster(ID);
  if(!being[monster].monster->isalive()) {
    TMonster* m=being[monster].monster;
    C_DEADS++;
    C_NAT_DEADS += (natural)?1:0;
    nfree++;
    n_monsters--;
    updatemanurewithdeath(monster);
    //DEADStack.push(being[monster].monster);
    IDstack.push(being[monster].IDs);
    being[monster].monster=NULL;
    being[monster].IDs=-1;
    surface[being[monster].x*dim+being[monster].y].monster=NULL;
    surface[being[monster].x*dim+being[monster].y].monster_ID=-1;
    surface[being[monster].x*dim+being[monster].y].index=-1;
    delete m;
  } else {
    printf("NON DEVE SUCCEDERE %d, %s\n", __LINE__, __FILE__);
  }
  assert(check_invariant());

}

inline void TMondo::compute_aggregate(float & totalfood,
                                      float & totalmanure,
                                      float & totalweight, 
                                      float & totalenergy, 
                                      float & totalhormons,
                                      float & totalhungry,
                                      float & invariant,
                                      float & avgage,
                                      int & histcode_len) const {
  
  for (int i=0; i<dim*dim;i++) {
    totalfood+=surface[i].food;
    totalmanure+=surface[i].manure;
  }

  for (int i=0; i<maxmonsters; i++) {
    if (being[i].monster!=NULL) {
      totalweight+=being[i].monster->getsize();
      totalenergy+=being[i].monster->getenergy();
      totalhormons+=being[i].monster->get_hormons();
      totalhungry+=being[i].monster->gethungry();
      avgage+=(float)being[i].monster->get_age();
      histcode_len+=being[i].monster->get_learnedinstr();
    }
  }
  invariant = totalweight+(totalenergy/MASSENERGYFACTOR)+totalfood+totalmanure;
}

inline bool TMondo::check_invariant(float eps) const {
  float totalfood=0;
  float totalmanure=0;
  float totalweight=0.;
  float totalenergy=0.;
  float totalhormons=0.;
  float totalhungry=0.;
  float avgage=0.0;
  float invariant=0.0;
  int hcl;
  
  compute_aggregate(totalfood,
                    totalmanure,
                    totalweight,
                    totalenergy,
                    totalhormons,
                    totalhungry,
                    invariant,
                    avgage,
                    hcl);

  printf("energy = %e - invariant = %e \n", invariant, initial_invariant);
  if (fabs(initial_invariant-invariant) > eps) 
	  return false;
  else
    return true;
}

bool TMondo::NextStep(/*stupidclass &*/){
  static bool firstStep = true;

  static int monster,i;
  // static FILE *output = 0, *statfile = 0;
  float totalfood, totalmanure, totalhormons;
  float totalenergy, totalweight, totalhungry, avgage, invariant;
  //double totalMETAgoodness;
  //double totalHISTgoodness;
  static int stepOutCount=0, histcode_len=0;
  static FILE* statfile;
  static std::vector<int> localwork;

  if (firstStep){
	  localwork.resize(maxmonsters);
	  for (i=0; i<maxmonsters; i++)
		localwork[i]=i;
	  statfile=fopen("statdata.dat","w");

    totalfood=0;
    totalmanure=0;
    totalweight=0.;
    totalenergy=0.;
    totalhormons=0.;
    totalhungry=0.;
    avgage=0.0;
    initial_invariant=0.0;
    //to compute invariant
    compute_aggregate(totalfood,
                      totalmanure,
                      totalweight,
                      totalenergy,
                      totalhormons,
                      totalhungry,
                      initial_invariant,
                      avgage,
                      histcode_len);

#ifdef WORLDDUMP
	  //FILE* file=fopen("mondo.txt","w");
#endif

	  firstStep = false;
  }

  if (nfree<dim*dim) {
    time++;

    if (time>timestepstorun) {
      return false;
    }


    totalfood=0;
    totalmanure=0;
    totalweight=0.;
    totalenergy=0.;
    totalhormons=0.;
    totalhungry=0.;
    avgage=0.0;
    invariant=0.0;
    histcode_len=0;
    compute_aggregate(totalfood,
                      totalmanure,
                      totalweight,
                      totalenergy,
                      totalhormons,
                      totalhungry,
                      invariant,
                      avgage,
                      histcode_len);

    fprintf(statfile, "%d\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%e\t%d\n", 
            n_monsters, totalfood, totalmanure, totalenergy/MASSENERGYFACTOR, 
            totalweight, totalhormons, totalhungry, invariant, 
            C_BORNS, C_DEADS, C_NAT_DEADS, C_FIGHTS, C_MATES, T_FIGHTS, T_MATES, avgage/n_monsters, histcode_len);
    fflush(statfile);

    for (i=0; i<dim*dim; i++)
      foodevolution(i);

    if (time%10==0) //shuffle only sometime
	  {
      std::random_shuffle(localwork.begin(), localwork.end(), p_myrandom);
	  }

    for (i=0; i<maxmonsters; i++) {

      monster=localwork[i];
      if (being[monster].monster!=NULL) {

#ifndef NDEBUG
		  if (!being[monster].monster->isalive()) {
#ifdef DBG
          printf("%d -> DEAD!\n", being[monster].IDs);
#endif
          assert(false);
          assert(true);
          nfree++;
          n_monsters--;
          updatemanurewithdeath(monster);
          delete(being[monster].monster);
          IDstack.push(being[monster].IDs);
          being[monster].monster=NULL;
          being[monster].IDs=-1;
          surface[being[monster].x*dim+being[monster].y].monster=NULL;
          surface[being[monster].x*dim+being[monster].y].monster_ID=-1;
          surface[being[monster].x*dim+being[monster].y].index=-1;
        } else 
#endif
		  {
#ifdef DBG
          printf("Processing %d\n", being[monster].IDs);
#endif
          being[monster].monster->elect();

          being[monster].monster->execute();
		  if (being[monster].monster!=NULL) {
            if (being[monster].monster->isalive())
              if (being[monster].monster->update(0,0,0))
				assert(being[monster].monster->instrstackempty());
          } 

          //stupida.fai();
        } // else of if (!being[monster].monster->isalive())
      } // if (being[monster].monster!=NULL)
    } // for (i=0; i<maxmonsters; i++) {

    stepOutCount=(stepOutCount+1)%stepOut;

	} else { // if (nfree<dim*dim) {
	  fclose(statfile);

	  return false;
	}
	return true;
}


const std::vector<TCella>& TMondo::GetCelle() const {
  return surface;
}

const std::vector<TBeing>& TMondo::GetBeings() const {
  return being;
}
