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

#ifndef _INSTRUCTIONS_IMPL_H_
#define _INSTRUCTIONS_IMPL_H_

inline int is_food_instr(Tinstr instr) {
  if (((instr>>30)&3)==HUNGRY) return 1;
  return 0;
}

inline int is_metab_instr(Tinstr instr) {
  return !is_food_instr(instr);
}

inline int get_kind(Tinstr instr) {
  int mask=3;
  return (instr>>30)&mask;
}


inline int get_region(Tinstr instr) {

  int mask=15;
  return (instr>>26)&mask;
}

inline int get_op(Tinstr instr) {

  int mask=1;
  return (instr>>15)&mask;
}

inline int get_op2(Tinstr instr) {
  // To be used when LT and GT are used for equality, too
  int mask=1;
  //if (is_food_instr(instr))
  return (instr>>14)&mask;
  return -1;
}

inline int get_threshold(Tinstr instr) {

  int mask=15;
  return (instr>>22)&mask;
}

inline int get_value(Tinstr instr) {

  if (is_food_instr(instr)) {
	int mask=3;
	return (instr>>12)&mask;
  } else {
	int mask=15;
	return (instr>>11)&mask;
  }
}

inline int get_move(Tinstr instr) {
  int mask=15;

	return (instr>>18)&mask;
}

inline int get_action(Tinstr instr) {
  int mask=3;

	return (instr>>16)&mask;

}

inline int get_allaction(Tinstr instr) {
  int mask=63;

	return (instr>>16)&mask;

}

inline int allaction2region(int completeaction) {
  return (completeaction>>2)&15;
}

inline int allaction2action(int completeaction) {
  return (completeaction)&3;
}

#endif
