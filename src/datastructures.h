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

#ifndef __DATASTRUCTURES_H__
#define __DATASTRUCTURES_H__

#include "instructions.h"
#include <stack>
#include <vector>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/bernoulli_distribution.hpp>

#define MAXSIZE 16 // Max size of a being

typedef boost::mt11213b random_gererator;

// Orientations
#define NORTH 0
#define SOUTH 2
#define EAST 1
#define WEST 3
// The order is important, indeed, summing 1 (mod 3) to orientations
// means turn 90 degree right, summing 2 means turn 180 degree,
// summing 3 means turn 90 degree left


// Kind of sex
#define MALE 0
#define FEMALE 1

// Kind of revelated beings while seeing into one region
#define NOBODY 0
#define BIGSAMESEX 1
#define BIGOTHERSEX 2
#define SMALLSAMESEX 3
#define SMALLOTHERSEX 4
#define HEATSAMESEX 5
#define HEATOTHERSEX 6
#define SAMESEXES 7
#define OTHERSEXES 8
#define MIX 9
#define MIXSAMEHEAT 10
#define MIXOTHERHEAT 11
#define MIXBOTHHEAT 12
#define SAMEHEATS 13
#define OTHERHEATS 14
#define NOBODY2 15

// Metabolic entities
#define HUNGRY 0
#define HORMONS 1
#define ENERGY 2
#define WEIGHT 3

// Actions
#define EAT 0
#define FIGHT 1
#define MATE 2
#define FRONT 0
#define LEFT 1
#define NOTHING 3

// Movements
#define RIGHT 2
#define BEHIND 3
#define FRONTLEFTLEFT 4
#define FRONTLEFTFRONT 5
#define FRONTFRONT 6
#define FRONTRIGHTFRONT 7
#define FRONTRIGHTRIGHT 8
#define RIGHTRIGHT 9
#define BEHINDRIGHTRIGHT 10
#define BEHINDRIGHTBEHIND 11
#define BEHINDBEHIND 12
#define BEHINDLEFTBEHIND 13
#define BEHINDLEFTLEFT 14
#define LEFTLEFT 15

// Special Actions
#define DONOTMOVE 3


// Others
//#define MAXMANURE 100 // quantit‡ massima di fertilizzante // NO LONGER USED
#define INITIALMANURE 3 // quantit‡ iniziale di fertilizzante
// La probabilit‡ che nasca cibo in una cella Ë INITIALMANURE/MAXMANURE
//#define FOODLIFETIME 10 // # of timesteps NO LONGER USED
//#define CODELENGTH  (1<<15)
//#define HISTORYLENGHT (1<<12)
//#define MAXFOOD 10
//#define MASSENERGYFACTOR 10

//int CODELENGTH;
//int HISTORYLENGHT;
//int MASSENERGYFACTOR;

class mystack {
  std::vector<Tinstr> store;
  size_t len;
public:
  mystack(size_t size): store(size), len(0) {}
  void push(Tinstr i) {
    if (len > store.size()) {
      store.push_back(i);
      ++len;
    } else {
      store[len++]=i;
    }
    assert(len<=store.size());
  }

  void pop() {len--; assert(len>=0);}
  Tinstr &top() {assert(len>0); return store[len-1];}
  bool empty() {assert(len>=0 && len<=store.size()); return len==0;}
  void clear() {len=0;}
  size_t size() {assert(len>=0); return len;}
};

inline int modulo(int a, int b) {
  int c=a%b;
  return (c<0)?b+c:c;
}

class TMondo;

class TMonster {
private:

  int ID, age, sex, timestamp;
  TMondo *world;
  bool ALIVE; // true if the being is alive...

  bool do_learn; // variable to state if learning should happen or not. If no instruction were evaluated to true, no learning should be done.

  float energy,hungry,hormons,weight;
  int n_metabcode;
  int maxactions; // # of different possible actions (6bit->64)
  int n_histcode;
  int nextaction; // action elected for execution
  std::vector<Tinstr> metabcode;
  // int *sensecode, n_sensecode;
  std::vector<Tinstr> histcode;

  float sensecoef, metabcoef, histcoef; // weigths associated with each
  // kind of instructions
  std::vector<int> metabactions, histactions; //, *senseactions; // 0...maxactions-1
  std::vector<float> counter_actions; // see elect()
  std::vector<int> iso_count; // see elect()

  mystack INSTRstack; // structures used to store potentially good
                            // instructions to enhance the historical mamory
  void learn(float denergy, float dhungry, float dhormons, float dweight);
  // Uses delta values to take from INSTRstack the instructions to learn

  int test_instr(Tinstr instr, int &action); // Test if clouse is
  // possible ant return the deriven action
  int compare(float a, int op, int b); // This is used to compare a and
  // b with op

