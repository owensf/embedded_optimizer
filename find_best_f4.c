#include <stdio.h>
#include <string.h>
#include "f4s.h"
// Fred Owens
/* find_best_f4.c attempts to find the highest win probability final four given that other final four picks in the pool are known. The algorithm has two broad parts:
1) Determine all teams that have a F4 probability > 10% or that have been picked by someone else in the pool. Create every permutation from these eligible teams
2) Iterate through list of final fours, calculating the win probability for each. Save win probability and final four if it's the highest win probability
*/

// test_generator is a data structure for keeping track of "test" final four scenarios.  
void init_test_gen(test_generator * test_gen){
	for (uint16_t i = 0; i < NUM_ELEMENTS_BRACKET; i++){
		test_gen -> test_f4[i] = 0;
	}
	for (uint16_t i = 0; i < NUM_TEAMS_REGION + 1; i++){
		test_gen -> test_south_teams[i] = 0;
		test_gen -> test_east_teams[i] = 0;
		test_gen -> test_midwest_teams[i] = 0;
		test_gen -> test_west_teams[i] = 0;
	}

	test_gen -> num_south_teams = 0;
	test_gen ->  num_east_teams = 0;
	test_gen -> num_midwest_teams = 0;
	test_gen -> num_west_teams = 0;
	test_gen -> south_tracker = 0;
	test_gen -> east_tracker = 0;
	test_gen -> midwest_tracker = 0;
	test_gen -> west_tracker = 0;
	test_gen -> num_scenarios = 0;
	test_gen -> num_scenarios_calculated = 0;
	test_gen -> scenario_tracker = 0;
}

// find the winning pool in a given scenario by calculating the number of points each bracket gets in that scenario. given_f4 is the scenario, f4s_list is the list of the f4s, and winners keeps track of which bracket won
// the "winners" array is necessary because there can easily be multiple winners
uint16_t find_winner_scenario(uint16_t given_f4[NUM_ELEMENTS_BRACKET], uint16_t f4s_list[NUM_BRACKETS][NUM_ELEMENTS_BRACKET], uint16_t winners[NUM_BRACKETS+1]){
	uint16_t i, j;
	//static uint16_t winners[NUM_BRACKETS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t winners_tracker = 0;
	uint16_t max_points = 0;
	for (i=0; i < NUM_BRACKETS+1; i++){
		int points = 0;
		for(j=0; j < NUM_ELEMENTS_BRACKET; j++){
			uint16_t team = given_f4[j];
			//TODO: can make this better with some sort of shifting arrangement
			if (j == 0) {
				points += (team == f4s_list[i][j]) * 4;
			}
			else if (j <= 2){
				points += (team == f4s_list[i][j]) * 2;
			}
			else {
				points += (team == f4s_list[i][j]);
			}

		}
		if (points == max_points){
			winners[winners_tracker] = i;
			winners_tracker++;
		}
		else if (points > max_points){
			winners[0] = i;
			winners_tracker = 1;
			max_points = points;
		}
	}
	// make everything beyond the winner index 0
	for (i=winners_tracker;i< NUM_BRACKETS+1;i++){
		winners[i] = 0;
	}
	return winners_tracker;
}
// Find the highest-probability F4 given the f4_picks
float find_best_f4(uint16_t f4_picks[NUM_BRACKETS][NUM_ELEMENTS_BRACKET]){
	f4_struct f4;
	test_generator test_gen;
	float total_prob = 0.0;
	int num_scenarios = 0;
	double max_test_prob = 0.0;
	uint16_t best_f4[NUM_ELEMENTS_BRACKET];
	uint16_t f4s_plus_test[NUM_BRACKETS+1][NUM_ELEMENTS_BRACKET];

	for (uint16_t i = 0; i < NUM_BRACKETS; i++){
		for (uint16_t j = 0; j < NUM_ELEMENTS_BRACKET; j++){
			f4s_plus_test[i][j] = f4_picks[i][j];
		}
	}
	// last bracket is "test bracket" initialized to 0
	for (uint16_t i = 0; i < NUM_ELEMENTS_BRACKET; i++){
		best_f4[i] = 0;
		f4s_plus_test[NUM_BRACKETS][i] = 0;
	}
	double full_probs[NUM_TEAMS+4][NUM_ROUNDS];
	init_test_gen(&test_gen);
	// populate the test bracket
	find_test_teams(f4_picks, &test_gen);
	// stops when the # of test scenarios has been completely iterated through
	while(generate_test_scenario(&test_gen) == 1){
		init_f4_struct(&f4);
		for (uint16_t i = 0; i < NUM_ELEMENTS_BRACKET; i++){
			f4s_plus_test[NUM_BRACKETS][i] = test_gen.test_f4[i];
		}
		// set up &f4s. Have an array element for each unique team. Then have one element at the end for all "unpicked" scenarios
		find_unique_elements(f4s_plus_test, &f4);
		// unpicked teams needs to be kept track of because those result in ties
		generate_unpicked_probs(&f4);
		// calcualte probabilities each f4 pick makes champ, wins champ, etc. Make last element into generic "untracked" 
		calculate_full_probs(full_probs, &f4);
		uint16_t winners[NUM_BRACKETS+1]; // need + 1 because we need to compare test bracket
		// iterate through each f4 scenario. See who wins in each scenario, and what the probability of that scenario is. Add the probability of that scenario to the winner's win prob
		while(generate_f4_scenario(&f4)==1){
			// calculate probability of a given scenario
			calculate_prob_scenario(full_probs, &f4);
			// sometimes there's multiple winners
			uint16_t num_winners = find_winner_scenario(f4.given_f4, f4s_plus_test, winners);
			// total_prob is just for debugging
			total_prob += f4.prob_scenario;
			num_scenarios++;
			for(uint16_t i = 0; i<num_winners; i++){
				f4.winner_probs[winners[i]] += f4.prob_scenario/((double)num_winners);
			}
		}
		// store best bracket
		if (f4.winner_probs[NUM_BRACKETS] > max_test_prob){
			max_test_prob = f4.winner_probs[NUM_BRACKETS];
			for (uint16_t i = 0; i < NUM_ELEMENTS_BRACKET; i++){
				best_f4[i] = test_gen.test_f4[i];
			}
		}
	}
	return max_test_prob;
}


