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

#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_
// One bit comparing operator

#include <vector>
typedef unsigned Tinstr;

#define EQ 2
#define NE 3
#define LT 0
#define GT 1

// The following flags are intended to identify two kind of "goodness"
// of an instruction. An instruction may be good or not in the sense
// that it lead to good or bad behavior. If a being is quering a
// region and then (if the clause is true) it move on the other side,
// it may be not a good intruction. It depends, howerve, on the kind
// of event that are present into the iquired region. Another example
// is this: suppose that the being is not hungry and that decides to
// eat. This is not a good choice, since this may lead to obesity. So
// the goodness MUST take into account also the comparing operator and
// the event associated with an instruction. This is not a trivial
// problem. Moreover, it is difficult tu rank the instructions in
// order to have a good set of good instructions preserving the
// freedom (which is a key factor of this project). To allow differnt
// ranking models, we decided to include two differnt ranking schema:
// one is move driven and the other is kind driven. The rank of an
// instruction is a four bit number obrained by the concatenation of
// the "action ranking" and the "move ranking". The action ranking
// comes from the goodness of an action related with a kind (1 bit),
// while the movement ranking comes from the movement associated with
// a destination region (3 bits (0...4)). The two models refer to the
// order in which this two (binary) numbers are concatenated. In the
// move driven model the kind rank is placed as the least significant
// bit, while in the kind driven it is the most significant bit. The
// difference is this: If the kind rank is placed as the most
// significant bit, then a sorting (in increasing order) of the
// instructions by goodness lead to the low kind ranked instructions
// to came before all the others, while, on the other side, if kind
// rank is placed as least significant bit, then we prefer movements
// on the right direction even though the action taken is not good.
// The second model may be better if the destination region is
// considered independently from the action (i.e., first a destination
// cell is chosen and then the action is chosen). In this case,
// infact, we may force a region which is good and then hoping the
// action is good. The first model is the contrary. Since the being
// need to eat, for instance, then is not important where it does, the
// important is that it does! Hence, these two models seem to be in
// some sense equivalent, only experience will demonstrate which is
// the better (it there is).  NOTE: How it is not true, in general,
// that a kind is good to perform an action if the operand is not set
// to the appropriate value, it is also not true that the direction of
// the movement is bad if it is on the opposite side with respect to
// the region referred to the instruction. The possibility to escape,
// for instance, may preserve the life of the being more than a fight
// (wanted or not by the being itself).
#define MOVEDRIVEN_SCORE 0
#define KINDDRIVEN_SCORE 1

int get_mutations();

int is_food_instr(Tinstr instr);

int is_metab_instr(Tinstr instr);

int get_kind(Tinstr instr);

int get_region(Tinstr instr);

int get_op(Tinstr instr);

int get_op2(Tinstr instr);

int get_threshold(Tinstr instr);

int get_value(Tinstr instr);

int get_action(Tinstr instr);

int get_move(Tinstr instr);

int get_allaction(Tinstr instr);

int allaction2region(int completeaction);

int allaction2action(int completeaction);

Tinstr mutate(Tinstr instr);

void describe_instruction(Tinstr instr);

double set_goodness(Tinstr* instrs, int n, int model, double&, double&, double&);

// implementation

#endif
