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

#include <stdio.h>
#include <stack>
#include "datastructures.h"
#include "instructions_impl.h"
//#include "physicandstats.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

extern random_gererator rndgen;

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((b)>(a))?(a):(b))
#define CHOOSE(a,b) ((boost::random::uniform_01<>()(rndgen)>1/2)?(a):(b))

//#define LOGISTIC(x,R) ((x/R)*(x/R)*(3.0-2.0*(x/R))*R)
#define LOGISTIC(x,R) (x)

extern int RATIONAL_BIAS;
extern int CODELENGTH;
extern int HISTORYLENGHT;
extern float MASSENERGYFACTOR;
extern float HISTMETARATIO;
extern float MAXWEIGHT;
extern float MAXHORMONS;
extern float INITIALENERGY;
extern float INITIALWEIGHT;
extern float MINVITA;
extern int TTL;
extern bool DO_RANDOM;

#ifdef _MYROUNDF
inline int roundf(float f) {
  if (f-floor(f)<0.5)
	return (int)floor(f);
  else
	return (int)floor(f)+1;
}
#endif

std::vector<Tinstr>& TMonster::get_metabcode() {
  // return pointer to metabcode
  return metabcode;
}

std::vector<Tinstr>& TMonster::get_histcode(int& hcode_len) {
  // return pointer to histcode and
  hcode_len=n_histcode;
  return histcode;
}

int TMonster::getnextaction() {
  return nextaction;
}

TMonster::~TMonster() {}

TMonster::TMonster(int id, TMondo *mondo) 
  : n_metabcode(CODELENGTH)
  , maxactions(64)
  , metabcode(CODELENGTH)
  , histcode(HISTORYLENGHT)
  , metabactions(maxactions)
  , histactions(maxactions)
  , counter_actions(maxactions)
  , iso_count(maxactions)
  , INSTRstack(CODELENGTH) // CONSERVATIVE
{
  int i;
  world = mondo;
  ID=id;
  age=0;
  timestamp=world->get_time();
  ALIVE=1;

  sex=(boost::random::uniform_01<>()(rndgen)>=0.5)?MALE:FEMALE;

  n_histcode=0;

  for (i=0; i<n_metabcode; i++)
	metabcode[i]=2*boost::random::uniform_int_distribution<>(0,214748356)(rndgen)+boost::random::uniform_int_distribution<>(0,214748356)(rndgen);

  // set_goodness(metabcode,n_metabcode);

  metabcoef = 1;
  histcoef=HISTMETARATIO*metabcoef;

  hormons=0.0;
  hungry=0.0;
  weight=INITIALWEIGHT;
  energy=INITIALENERGY;

//  horfreq=gennor(5*0.0628, 0.0628/10);
//  hormod=gennor(5*0.0628*10, 0.0628);
  horperiod=boost::random::normal_distribution<>(6.0,1.0)(rndgen);
  enefreq=boost::random::normal_distribution<>(5*0.0628, 0.0628/10)(rndgen);
  weifreq=boost::random::normal_distribution<>(5*0.0628, 0.0628/10)(rndgen);
  hunfreq=boost::random::normal_distribution<>(20*0.0628, 2*0.0628)(rndgen);
  learnedintructions=0;
}