void calculate_full_probs(double full_probs[NUM_TEAMS+4][NUM_ROUNDS], f4_struct *f4){
	for (uint16_t i = 0; i < NUM_TEAMS; i++){
		for(uint16_t j = 0; j < NUM_ROUNDS; j++){
			full_probs[i][j] = probs[i][j];
		}
	}
	for (uint16_t i = 0; i < 4; i++){
		for(uint16_t j = 0; j < NUM_ROUNDS; j++){
			full_probs[NUM_TEAMS+i][j] = f4->unpicked_probs[i][j];
		}
	}
}


void calculate_prob_scenario(double full_probs[NUM_TEAMS+4][NUM_ROUNDS], f4_struct *f4){

	f4->prob_scenario = full_probs[f4->given_f4[0]-1][CHAMP_INDEX];
	if (f4->given_f4[0] == f4->given_f4[1]){
		f4->prob_scenario *= full_probs[f4->given_f4[2]-1][RU_INDEX];
	}
	else{
		f4->prob_scenario *= full_probs[f4->given_f4[1]-1][RU_INDEX];
	}
	if (f4->given_f4[1] == f4->given_f4[4]){
		f4->prob_scenario *= full_probs[f4->given_f4[3]-1][F4_INDEX];
	}
	else{
		f4->prob_scenario *= full_probs[f4->given_f4[4]-1][F4_INDEX];
	}
	if (f4->given_f4[2] == f4->given_f4[5]){
		f4->prob_scenario *= full_probs[f4->given_f4[6]-1][F4_INDEX];
	}
	else{
		f4->prob_scenario *= full_probs[f4->given_f4[5]-1][F4_INDEX];
	}
}


