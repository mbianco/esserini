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
#include "instructions_impl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int get_difference_of_distance(int region, int move) {

  // return the difference between the distance between (0,0) and the
  // center of region 'region' and the (0,0) and the destination cell
  // of 'move'. The distance taken into account is the "infinity norm"
  // distance, which can be proved to be the minimum number of steps
  // the being must do to go from one cell to another.

  int x1, y1, x2, y2, d1, d2;

  // The first thing is retrive the coordinates of the two involved
  // cells.
  switch (move) {
  case 0:
  case 1:
  case 2:
  case 3:
	x1=0;
	y1=0;
	break;
  case 4:
  case 5:
	x1=-1;
	y1=1;
	break;
  case 6:
	x1=0;
	y1=1;
	break;
  case 7:
  case 8:
	x1=1;
	y1=1;
	break;
  case 9:
	x1=1;
	y1=0;
	break;
  case 10:
  case 11:
	x1=1;
	y1=-1;
	break;
  case 12:
	x1=0;
	y1=-1;
	break;
  case 13:
  case 14:
	x1=-1;
	y1=-1;
	break;
  case 15:
	x1=-1;
	y1=0;
	break;
  default:
	return -1;
  }

  switch (region) {
  case 0:
	x2=0;
	y2=0;
	break;
  case 1:
	x2=-1;
	y2=1;
	break;
  case 2:
	x2=0;
	y2=1;
	break;
  case 3:
	x2=1;
	y2=1;
	break;
  case 4:
	x2=-2;
	y2=2;
	break;
  case 5:
	x2=-1;
	y2=2;
	break;
  case 6:
	x2=0;
	y2=2;
	break;
  case 7:
	x2=1;
	y2=2;
	break;
  case 8:
	x2=2;
	y2=2;
	break;
  case 9:
	x2=-2;
	y2=0;
	break;
  case 10:
	x2=2;
	y2=0;
	break;
  case 11:
	x2=-2;
	y2=-2;
	break;
  case 12:
	x2=2;
	y2=-2;
	break;
  case 13:
	x2=0;
	y2=-2;
	break;
  case 14:
	x2=-3;
	y2=0;
	break;
  case 15:
	x2=3;
	y2=0;
	break;
  default:
	return -1;
  }


  d1=(abs(x1)>abs(y1))?abs(x1):abs(y1);
  d2=(abs(x2)>abs(y2))?abs(x2):abs(y2);
  return abs(d1-d2);

}


int not_coherent_directions(int region, int move) {
  // This is a qualitative function. Given region and move it:
  // return 0 if the movement indicated by move is toward region
  // return 1 if the movement is more toward that opposite

  // return 2 if the movement is indifferent w.r.t. region (the
  // distance is not decreased, hence, this is the return value event
  // if no movements is involved in move (i.e., only rotation))

  // return 3 if the movement is more opposite that toward
  // return 4 if the movement is opposite to direction


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
  */

  switch (move) {
  case 0:
  case 1:
  case 2:
  case 3:
	if (region == 0)
	  return 0;
	return 2;
	break;
  case 4:
  case 5:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 0;
	case 2:
	case 6:
	case 14:
	case 9:
	  return 1;
	case 3:
	case 7:
	case 8:
	  return 2;
	case 10:
	case 15:
	case 11:
	  return 3;
	case 13:
	case 12:
	  return 4;
	default: return -1;
	}
	break;
  case 6:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 1;
	case 2:
	case 6:
	  return 0;
	case 14:
	case 9:
	  return 3;
	case 3:
	case 7:
	case 8:
	  return 1;
	case 10:
	case 15:
	  return 2;
	case 11:
	case 12:
	case 13:
	  return 4;
	default: return -1;
	}
	break;
  case 7:
  case 8:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 3;
	case 2:
	case 6:
	  return 2;
	case 14:
	case 9:
	  return 4;
	case 3:
	case 7:
	case 8:
	  return 0;
	case 10:
	case 15:
	  return 1;
	case 12:
	  return 3;
	case 11:
	case 13:
	  return 4;
	default: return -1;
	}
	break;
  case 9:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 3;
	case 2:
	case 6:
	case 13:
	  return 2;
	case 9:
	case 14:
	  return 4;
	case 3:
	case 7:
	case 8:
	  return 0;
	case 10:
	  return 1;
	case 11:
	  return 3;
	case 12:
	  return 2;
	default: return -1;
	}
  case 10:
  case 11:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 4;
	case 2:
	case 6:
	case 14:
	case 9:
	  return 3;
	case 3:
	case 7:
	case 8:
	  return 2;
	case 10:
	case 15:
	  return 1;
	case 12:
	  return 0;
	case 11:
	  return 3;
	case 13:
	  return 2;
	default: return -1;
	}
	break;
  case 12:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 3;
	case 2:
	case 6:
	  return 4;
	case 14:
	case 9:
	  return 2;
	case 3:
	case 7:
	case 8:
	  return 3;
	case 10:
	case 15:
	  return 2;
	case 11:
	case 12:
	  return 1;
	case 13:
	  return 0;
	default: return -1;
	}
	break;
  case 13:
  case 14:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 2;
	case 2:
	case 6:
	  return 3;
	case 14:
	case 9:
	  return 1;
	case 3:
	case 7:
	case 8:
	  return 4;
	case 10:
	case 15:
	  return 4;
	case 11:
	  return 0;
	case 12:
	  return 2;
	case 13:
	  return 1;
	default: return -1;
	}
	break;
  case 15:
	switch (region) {
	case 0:
	  return 4;
	case 1:
	case 4:
	case 5:
	  return 1;
	case 2:
	case 6:
	  return 2;
	case 14:
	case 9:
	  return 0;
	case 3:
	case 7:
	case 8:
	  return 3;
	case 10:
	case 15:
	  return 4;
	case 11:
	  return 1;
	case 12:
	  return 2;
	case 13:
	  return 3;
	default: return -1;
	}
  default:
	return -1;
  }
}