TMonster::TMonster(int id, TMonster *mother, TMonster *father, TMondo *mondo, float initialenergy, float initialweight)
  : n_metabcode(CODELENGTH)
  , maxactions(64)
  , metabcode(n_metabcode)
  , histcode(HISTORYLENGHT)
  , metabactions(maxactions)
  , histactions(maxactions)
  , counter_actions(maxactions)
  , iso_count(maxactions)
  , INSTRstack(CODELENGTH) // CONSERVATIVE
{
  int i,tot;
  int father_hcode_len, mother_hcode_len;

  world = mondo;
  ID=id;
  age=0;
  timestamp=world->get_time();
  ALIVE=1;

  sex=(boost::random::uniform_01<>()(rndgen)>=0.5)?MALE:FEMALE;

  n_histcode=0;

  std::vector<Tinstr>& father_mcode=father->get_metabcode();
  std::vector<Tinstr>& mother_mcode=mother->get_metabcode();
  std::vector<Tinstr>& father_hcode=father->get_histcode(father_hcode_len);
  std::vector<Tinstr>& mother_hcode=mother->get_histcode(mother_hcode_len);
  
  i=0;
  tot=0;
  while (i<father_hcode_len) {
	if (boost::random::uniform_01<>()(rndgen)<0.80)
	  metabcode[tot++]=mutate(father_hcode[i]);
	i++;
  }

  i=0;
  while (i<mother_hcode_len) {
	if (boost::random::uniform_01<>()(rndgen)<0.80)
	  metabcode[tot++]=mutate(mother_hcode[i]);
	i++;
  }

  i=0;
  while (tot<n_metabcode) {
    if (boost::random::uniform_01<>()(rndgen)<0.50) {
      // from father
      metabcode[tot++]=mutate(father_mcode[i++]);
    } else {
      // from mother
      metabcode[tot++]=mutate(mother_mcode[i++]);
    }
  }

  metabcoef = 1;
  histcoef=HISTMETARATIO*metabcoef;

  hormons=0.0;
  hungry=0.0;
  weight=initialweight;
  energy=initialenergy;

//  horfreq=(father->get_horfreq()+mother->get_horfreq())/2;
//  horfreq=gennor(horfreq, horfreq/10);
  horperiod=CHOOSE(father->get_horperiod(),mother->get_horperiod());
  horperiod=boost::random::normal_distribution<>(horperiod, horperiod/10)(rndgen);
//  hormod=(father->get_hormod()+mother->get_hormod())/2;
//  hormod=gennor(hormod, hormod/10);
  enefreq=CHOOSE(father->get_enefreq(),mother->get_enefreq());
  enefreq=boost::random::normal_distribution<>(enefreq, enefreq/10)(rndgen);
  weifreq=CHOOSE(father->get_weifreq(),mother->get_weifreq());
  weifreq=boost::random::normal_distribution<>(weifreq, weifreq/10)(rndgen);
  hunfreq=CHOOSE(father->get_hunfreq(),mother->get_hunfreq());
  hunfreq=boost::random::normal_distribution<>(hunfreq, hunfreq/10)(rndgen);
  learnedintructions=0;
}


int TMonster::compare(float a, int op, int b) {

  switch (op) {
  case LT:
	return (a)<=(float)b;
  case GT:
	return (a)>=(float)b;
  case EQ: // not used
	return ((a)<=(float)b) && ((a)>=(float)b);
  case NE: // not used
	return ((a)>(float)b) || ((a)<(float)b);
  }
  assert(0);
  exit(12);
}


int TMonster::test_instr(Tinstr instr, int &action) {

  float field;

  //  printf("\n%d ----> Testing instructions (%d)\n", ID, instr);

  switch (get_kind(instr)) {
  case HUNGRY:
    field=hungry; break;
  case HORMONS:
    field=hormons; break;
  case ENERGY:
    field=energy; break;
  case WEIGHT:
    field=weight; break;
  }

  if (compare(field, get_op(instr), get_threshold(instr)) 
      && world->evalue(ID, instr)) {
    action=get_allaction(instr);
    return 1;
  } else {
    action=DONOTMOVE;
    return 0;
  }

}