void generate_unpicked_probs(f4_struct * f4){
	// -1 because unpicked is included in there
	for (uint16_t i = 1; i <= MAX_SOUTH; i++){
		int found = 0;
		for (uint16_t j = 0; j < f4->num_south_teams-1; j++){
			if (f4->unique_south_teams[j]==i){
				found = 1;
			}
		}
		if (found == 0){
			f4->unpicked_probs[0][CHAMP_INDEX]+= probs[i-1][CHAMP_INDEX];
			f4->unpicked_probs[0][RU_INDEX]+= probs[i-1][RU_INDEX];
			f4->unpicked_probs[0][F4_INDEX]+= probs[i-1][F4_INDEX];
		}
	}
	for (uint16_t i = MAX_SOUTH+1; i <= MAX_EAST; i++){
		int found = 0;
		for (uint16_t j = 0; j < f4->num_east_teams-1; j++){
			if (f4->unique_east_teams[j]==i){
				found = 1;
			}
		}
		if (found == 0){
			f4->unpicked_probs[1][CHAMP_INDEX]+= probs[i-1][CHAMP_INDEX];
			f4->unpicked_probs[1][RU_INDEX]+= probs[i-1][RU_INDEX];
			f4->unpicked_probs[1][F4_INDEX]+= probs[i-1][F4_INDEX];
		}
	}
	for (uint16_t i = MAX_EAST+1; i <= MAX_MIDWEST; i++){
		int found = 0;
		for (uint16_t j = 0; j < f4->num_midwest_teams-1; j++){
			if (f4->unique_midwest_teams[j]==i){
				found = 1;
			}
		}
		if (found == 0){
			f4->unpicked_probs[2][CHAMP_INDEX]+= probs[i-1][CHAMP_INDEX];
			f4->unpicked_probs[2][RU_INDEX]+= probs[i-1][RU_INDEX];
			f4->unpicked_probs[2][F4_INDEX]+= probs[i-1][F4_INDEX];
		}
	}
	for (uint16_t i = MAX_MIDWEST+1; i <= MAX_WEST; i++){
		int found = 0;
		for (uint16_t j = 0; j < f4->num_west_teams-1; j++){
			if (f4->unique_west_teams[j]==i){
				found = 1;
			}
		}
		if (found == 0){
			f4->unpicked_probs[3][CHAMP_INDEX]+= probs[i-1][CHAMP_INDEX];
			f4->unpicked_probs[3][RU_INDEX]+= probs[i-1][RU_INDEX];
			f4->unpicked_probs[3][F4_INDEX]+= probs[i-1][F4_INDEX];
		}
	}
}


void init_f4_struct(f4_struct *f4){
	f4->num_south_teams = 0;
	f4->num_east_teams = 0;
	f4->num_midwest_teams = 0;
	f4->num_west_teams = 0;
	f4->south_tracker = 0;
	f4->east_tracker = 0;
	f4->midwest_tracker = 0;
	f4->west_tracker = 0;
	f4->num_scenarios = 0;
	f4->num_scenarios_calculated = 0;
	f4-> scenario_tracker = 0;
	f4-> prob_scenario = 0.0;
	for (uint16_t i =0; i < NUM_BRACKETS+1; i++){
		f4->winner_probs[i]=0.0;
	}
	for (uint16_t i = 0; i < 4; i++){
		for (uint16_t j=0; j < CHAMP_INDEX+1; j++){
			f4->unpicked_probs[i][j] = 0.0;
		}
	}
}