  // These are "physical" parameters
  // Frequency of hormons
  //float horfreq;
  // Frequency of hormons modulant (for heat periods)
  //float hormod;
  // periodo of oscillation for hormon
  float horperiod;
  // Frequency of energy
  float enefreq;
  // Frequency of weight
  float weifreq;
  // Frequency of hungry
  float hunfreq;

  int learnedintructions;
public:

  TMonster(int id, TMondo *mondo);
  TMonster(int id, TMonster *mother, TMonster *father, TMondo *mondo, float initialenergy, float initialweight);
  ~TMonster();
  int isheat();
  int getsex();
  float gethungry() {return hungry;}
  float getenergy();
  float get_propension2fight ();
  float getsize(); // il peso, o la taglia 0..MAXSIZE
  float get_hormons();
  bool update(float food, int mate, float energy); // Function to update . returns true id being is alive after the operation
 // all the methabolical variables
  void elect(); // Function to elect one instruction
  void execute(); // function to execute the next instruction
  int getnextaction();
  bool isalive();

  std::vector<Tinstr>& get_metabcode();  // return pointer to metabcode
  std::vector<Tinstr>& get_histcode(int& hcode_len); // return pointer to histcode and
  // length of the code into hcode_len

//  float get_horfreq();
  // Frequency of hormons modulant (for heat periods)
//  float get_hormod();

  float get_horperiod() {return horperiod;}
  // Frequency of energy
  float get_enefreq();
  // Frequency of weight
  float get_weifreq();
  // Frequency of hungry
  float get_hunfreq();
  int get_learnedinstr() {return learnedintructions;}
  int get_ID() {return ID;}
  int get_age() {return age;}
  void annichil() {weight=0.0; energy=0.0;}
  double get_META_goodness(int model) {double a,b,c; return set_goodness(&metabcode[0], n_metabcode, model, a,b,c);}
  double get_HIST_goodness(int model) {double a,b,c; return set_goodness(&histcode[0], n_histcode, model,a,b,c);}

#ifndef NDEBUG
  bool instrstackempty() {return INSTRstack.empty();}
#endif
};


struct TCella{
	int monster_ID;
	float food;
	int timestamp; // per sapere quando  nato il cibo nella cella...
	float manure;
	int index; // index within array being...
	TMonster *monster;
  };

struct TBeing {
	int IDs;
	int orientation;
	int x,y; // posizione
	TMonster *monster;
  };


class TMondo {

private:

  float initial_invariant;

  int time;

  std::stack<int> IDstack; // stack containing monsters unique IDs

  int nfree; // number of free cells

  int timestepstorun;

  int dim;
  int maxmonsters;

  int stepOut;

  std::vector<TCella> surface;
  std::vector<int> work;
  int n_monsters;

  std::vector<TBeing> being;

  void setup(); // inizializza il mondo fisico (surface)
  //void update(); // aggiorna il mondo fisico (surface)
  void foodevolution(int i);
  /*  void randperm(int n); */

  void updatemanurewithdeath(int monster);
  // index of monster within the being array
  void fight(int index1, int index2);
  void mate(int index1, int index2);
  void get_coordinates(int region, int orientation, int &x, int &y);
  void random_move(int index);

  int T_MATES; // tries to mate
  int T_FIGHTS; // tries to fight
  int C_BORNS; // count borns
  int C_DEADS; // count deads
  int C_FIGHTS; // count fights
  int C_MATES; // count mates
  int C_NAT_DEADS; // count how many beings died of aging

public:

  TMondo(int n, int m, int stepOut);

  int beignsaround(int monster);

  void timetoexecute(int TIMES) {timestepstorun=TIMES;}
  int evalue(int id, Tinstr instr); // dato l'essere id vedi se la
  // clausola in instr e` vera (almeno la parte geografica).

  int moveto(int id); // uses next action
  //int act(int id); // uses next action
  void eat(int id);
  void nothing(int id);
  int get_time() const;
  void try2mate(int id);
  void try2fight(int id);
  void dead(int ID, bool natural=false); // call when a being die!
  void mainloop();
    // this was private, but the moveto function has changed interface and
    // this need to be called from monster.cc
  int get_monster(int id); // on return id contains the
  void updatewithshit(float shit, int ID);
  bool NextStep(/*stupidclass&*/);
  const std::vector<TCella>& GetCelle() const;
  const std::vector<TBeing>& GetBeings() const;

  bool check_invariant(float =1.e-2) const;
  void return_to_world(float);
private:
  void compute_aggregate(float&, float&, float&, float&, float&, float&, float&, float&, int&) const;
};


#endif