void TMonster::elect() {
  int metab, hist, i, k;
  float maxvalue;
  int action;

  if (!DO_RANDOM) {
    //printf("%d -> Chosing next action\n", ID);
    // This shoul nexver happen
	assert(ALIVE);
	assert(INSTRstack.empty());
#ifndef NDEBUG
    if (!INSTRstack.empty()) {
#ifdef _DBG
      printf("INSTRstack is not empty!!! %d, %s\n", __LINE__, __FILE__);
#endif
        INSTRstack.clear();
    }
#endif

    for (i=0; i<maxactions; i++) {
      metabactions[i]=0;
      //senseactions[i]=0;
      histactions[i]=0;
    }

#ifdef _DBG
    printf("%d -> testing hist instructions %d, %s\n", ID, __LINE__, __FILE__);
#endif
    for (hist=0; hist<n_histcode; hist++) {// ciclo sulle istruzioni
      // metaboliche
      if (test_instr(histcode[hist], action)) {
        histactions[action]++;
        INSTRstack.push(histcode[hist]);
      }
    }

	if ((RATIONAL_BIAS*n_histcode <= HISTORYLENGHT) || // Probably here something about histmetaratio...
		(2*RATIONAL_BIAS*INSTRstack.size() < n_histcode) || 
		boost::random::bernoulli_distribution<>(1/RATIONAL_BIAS/2)(rndgen)) 
	  {


#ifdef _DBG
	  printf("%d -> testing metab instructions %d, %s\n", ID, __LINE__, __FILE__);
#endif
	  for (metab=0; metab<n_metabcode; metab++) {// ciclo sulle
		// istruzioni metaboliche
		//printf("%d %d\r", metab, action);
		if (test_instr(metabcode[metab], action)) {
#ifdef _DBG
		  if (action>64) {
			printf("Ciao... %d\n", action);
			exit(100);
		  }
#endif
		  metabactions[action]++;
		  INSTRstack.push(metabcode[metab]);
		}
	  }
	}
#ifdef _DBG
    printf("%d -> counting actions %d, %s\n", ID, __LINE__, __FILE__);
#endif
    for (i=0; i<maxactions; i++)
      counter_actions[i]=metabcoef * metabactions[i] +
        histcoef * histactions[i];
    //sensecoef * senseactions[i] +


    // compute stastisctics on potentially taken actions
    maxvalue=0;
#ifdef _DBG
    printf("%d -> computing stats %d, %s\n", ID, __LINE__, __FILE__);
#endif
    for (i=0; i<maxactions; i++) {
      if (counter_actions[i] > maxvalue) 
        {
          maxvalue = counter_actions[i];
        }
    }

#ifdef _DBG
    printf("%d -> Chosing next action (%d)\n", ID, maxvalue);
#endif
    if (maxvalue==0) {
	  assert(INSTRstack.empty());
      nextaction=DONOTMOVE;
    } else {
      /* Here every statistical consideration can be done. It is
         possible, for instance, to chose among actions whose count is
         less than max with a very low probability, but this may lead
         to excessive rondomization */

      /* We have to take into account, however, the case in which the
         max count is accomplished by more than one action and chose
         randomly among them 
      */

        assert(!INSTRstack.empty());
        k=0;
		int how_many=0;
        for (i=0; i<maxactions; i++)
          if (counter_actions[i] == maxvalue) { 
			iso_count[k++]=i;
			++how_many;
		  }

		if (how_many>1)
		  k = boost::uniform_smallint<>(0,k-1)(rndgen);
		else
		  k=0;
        nextaction=iso_count[k];
#ifdef _DBG
        printf("%d -> next instr = %d -- %d, %s\n", ID, nextaction, __LINE__, __FILE__);
#endif
	}

  } else {//DO_RANDOM
    nextaction = boost::random::uniform_smallint<>(0,(1<<7)-1)(rndgen);	
  }
}


void TMonster::execute() {

  // Take nextaction and try to execute it

  switch (allaction2action(nextaction)) {
  case EAT:
    // This is the most simple case: call the function to move and
    // then eat. The function moveto may result in not move at
    // all. This version of the code try to perform the action even
    // though the being has not moved.
    if (world->moveto(world->get_monster(ID)))
      world->eat(ID);
    else
      world->eat(ID);
    break;
  case MATE:
    world->try2mate(ID);
    break;
  case FIGHT:
    world->try2fight(ID);
    break;
  case NOTHING:
    if (world->moveto(world->get_monster(ID)))
      world->nothing(ID);
    else
      world->nothing(ID);
    break;
  default:
    printf("Doing default!!! %d, %s\n", __LINE__, __FILE__);
    world->nothing(ID);
    break;
  }
}