uint16_t generate_test_scenario(test_generator * test_gen){

	if (test_gen->num_scenarios_calculated < test_gen->num_scenarios){
		test_gen->test_f4[6] = test_gen->test_south_teams[test_gen->south_tracker];
		test_gen->test_f4[5] = test_gen->test_east_teams[test_gen->east_tracker];
		test_gen->test_f4[4] = test_gen->test_midwest_teams[test_gen->midwest_tracker];
		test_gen->test_f4[3] = test_gen->test_west_teams[test_gen->west_tracker];
		test_gen->num_scenarios_calculated++;

		switch(test_gen->scenario_tracker){
			// South winner, MW runner-up
			case(0):{
				test_gen->test_f4[0]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[2]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[1]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// south winner, west runnerup
			case(1):{
				test_gen->test_f4[0]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[2]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[1]=test_gen->test_west_teams[test_gen->west_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// east winner, midwest runnerup
			case(2):{
				test_gen->test_f4[0]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[2]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[1]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// east winner, west runnerup
			case(3):{
				test_gen->test_f4[0]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[2]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[1]=test_gen->test_west_teams[test_gen->west_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// midwest winner, south runnerup
			case(4):{
				test_gen->test_f4[0]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->test_f4[2]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[1]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// midwest winner, east runnerup
			case(5):{
				test_gen->test_f4[0]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->test_f4[2]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[1]=test_gen->test_midwest_teams[test_gen->midwest_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// west winner, south runnerup
			case(6):{
				test_gen->test_f4[0]=test_gen->test_west_teams[test_gen->west_tracker];
				test_gen->test_f4[2]=test_gen->test_south_teams[test_gen->south_tracker];
				test_gen->test_f4[1]=test_gen->test_west_teams[test_gen->west_tracker];
				test_gen->scenario_tracker++;
				break;
			}
			// west winner, east runnerup
			case(7):{
				test_gen->test_f4[0]=test_gen->test_west_teams[test_gen->west_tracker];
				test_gen->test_f4[2]=test_gen->test_east_teams[test_gen->east_tracker];
				test_gen->test_f4[1]=test_gen->test_west_teams[test_gen->west_tracker];
				// increment trackers
				test_gen->scenario_tracker = 0;
				if (test_gen->west_tracker >= test_gen->num_west_teams-1){
					test_gen->west_tracker = 0;
					if(test_gen->midwest_tracker >= test_gen->num_midwest_teams-1){
						test_gen->midwest_tracker = 0;
						if (test_gen->east_tracker >= test_gen->num_east_teams-1){
							test_gen->east_tracker = 0;
							if(test_gen->south_tracker >= test_gen->num_south_teams-1){
								// SHOULDN'T GET HERE!!
								test_gen->num_scenarios_calculated = 0xffff;
							}
							else{
								test_gen->south_tracker++;
							}
						}
						else{
							test_gen->east_tracker++;
						}
					}
					else{
						test_gen->midwest_tracker++;
					}
				}
				else{
					test_gen->west_tracker++;
				}
				break;
			}
		}
		return 1;
	}
	else{
		return 0;
	}

}


// TODO: can I combine this with  generate_test_scenario somehow? passing in some sort of void object?
uint16_t generate_f4_scenario(f4_struct * f4){

	if (f4->num_scenarios_calculated < f4->num_scenarios){
		f4->given_f4[6] = f4->unique_south_teams[f4->south_tracker];
		f4->given_f4[5] = f4->unique_east_teams[f4->east_tracker];
		f4->given_f4[4] = f4->unique_midwest_teams[f4->midwest_tracker];
		f4->given_f4[3] = f4->unique_west_teams[f4->west_tracker];
		f4->num_scenarios_calculated++;

		switch(f4->scenario_tracker){
			// South winner, MW runner-up
			case(0):{
				f4->given_f4[0]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[2]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[1]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->scenario_tracker++;
				break;
			}
			// south winner, west runnerup
			case(1):{
				f4->given_f4[0]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[2]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[1]=f4->unique_west_teams[f4->west_tracker];
				f4->scenario_tracker++;
				break;
			}
			// east winner, midwest runnerup
			case(2):{
				f4->given_f4[0]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[2]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[1]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->scenario_tracker++;
				break;
			}
			// east winner, west runnerup
			case(3):{
				f4->given_f4[0]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[2]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[1]=f4->unique_west_teams[f4->west_tracker];
				f4->scenario_tracker++;
				break;
			}
			// midwest winner, south runnerup
			case(4):{
				f4->given_f4[0]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->given_f4[2]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[1]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->scenario_tracker++;
				break;
			}
			// midwest winner, east runnerup
			case(5):{
				f4->given_f4[0]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->given_f4[2]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[1]=f4->unique_midwest_teams[f4->midwest_tracker];
				f4->scenario_tracker++;
				break;
			}
			// west winner, south runnerup
			case(6):{
				f4->given_f4[0]=f4->unique_west_teams[f4->west_tracker];
				f4->given_f4[2]=f4->unique_south_teams[f4->south_tracker];
				f4->given_f4[1]=f4->unique_west_teams[f4->west_tracker];
				f4->scenario_tracker++;
				break;
			}
			// west winner, east runnerup
			case(7):{
				f4->given_f4[0]=f4->unique_west_teams[f4->west_tracker];
				f4->given_f4[2]=f4->unique_east_teams[f4->east_tracker];
				f4->given_f4[1]=f4->unique_west_teams[f4->west_tracker];
				// increment trackers
				f4->scenario_tracker = 0;
				if (f4->west_tracker >= f4->num_west_teams-1){
					f4->west_tracker = 0;
					if(f4->midwest_tracker >= f4->num_midwest_teams-1){
						f4->midwest_tracker = 0;
						if (f4->east_tracker >= f4->num_east_teams-1){
							f4->east_tracker = 0;
							if(f4->south_tracker >= f4->num_south_teams-1){
								// SHOULDN'T GET HERE!!
								f4->num_scenarios_calculated = 0xffff;
							}
							else{
								f4->south_tracker++;
							}
						}
						else{
							f4->east_tracker++;
						}
					}
					else{
						f4->midwest_tracker++;
					}
				}
				else{
					f4->west_tracker++;
				}
				break;
			}
		}
		return 1;
	}
	else{
		return 0;
	}

}


// find test_gen candidates
void find_test_teams(uint16_t f4_picks[NUM_BRACKETS][NUM_ELEMENTS_BRACKET], test_generator* test_gen){
	uint16_t i, j, k;
	uint16_t south_index= 0;
	uint16_t east_index = 0;
	uint16_t midwest_index = 0;
	uint16_t west_index = 0;
	for (i=0; i < NUM_BRACKETS; i++){
		for(j=0; j < NUM_ELEMENTS_BRACKET; j++){
			int found = 0;
			if (f4_picks[i][j] < MAX_SOUTH + 1){
				// find south teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (test_gen->test_south_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					test_gen->test_south_teams[south_index] = f4_picks[i][j];
					south_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_EAST + 1){
				// find east teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (test_gen->test_east_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					test_gen->test_east_teams[east_index] = f4_picks[i][j];
					east_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_MIDWEST + 1){
				// find midwest teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (test_gen->test_midwest_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					test_gen->test_midwest_teams[midwest_index] = f4_picks[i][j];
					midwest_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_WEST + 1){
				// find west teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (test_gen->test_west_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					test_gen->test_west_teams[west_index] = f4_picks[i][j];
					west_index++;
				}
			}
		}
	}

	// add in any unpicked teams that have a good chance of making F4
	for (i=0; i<MAX_SOUTH; i++){
		int found = 0;
		if (probs[i][F4_INDEX] > MINIMUM_TEAM_PROB) {
			for (j=0; j < NUM_TEAMS_REGION + 1; j++){
				if (test_gen->test_south_teams[j] == i+1){
					found = 1;
				}
			}

			if (found == 0){
				test_gen->test_south_teams[south_index] = i;
				south_index++;
			}
		}
	}
	for (i=MAX_SOUTH; i<MAX_EAST; i++){
		int found = 0;
		if (probs[i][F4_INDEX] > MINIMUM_TEAM_PROB) {
			for (j=0; j < NUM_TEAMS_REGION + 1; j++){
				if (test_gen->test_east_teams[j] == i+1){
					found = 1;
				}
			}

			if (found == 0){
				test_gen->test_east_teams[east_index] = i+1;
				east_index++;
			}
		}
	}
	for (i=MAX_EAST; i<MAX_MIDWEST; i++){
		int found = 0;
		if (probs[i][F4_INDEX] > MINIMUM_TEAM_PROB) {
			for (j=0; j < NUM_TEAMS_REGION + 1; j++){
				if (test_gen->test_midwest_teams[j] == i+1){
					found = 1;
				}
			}

			if (found == 0){
				test_gen->test_midwest_teams[midwest_index] = i+1;
				midwest_index++;
			}
		}
	}
	for (i=MAX_MIDWEST; i<MAX_WEST; i++){
		int found = 0;
		if (probs[i][F4_INDEX] > MINIMUM_TEAM_PROB) {
			for (j=0; j < NUM_TEAMS_REGION + 1; j++){
				if (test_gen->test_west_teams[j] == i+1){ // has to be +1
					found = 1;
				}
			}

			if (found == 0){
				test_gen->test_west_teams[west_index] = i+1;
				west_index++;
			}
		}
	}
	test_gen->num_south_teams = south_index;
	test_gen->num_east_teams = east_index;
	test_gen->num_midwest_teams = midwest_index;
	test_gen->num_west_teams = west_index;

	test_gen->num_scenarios = (test_gen->num_south_teams) * (test_gen->num_midwest_teams) * (test_gen->num_east_teams) * (test_gen->num_west_teams) * 8;
}




// find all unique f4 teams
void find_unique_elements(uint16_t f4_picks[NUM_BRACKETS][NUM_ELEMENTS_BRACKET], f4_struct* f4){
	uint16_t south_index= 0;
	uint16_t east_index = 0;
	uint16_t midwest_index = 0;
	uint16_t west_index = 0;
	uint16_t i, j, k;
	// + 1 because I'm gonna add the "unpicked" option
	for (i=0; i < NUM_TEAMS_REGION + 1; i++){
		f4->unique_south_teams[i] = 0;
		f4->unique_east_teams[i] = 0;
		f4->unique_midwest_teams[i] = 0;
		f4->unique_west_teams[i] = 0;
	}
	for (i=0; i < NUM_BRACKETS; i++){
		for(j=0; j < NUM_ELEMENTS_BRACKET; j++){
			int found = 0;
			if (f4_picks[i][j] < MAX_SOUTH + 1){
				// find south teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (f4->unique_south_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					f4->unique_south_teams[south_index] = f4_picks[i][j];
					south_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_EAST + 1){
				// find east teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (f4->unique_east_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					f4->unique_east_teams[east_index] = f4_picks[i][j];
					east_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_MIDWEST + 1){
				// find midwest teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (f4->unique_midwest_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					f4->unique_midwest_teams[midwest_index] = f4_picks[i][j];
					midwest_index++;
				}
			}
			else if (f4_picks[i][j] < MAX_WEST + 1){
				// find west teams
				for(k=0; k < NUM_TEAMS_REGION; k++){
					if (f4->unique_west_teams[k] == f4_picks[i][j]){
						found = 1;
						// TODO: can I just break here?
					}
				}
				if (found == 0){
					f4->unique_west_teams[west_index] = f4_picks[i][j];
					west_index++;
				}
			}
		}
	}

	f4->unique_south_teams[south_index] = UNPICKED_SOUTH;
	f4->unique_east_teams[east_index] = UNPICKED_EAST;
	f4->unique_midwest_teams[midwest_index] = UNPICKED_MIDWEST;
	f4->unique_west_teams[west_index] = UNPICKED_WEST;

	f4->num_south_teams = south_index + 1; // +1 because it started from 0 and I didn't increment for unpicked
	f4->num_east_teams = east_index + 1;
	f4->num_midwest_teams = midwest_index + 1;
	f4->num_west_teams = west_index + 1;

	f4->num_scenarios = (f4->num_south_teams) * (f4->num_midwest_teams) * (f4->num_east_teams) * (f4->num_west_teams) * 8;
}