int not_coherent_actions(int kind, int action) {

  // return 0 if kind and action are coherent (e.g., HUNGRY and EAT)
  // return 1 otherwise
  //
  // This is assumed to be a "positive", it means that if kind is
  // ENERGY it means "I'm robust", so fighting should be a succesful
  // action. In order to compute the goodness of one instruction it
  // must be taken into account the operator involved (and also the
  // threshold for finer evaluations).

  switch (kind) {
  case HUNGRY:
	if (action == EAT)
	  return 0;
	return 1;
  case HORMONS:
	if (action == MATE)
	  return 0;
	return 1;
  case ENERGY:
	if (action == FIGHT)
	  return 0;
	return 1;
  case WEIGHT:
	if (action == FIGHT)
	  return 1;
	return 0;
  default:
	return -1;
  }
}

double set_goodness(Tinstr* instrs, int n, int model,  
    double &gmin, double &gavg, double &gmax) {
  // Return the average goodness
  int i;
  int instr;
  int kind_rank, move_rank, rank;
  double avgrank=0;

  if (n==0) {
      gmin=0.0;
      gavg=0.0;
      gmax=0.0;
      return 0.0;
  }

  gmax=-1.0;
  gmin=10001001.0;
  for(i=0; i<n; i++) {
	instr=instrs[i];
	kind_rank=not_coherent_actions(get_kind(instr), get_action(instr));
	move_rank=get_difference_of_distance(get_region(instr), get_move(instr));

	// Error checking
	if (move_rank==-1) {
	  printf("Error while not_coherent_directions(%d, %d)\n",get_region(instr), get_move(instr));
	  exit(-1);
	}
	if (kind_rank==-1) {
	  printf("Error while not_coherent_actions(%d, %d)\n",get_kind(instr), get_action(instr));
	  exit(-1);
	}

	//printf("Move rank = %d, Kind rank = %d\n",move_rank,kind_rank);
	if (is_food_instr(instr)) {
	  if (!(get_op(instr)==get_op2(instr))) {
		if (get_op(instr)==LT)
		  kind_rank=1^kind_rank;
	  }
	} else
	  if ((get_op(instr)==LT || get_op(instr)==NE) && (move_rank>=2))  kind_rank=1^kind_rank;

	if (!is_food_instr(instr)) {
	  switch (get_value(instr)) {
	  case BIGSAMESEX:
	  case BIGOTHERSEX:
		move_rank=4-move_rank; // complement
		break;
	  case OTHERSEXES:
	  case SAMESEXES:
	  case MIX:
		move_rank=move_rank+1;
		move_rank=move_rank>4?4:move_rank;
	  }
	}
	//printf("Move rank = %d, Kind rank = %d\n",move_rank,kind_rank);
	if (move_rank>=5) exit(1);
	if (model == MOVEDRIVEN_SCORE) {
	  rank=(move_rank<<1)+(kind_rank);
	} else {
	  rank=(kind_rank<<2)+move_rank;
	}
	//printf("Move rank = %d, Kind rank = %d, Final rank = %d\n",move_rank,kind_rank, rank);

	instr=((instr>>3)<<3)+(rank);
	avgrank+=(double)rank;
    if (((double)rank) < gmin)
        gmin = rank;
    if (((double)rank) > gmax)
        gmax = rank;
  }
  gavg=avgrank/(double)n;
  return avgrank/(double)n;
}


//  int set_goodness(Tinstr *instrs, int n, int model) {
//    int i;
//    int instr;
//    int kind_rank, move_rank, rank;

//    for(i=0; i<n; i++) {
//  	instr=instrs[i];
//  	kind_rank=not_coherent_actions(get_kind(instr), get_action(instr));
//  	move_rank=not_coherent_directions(get_region(instr), get_move(instr));

//  	// Error checking
//  	if (move_rank==-1) {
//  	  printf("Error while not_coherent_directions(%d, %d)\n",get_region(instr), get_move(instr));
//  	  exit(-1);
//  	}
//  	if (kind_rank==-1) {
//  	  printf("Error while not_coherent_actions(%d, %d)\n",get_kind(instr), get_action(instr));
//  	  exit(-1);
//  	}

//  	//	printf("Move rank = %d, Kind rank = %d\n",move_rank,kind_rank);
//  	if ((get_op(instr)==LT || get_op(instr)==NE) && (move_rank>=2))  kind_rank=1-kind_rank;

//  	if (!is_food_instr(instr)) {
//  	  switch (get_value(instr)) {
//  	  case BIGMALE:
//  	  case BIGFEMALE:
//  		move_rank=4-move_rank; // complement
//  		break;
//  	  case FEMALES:
//  	  case MALES:
//  	  case MIX:
//  		move_rank=move_rank+1;
//  		move_rank=move_rank>4?4:move_rank;
//  	  }
//  	}
//  	//	printf("Move rank = %d, Kind rank = %d\n",move_rank,kind_rank);

//  	if (model == MOVEDRIVEN_SCORE) {
//  	  rank=(move_rank<<1)+kind_rank;
//  	} else {
//  	  rank=(kind_rank<<3)+move_rank;
//  	}
//  	//	printf("Move rank = %d, Kind rank = %d, Final rank = %d\n",move_rank,kind_rank, rank);

//  	instr=((instr>>4)<<4)+rank;
//    }

//    return rank;
//  }

