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

#include <string>
#include "datastructures.h"
// #ifndef IBM
// #include "pgetopt.h"
// #endif

#include <boost/program_options.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int RATIONAL_BIAS;
int CODELENGTH;
int HISTORYLENGHT;
float MASSENERGYFACTOR;
float HISTMETARATIO;
float MAXWEIGHT;
float MAXHORMONS;
float MAXFOOD;
float INITIALWEIGHT;
float INITIALENERGY;
float MINVITA;
int TTL;
float PTF; // propension to fight
float PTM; // propension to mate
bool DO_RANDOM=false;
float MANURE_PROB;
float GROWTH_RATE;
float EAT_AMOUNT;

boost::mt11213b rndgen;


int main(int argc, char **argv) {

  TMondo *world;
  long int seed1=343212, rnd_reset_time;
  int gridX=10, gridY=10, esseriniN=30, stepOut=10;
  int TIMES;

  boost::program_options::options_description desc("Usage");
  desc.add_options()
    ("xdim,x", boost::program_options::value<int>(&gridX)->default_value(10), "X dimension of grid (int)\n")
    ("ydim,y", boost::program_options::value<int>(&gridY)->default_value(10), "Y dimension of grid (int)\n")
    ("n-beings,n", boost::program_options::value<int>(&esseriniN)->default_value(30), "Number of esserini (int)\n")
    ("n-steps-before-viz,o", boost::program_options::value<int>(&stepOut)->default_value(10), "Number of steps when printing output for viz (int)\n")
    ("random_seed,s", boost::program_options::value<long int>(&seed1)->default_value(343212), "First seed (ranlib) (int)\n")
    ("auto-reset-seed,a", boost::program_options::value<long int>(&rnd_reset_time)->default_value(((unsigned)1<<(unsigned)31)-1), "If set the random seed is re-set to current time every arg iterations - not implemented yet")
    ("metab-code-length,C", boost::program_options::value<int>(&CODELENGTH)->default_value(1<<15), "Length of metabolic code (int)\n")
    ("hist-code-lenght,H", boost::program_options::value<int>(&HISTORYLENGHT)->default_value(1<<12), "Length of historical code (int)\n")
    ("enery-mass-ratio,E", boost::program_options::value<float>(&MASSENERGYFACTOR)->default_value(1.0), "Energy-mass ratio (e=mE) (float) (best is 1, try to not use it)\n")
    ("learnt-over-metabolic,R", boost::program_options::value<float>(&HISTMETARATIO)->default_value(2.0), "Ratio between learnt istrs. and metabolic instrs. (float) \n")
    ("minimum-life,u", boost::program_options::value<float>(&MINVITA)->default_value(0.1), "When energy+weight is less that this value, the esserino dies. (float) \n")
    ("initial-energy,I", boost::program_options::value<float>(&INITIALENERGY)->default_value(3.0), "Initial Energy for beings (best below 16) (float) \n")
    ("initial-weigth,w", boost::program_options::value<float>(&INITIALWEIGHT)->default_value(3.0), "Initial weight for beings (best below 16) (float) \n")
    ("duration-of-life,L", boost::program_options::value<int>(&TTL)->default_value(50), "Duration of life! (int)\n")
    ("propension-to-fight,F", boost::program_options::value<float>(&PTF)->default_value(0.5), "Propensiont to fight! (0-1, float)\n")
    ("propension-to-mate,M", boost::program_options::value<float>(&PTM)->default_value(0.5), "Propension to mate! (0-1, float)\n")
    ("steps-to-execute,T", boost::program_options::value<int>(&TIMES)->default_value(100), "Number of timestep to execute (<0 is for unbounded) (int)\n")
    ("manure-prob,p", boost::program_options::value<float>(&MANURE_PROB)->default_value(0.5), "Probability to put manure ina cell at the beginning of the world (float)\n")
    ("growth-rate,g", boost::program_options::value<float>(&GROWTH_RATE)->default_value(0.3), "Max rate of growth of food (wrt manure) [-GROWTH_RATE*0.25)*surface[i].food, GROWTH_RATE*surface[i].manure] (float)\n")
    ("eat-amount,e", boost::program_options::value<float>(&EAT_AMOUNT)->default_value(1.0), "Max amount of food that is eaten at a time (float)\n")
    ("rational-bias,b", boost::program_options::value<int>(&RATIONAL_BIAS)->default_value(1.0), "This is complex. In the evaluation of the codes, learned instructions are evaluated firts. If\n*1* Instructions learned are less than <length of historical code>/b, or\n*2* The stack contains already <length of historical code>/(2*b) or\n*3* A bernoulli variable with probability 1/(2*b) is true\n then the metabolic code is evaluated, otherwise they are not evaluated. By setting b to 1, the metab code is always evaluated, otherwise it is not. These parameters are integers.\n")
    ("random,r", "All beings behaves randomly not responding to any code\n")
    ("help,?", "Produce help\n");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);  

  if (vm.count("help")) {
    std::cout << "Esserini  Copyright (C) 2011-...  Mauro Bianco\n";
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY\n";
    std::cout << "See <http://www.gnu.org/licenses/> for liocensing information\n";
    std::cout << "This is free software, and you are welcome to redistribute it\n";
    std::cout << "under certain conditions;\n";
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("random")) {
	DO_RANDOM=true;
  }

  if (vm.count("rational-bias")) {
	if (RATIONAL_BIAS <= 0) {
	  std::cout << " ERROR: rationa-biasl must be integer greater than 1\n\n";
	  std::cout << desc << "\n";
	  return 1;
	}
  }

  // CODELENGTH=1<<15;
  // HISTORYLENGHT=1<<12;
  // MASSENERGYFACTOR=1.;
  // HISTMETARATIO=2.;
  MAXWEIGHT=15.;
  MAXHORMONS=15.;
  MAXFOOD=5;
  // //MAXENERGY=16.;
  // INITIALWEIGHT=3.;
  // INITIALENERGY=3.*MASSENERGYFACTOR;
  // TTL=50;
  // PTF=0.5;
  // PTM=0.5;
  // int TIMES=100;

  world = new TMondo(gridX,esseriniN, stepOut);

  world->timetoexecute(TIMES);
  while (world->NextStep())
	{ }

  delete world;
  return 0;
}