bool TMonster::update(float food, int mate, float inc_energy) {
  // food: quantity of food eaten

  // mate: 0 if not mating had happen, <>0 otherwise

  // energy: amount of energy gained or loose (If a fighting had
  // happen this is the amount of energy loosed).
#ifdef _DBG
  printf("%d -> UPDATING!\n", ID);
#endif

  // the following are the value of quantity from the last update in the
  // same timestep
  static float prevenergy, prevweight, prevhungry, prevhormons;
  static float oldenergy,oldhungry,oldhormons,oldweight;
  float k=MASSENERGYFACTOR; // conversion between mass and anergy E=MK
  float dEt=0.0;
  bool first_update = false;

  assert(ALIVE);

  //if (ALIVE) {
    if (age!=world->get_time()-timestamp) {
      // first update for this time step
      age=world->get_time()-timestamp;
	  first_update = true;
    }
    if (age>TTL) {
      ALIVE=0;
      // need to acocunt for the food already took away from world - conservation law.
      weight+=food; //energy has not yet been removed from the world.
      world->dead(ID,true); // true == natural death
      //world->updatewithshit(energy/k+weight,ID);

      return false;
    }
	
	if (first_update) {
	  if (food>0) {
		if (hungry<1/4*MAXWEIGHT) { // not beneficial to eat - all food becomes shit
		  dEt=(MAXWEIGHT-hungry)*boost::random::uniform_real_distribution<>(0.0,0.2*((float)age)/((float)TTL))(rndgen)+food; //min(-k/1.5,-energy*genunf(0,age/(TTL))); // variation of energy due to time      age=world->get_time()-timestamp;
		  food=0;
		} else {
		  dEt=(MAXWEIGHT-hungry)*boost::random::uniform_real_distribution<>(0.0,0.2*((float)age)/((float)TTL))(rndgen);
		}
	  } else { // not a food action
		dEt=(MAXWEIGHT-hungry)*boost::random::uniform_real_distribution<>(0.0,0.2*((float)age)/((float)TTL))(rndgen);
	  }
	} else { // not the first update
	  if (hungry<1/4*MAXWEIGHT) { // not beneficial to eat - all food becomes shit
		dEt=food; //min(-k/1.5,-energy*genunf(0,age/(TTL))); // variation of energy due to time      age=world->get_time()-timestamp;
		food=0;
	  } 
	}

	


	  oldenergy=energy;
	  oldhungry=hungry;
	  oldhormons=hormons;
	  oldweight=weight;
	  prevenergy=energy;
	  prevweight=weight;
	  prevhungry=hungry;
	  prevhormons=hormons;

    //inc_energy-=weight/10; // each time step cost an amount of energy
    // proportional to weight

    float dEf, dWf, dEe, dWe; // variations of energy and weight due to food or energy

    float partition;
 
	assert((food>=0.0 && inc_energy==0.0) || (food==0 && inc_energy<=0));
	inc_energy-=dEt; //inc_energy is negative, dEt is positive
  inc_energy=-min(-inc_energy, energy);
	partition=boost::random::uniform_01<>()(rndgen);
  dEf=partition*food*k;
  dWf=(1-partition)*food;
  partition=boost::uniform_smallint<>(0,1)(rndgen)?1.0:boost::random::uniform_01<>()(rndgen);
  dEe=partition*inc_energy;
  dWe=(1-partition)*inc_energy/k;
	assert(weight>=0);
	assert(dWe<=0 && dWf>=0);
	if (weight<(-dWe)) {
		// partition was too aggressive
		// -inc_energy is less than or equel to energy so it is ok to move weight away without breaking energy rule
		//dWe is greater than the change in weight, since dWf should be 0
		dEe-=((-dWe)-weight)*k;
		dWe=-weight;
	}

    energy+=dEf+dEe;
    weight+=dWf+dWe;

    float shit=-(inc_energy)/k; // amount of shit

    if (weight>MAXWEIGHT) {
      shit+=weight-MAXWEIGHT;
      weight=MAXWEIGHT;
    }

    if (energy/k>MAXWEIGHT) {
      shit+=energy/k-MAXWEIGHT;
      energy=MAXWEIGHT*k;
    }

    assert(energy>=0);
    assert(weight>=0);

	  assert(shit>=0);

	  if (shit>0.0)
      world->updatewithshit(shit,ID);

      // Using MASSTOENERGY as coefficient....
    hormons=((energy/k+hormons)>MAXHORMONS)?MAXHORMONS:(energy/k+hormons);
    //hormons=MAXHORMONS;
    if (mate) hormons/=3;
#define STOKA 1
    hungry=LOGISTIC((STOKA*MAXWEIGHT-energy/k-weight)/STOKA,16.0);//(energy-oldenergy)/k+weight-oldweight;

    if ((energy<=0.0)||(weight<=0.0) || (energy+weight<MINVITA) ) {
      ALIVE=0;
      world->dead(ID);
	  return false;
    } else {
      // ajusting balance among energy and mass
      float MEgap=(energy-(weight*k))/2;
      if (MEgap>0.0) {
        float newG;
        //do
        //    newG=genexp(max(MEgap/4.0,1.0));
        //while (newG>(MEgap));
        newG=boost::random::uniform_real_distribution<>(0.0,MEgap)(rndgen);
        energy=energy-newG;
        weight=weight+newG/k;
      } else {
        if (MEgap<0.0) {
          float newG;
          //do
          //    newG=genexp(max(-MEgap/4.0,1.0));
          //while (newG>(-MEgap));
          newG=boost::random::uniform_real_distribution<>(0.0,-MEgap)(rndgen);
          energy=energy+newG;
          weight=weight-newG/k;
        }
		assert(energy>=0);
		assert(weight>=0);
      }
	  learn(energy-prevenergy, hungry-prevhungry, hormons-prevhormons, weight-prevweight);
    }
    return true;
	//} else {
	///  assert(0);
	//}

}

