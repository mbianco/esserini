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
#include <stdlib.h>

#include "datastructures.h"
#include "instructions_impl.h"
//#include "physicandstats.h"

static const float bitmutationprob=1e-5;
static int instrmutations=0;

int get_mutations() {
  return instrmutations;
}

Tinstr mutate(Tinstr instr) {

//  if (genunf(0,1.0)<mutationprobability) { // Mutation probability
return instr;
	//int i = ignbin(21,bitmutationprob); // bitmutationprob has not to be considered
                                  // "given that mutation happens"
	//int k;
	//for (int j=0; j<i; j++) {
    //  instrmutations++;
	//  k=ignuin(11,31);
	//  instr=instr^(1<<k);
	//}
  //}
  return instr;
}

void describe_instruction(Tinstr instr) {

  printf("If ");
  switch (get_kind(instr)) {
  case HUNGRY:
	printf("hungry: "); break;
  case HORMONS:
	printf("hormons: "); break;
  case ENERGY:
	printf("energy: "); break;
  case WEIGHT:
	printf("weight: "); break;
  }

  switch (get_op(instr)) {
  case LT:
	printf("less than %d, and within region %d ", 
		   get_threshold(instr), get_region(instr));
	break;
  case GT:
	printf("greater than %d, and within region %d ", 
		   get_threshold(instr), get_region(instr));
	break;
  case EQ:
	printf("equal to %d, and within region %d ", 
		   get_threshold(instr), get_region(instr));
	break;
  case NE:
	printf("not equal to %d, and within region %d ", 
		   get_threshold(instr), get_region(instr));
	break;
  }

  if (is_food_instr(instr)) {
	switch (get_op2(instr)) {
	case LT:
	  printf("food is less than %d ", 
			 get_value(instr));
	  break;
	case GT:
	  printf("food is greater than %d ", 
			 get_value(instr));
	  break;
	case EQ:
	  printf("food is equal to %d ", 
			 get_value(instr));
	  break;
	case NE:
	  printf("food is not equal to %d ", 
			 get_value(instr));
	  break;
	}
  } else {
	switch (get_value(instr)) {
	case NOBODY :
	  printf("there is nobody "); break;
	case BIGSAMESEX :
	  printf("there is a big of my sex "); break;
	case BIGOTHERSEX :
	  printf("there is big of other sex "); break;
	case SMALLSAMESEX :
	  printf("there is small my sex "); break;
	case SMALLOTHERSEX :
	  printf("there is small other sex "); break;
	case HEATSAMESEX :
	  printf("there is a my sex being in heat "); break;
	case HEATOTHERSEX :
	  printf("there is a other sex being in heat "); break;
	case SAMESEXES :
	  printf("there are beings of my sex (>1) "); break;
	case OTHERSEXES :
	  printf("there are beings of other sex (>1) "); break;
	case MIX :
	  printf("there are mixed beings "); break;
	case MIXSAMEHEAT :
	  printf("there are mixed beings and of my sex in heat "); break;
	case MIXOTHERHEAT :
	  printf("there are mixed beings and of other sex in heat "); break;
	case MIXBOTHHEAT :
	  printf("there are mixed beings and some are in heat"); break;
	case SAMEHEATS :
	  printf("there are beings of my sex (>1) in heat"); break;
	case OTHERHEATS :
	  printf("there are beings of other sex (>1) in heat"); break;
	case NOBODY2 :
	  printf("there is nobody"); break;
	}
  }

  printf("then, \n");

  switch (get_move(instr)) {
  case FRONT:
	printf("remain there "); break;
  case LEFT:
	printf("round left "); break;
  case RIGHT:
	printf("round right "); break;
  case BEHIND:
	printf("round behind "); break;
  case FRONTLEFTLEFT:
	printf("move to front-left and round left "); break;
  case FRONTLEFTFRONT:
	printf("move to front-left and watch to front "); break;
  case FRONTFRONT:
	printf("move to front and watch to front "); break;
  case FRONTRIGHTFRONT:
	printf("move to  to front-rigth and watch to front "); break;
  case FRONTRIGHTRIGHT:
	printf("move to  to front-right and turn right "); break;
  case RIGHTRIGHT:
	printf("move to right and turn right "); break;
  case BEHINDRIGHTRIGHT:
	printf("move to behind-right and turn right "); break;
  case BEHINDRIGHTBEHIND:
	printf("move to behind-right and turn behind "); break;
  case BEHINDBEHIND:
	printf("move to behind and round behind "); break;
  case BEHINDLEFTBEHIND:
	printf("move to behind-left and turn behind "); break;
  case BEHINDLEFTLEFT:
	printf("move to behind-left and turn left "); break;
  case LEFTLEFT:
	printf("move to left and turn left "); break;
  }

  switch (get_action(instr)) {
  case EAT:
	printf("and eat.\n"); break;
  case FIGHT:
	printf("and figth.\n"); break;
  case MATE:
	printf("and mate.\n"); break;
  case NOTHING:
	printf("and do nothing.\n"); break;
  }
    double gmin, gmax, gavg;
  printf("Calculated Goodness in move driven model: %e (small is better)\n", set_goodness(&instr, 1, MOVEDRIVEN_SCORE, gmin, gavg, gmax));
  printf("Calculated Goodness in kind driven model: %e (small is better)\n", set_goodness(&instr, 1, KINDDRIVEN_SCORE, gmin, gavg, gmax));
}