void TMonster::learn(float denergy, float dhungry, float dhormons, float dweight) {
  Tinstr i;

  if (!DO_RANDOM) {
#ifdef _DBG
    printf("%d -> LEARNING\n",ID);
#endif
    if ((dhormons<=0) || (denergy>=0) /*|| (hungry<=0)*/) {
      // LEARN: In this version the being learns all the previously taken actions
      // Possibility to record "paths" are not considered yet!!!!

#ifdef _DBG
      printf("%d -> stack size: %d\n", ID, INSTRstack.size());
#endif
      while (!INSTRstack.empty()) {
        i=INSTRstack.top();
        if (get_allaction(i) == nextaction) { //learn only instructions with right action
          if (boost::random::bernoulli_distribution<>(0.85)(rndgen)) {
            n_histcode=(n_histcode+1)%HISTORYLENGHT;
            histcode[n_histcode]=i;
            learnedintructions++;
#ifdef _DBG
            //printf("%d -> learn instruction %d\r", ID, i);
#endif
            //describe_instruction(i);
          }
        }
        INSTRstack.pop();
      }
 	  assert(INSTRstack.empty());

#ifdef _DBG
      printf("\n");
#endif
    } else {
      INSTRstack.clear();
 	  assert(INSTRstack.empty());
    }
  } //DO_RANDOM
  else
	{
	  assert(INSTRstack.empty());
	}
}

float TMonster::get_propension2fight() {
  // deve dipendere da quanti esserini ci sono attorno a lui
  return (energy+weight*MASSENERGYFACTOR)*world->beignsaround(ID);
}

bool TMonster::isalive() {
  return ALIVE;
}

float TMonster::get_hormons() {
  return hormons;
}

int TMonster::isheat() {
  return hormons>4.0;
}

int TMonster::getsex() {
  return sex;
}

float TMonster::getenergy() {
  return energy;
}

float TMonster::getsize() { // il peso, o la taglia 0..MAXSIZE
  return weight;
}

//float TMonster::get_horfreq() {
 // return horfreq;
//}
// Frequency of hormons modulant (for heat periods)
//float TMonster::get_hormod() {
//  return hormod;
//}

// Frequency of energy
float TMonster::get_enefreq() {
  return enefreq;
}

// Frequency of weight
float TMonster::get_weifreq() {
  return weifreq;
}

// Frequency of hungry
float TMonster::get_hunfreq() {
  return hunfreq;
}
