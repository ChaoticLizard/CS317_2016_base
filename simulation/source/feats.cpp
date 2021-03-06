/*
Simulation for zebrafish segmentation
Copyright (C) 2013 Ahmet Ay, Jack Holland, Adriana Sperlea, Sebastian Sangervasi

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
*/

/*
feats.cpp contains functions to analyze and test the oscillation features of simulations.
*/

#include <cfloat> // Needed for DBL_MAX

#include "feats.hpp" // Function declarations

#include "io.hpp"
#include "sim.hpp"

using namespace std;

extern terminal* term; // Declared in init.cpp

/*

In the future need to find a way to unify the functions in this file into some more consistent ones.
For the time being, I'm focusing on obtaining the needed data.

*/
int get_peaks_and_troughs1 (sim_data& sd, con_levels& cl, int actual_cell, int time_start, growin_array& crit_points, growin_array& type, growin_array& position, int mr) {
	/*
	Calculates all the peaks and troughs for the her1 mRNA oscillations in a cell within a time range. The cell number is given as a
	number relative to the entire PSM.
	*/
	int num_points = 0;
	int col = actual_cell % sd.width_total;
	
	for (int j = time_start + 1; j < sd.time_end - 1 && cl.cons[BIRTH][j][actual_cell] == cl.cons[BIRTH][j - 1][actual_cell] && cl.cons[BIRTH][j][actual_cell] == cl.cons[BIRTH][j + 1][actual_cell]; j++) {
		
		int pos = 0;
		if (cl.active_start_record[j] >= col) {
			pos = cl.active_start_record[j] - col;
		} else {
			pos = cl.active_start_record[j] + sd.width_total - col;
		}
	
		// check if the current point is a peak
		// check if the current point is a trough
		bool is_peak = true;
		bool is_trough = true;
		for (int k = MAX(j - (2 / sd.step_size / sd.big_gran), time_start); k <= MIN(j + 2 / sd.step_size / sd.big_gran, sd.time_end - 1); k++) {
			if (cl.cons[mr][j][actual_cell] <= cl.cons[mr][k][actual_cell] && j!=k) {
				is_peak = false;
			}

			if (cl.cons[mr][j][actual_cell] >= cl.cons[mr][k][actual_cell] && j!=k) {
				is_trough = false;
			}

		}
		if (is_peak) {
			crit_points[num_points] = j;
			type[num_points] = 1;
			position[num_points] = pos;
			num_points++;
		}
		
		if (is_trough) {
			crit_points[num_points] = j;
			type[num_points] = -1;
			position[num_points] = pos;
			num_points++;
		}
	}
	//cout<<max-min<<endl;
	return num_points;
}


int get_peaks_and_troughs2 (sim_data& sd, con_levels& cl, int actual_cell, int time_start, growin_array& crit_points, growin_array& type, growin_array& position, int mr, double* mh1_comp, double* mespa_comp, double* mespb_comp) {
	/*
	151221: record the concentration value of mh1, mespa, mespb in the time specified in order to calculate the complementary expression score of mespa and mespb. 
	*/
	int num_points = 0;
	int col = actual_cell % sd.width_total;
	
	int compl_count=0;
	for (int j = time_start + 1; j < sd.time_end - 1 && cl.cons[BIRTH][j][actual_cell] == cl.cons[BIRTH][j - 1][actual_cell] && cl.cons[BIRTH][j][actual_cell] == cl.cons[BIRTH][j + 1][actual_cell]; j++) {
		// calculate position in the PSM of the cell
		
		mh1_comp[compl_count]=cl.cons[CMH1][j][actual_cell];                 //record concentration value of mh1 151221
		mespa_comp[compl_count]=cl.cons[CMMESPA][j][actual_cell];           //record concentration value of mespa 151221
		mespb_comp[compl_count]=cl.cons[CMMESPB][j][actual_cell];             //record concentration value of mespb 151221
		compl_count++;
		
		int pos = 0;
		if (cl.active_start_record[j] >= col) {
			pos = cl.active_start_record[j] - col;
		} else {
			pos = cl.active_start_record[j] + sd.width_total - col;
		}
	
		// check if the current point is a peak
		// check if the current point is a trough
		bool is_trough = true;
		bool is_peak = true;
		for (int k = MAX(j - (2 / sd.step_size / sd.big_gran), time_start); k <= MIN(j + 2 / sd.step_size / sd.big_gran, sd.time_end - 1); k++) {
			if (cl.cons[mr][j][actual_cell] <= cl.cons[mr][k][actual_cell] && j!=k) {
				is_peak = false;
				
			}

			if (cl.cons[mr][j][actual_cell] >= cl.cons[mr][k][actual_cell] && j!=k) {
				is_trough = false;
			}	

		}
		if (is_peak) {
			crit_points[num_points] = j;
			type[num_points] = 1;
			position[num_points] = pos;
			num_points++;
		}
		

		if (is_trough) {
			crit_points[num_points] = j;
			type[num_points] = -1;
			position[num_points] = pos;
			num_points++;
		}
	}
	//cout<<max-min<<endl;
	return num_points;
}

double test_complementary (sim_data& sd, con_levels& cl, int time, int con1, int con2) {   // calculate the complementary expression score of mespa and mespb. not used
	double avg_row_con1[sd.width_total];
    double avg_row_con2[sd.width_total];
	memset(avg_row_con1, 0, sizeof(double) * sd.width_total);
    memset(avg_row_con2, 0, sizeof(double) * sd.width_total);

	for (int y = 0; y < sd.width_total; y++) {
		for (int x = 0; x < sd.height; x++) {
			int cell = x * sd.width_total + y;
			avg_row_con1[y] += cl.cons[con1][time][cell];
            avg_row_con2[y] += cl.cons[con2][time][cell];
		}
		
		avg_row_con1[y] /= sd.height;
        avg_row_con2[y] /= sd.height;
	}

	return pearson_correlation(avg_row_con1, avg_row_con2, 0.6*sd.width_total, sd.width_total);  //JY WT.10 rounding?
}

double test_compl(sim_data& sd, double* con1, double* con2) {  // 151221: calculate the complementary expression score of mespa and mespb
	//int count=sd.width_total*sd.steps_split - 2;
	
	return pearson_correlation(con1, con2, (int)(0.6*(sd.width_total*sd.steps_split - 2)),sd.width_total*sd.steps_split - 2);   
}



void osc_features_ant (sim_data& sd, input_params& ip, features& wtfeat, char* filename_feats, con_levels& cl, mutant_data& md, int start_line, int end_line, int start_col, int end_col, int set_num) {
	static int con[5] = {CMH1, CMH7, CMDELTA, CMMESPA, CMMESPB};
	static int ind[5] = {IMH1, IMH7, IMDELTA, IMMESPA, IMMESPB};
	static const char* concs[5] = {"mh1", "mh7," "mdelta", "mespa", "mespb"};
	static const char* feat_names[NUM_FEATURES] = {"period", "amplitude", "sync"};
	static double curve[101] = {1, 1.003367003, 1.003367003, 1.003367003, 1.004713805, 1.004713805, 1.007407407, 1.015488215, 1.015488215, 1.020875421, 1.023569024, 1.023569024, 1.026262626, 1.028956229, 1.037037037, 1.037037037, 1.03973064, 1.042424242, 1.047811448, 1.050505051, 1.055892256, 1.058585859, 1.061279461, 1.066666667, 1.069360269, 1.072053872, 1.077441077, 1.082828283, 1.088215488, 1.090909091, 1.096296296, 1.098989899, 1.104377104, 1.10976431, 1.115151515, 1.115151515, 1.120538721, 1.125925926, 1.128619529, 1.139393939, 1.142087542, 1.15016835, 1.155555556, 1.160942761, 1.169023569, 1.174410774, 1.182491582, 1.187878788, 1.195959596, 1.201346801, 1.212121212, 1.22020202, 1.228282828, 1.239057239, 1.247138047, 1.255218855, 1.268686869, 1.276767677, 1.287542088, 1.301010101, 1.314478114, 1.325252525, 1.336026936, 1.352188552, 1.368350168, 1.381818182, 1.397979798, 1.414141414, 1.432996633, 1.454545455, 1.476094276, 1.492255892, 1.519191919, 1.546127946, 1.573063973, 1.6, 1.632323232, 1.672727273, 1.705050505, 1.742760943, 1.785858586, 1.837037037, 1.896296296, 1.955555556, 2.025589226, 2.106397306, 2.195286195, 2.303030303, 2.418855219, 2.572390572, 2.725925926, 2.941414141, 3.208080808, 3.574410774, 4, 8.399297321, 12.79859464, 17.19789196, 21.59718928, 25.99648661, 30.39578393};
	
	growin_array crit_points(sd.steps_total / (20/sd.step_size)); // Array that will hold all the critical points in the graph
	growin_array type(sd.steps_total / (20/sd.step_size)); // Array that will specify whether each critical point is a peak or a trough (-1 for trough, 1 for peak)
	growin_array position(sd.steps_total / (20/sd.step_size)); // Array that will hold the position in the PSM of the critical points
	
	int strlen_set_num = INT_STRLEN(set_num); // How many bytes the ASCII representation of set_num takes
	char* str_set_num = (char*)mallocate(sizeof(char) * (strlen_set_num + 1));
	sprintf(str_set_num, "%d", set_num);
	
	int num_cell = (end_line - start_line) * (end_col - start_col);
	double mh1_comp[sd.width_total*sd.steps_split - 2];   //151221: structure to store concentration value for mh1, used in get_peaks_and_troughs2 
	double mespa_comp[sd.width_total*sd.steps_split - 2]; //151221: structure to store concentration value for mespa, used in get_peaks_and_troughs2
	double mespb_comp[sd.width_total*sd.steps_split - 2]; //151221: structure to store concentration value for mespb, used in get_peaks_and_troughs2
	double comp_score_a = 0; //151221: complementary score for mespa
	double comp_score_b = 0; //151221: complementary score for mespa
	memset(mh1_comp, 0, sizeof(double) * (sd.width_total*sd.steps_split - 2));
	memset(mespa_comp, 0, sizeof(double) * (sd.width_total*sd.steps_split - 2));
	memset(mespb_comp, 0, sizeof(double) * (sd.width_total*sd.steps_split - 2));
	
	
	for (int i = 0; i < 5; i++) {
		ofstream features_files[NUM_FEATURES]; // Array that will hold the files in which to output the period and amplitude
	
		int mr = con[i];
		int index = ind[i];
		if (ip.ant_features) {
			for (int j = 0; j < NUM_FEATURES; j++) {
				char* filename = (char*)mallocate(sizeof(char) * strlen(filename_feats) + strlen("set_") + strlen_set_num + 1 + strlen(feat_names[j]) + 1 + strlen(concs[i]) + strlen("_ant.feats") + 1);
				sprintf(filename, "%sset_%s_%s_%s_ant.feats", filename_feats, str_set_num, feat_names[j], concs[i]);
				cout << "      ";
				open_file(&(features_files[j]), filename, false);
				mfree(filename);
			}
			
			features_files[PERIOD] << sd.height << "," << sd.width_total << endl;
			features_files[AMPLITUDE] << sd.height << "," << sd.width_total << endl;
		}
		double amp_avg = 0;
		double period_avg = 0;
        int time_start;
        if (md.induction == 0) {
		    time_start = anterior_time(sd, sd.steps_til_growth + (sd.width_total - sd.width_initial) * sd.steps_split); // time after which the PSM is full of cells
        } else {
            //time_start = anterior_time(sd, md.induction + (30 / sd.big_gran));
			time_start = anterior_time(sd, sd.steps_til_growth + (sd.width_total - sd.width_initial) * sd.steps_split);//anterior_time(sd, sd.steps_til_growth + (30 / sd.step_size));
        }
		int num_cells_passed = 0;
		
		for (int col = start_col; col < end_col; col++) {						
			for (int line = start_line; line < end_line; line++) {
				int pos = cl.active_start_record[time_start]; // always looking at cell at position active_start because that is the newest cell
				int cell = line * sd.width_total + pos;
				int num_points = 0;
                if ( mr != CMMESPA) {
				    num_points = get_peaks_and_troughs1(sd, cl, cell, time_start, crit_points, type, position, mr);
                } else {
					
					num_points = get_peaks_and_troughs2(sd, cl, cell, time_start, crit_points, type, position, mr, mh1_comp, mespa_comp, mespb_comp);  // 151221: get_peaks_and_troughs2 records the concentration value of mh1, mespa and mespb and store them in mh1_comp, mespa_comp, mespb_comp
					comp_score_a+=test_compl(sd, mh1_comp, mespa_comp);
					comp_score_b+=test_compl(sd, mh1_comp, mespb_comp);
				} 
			
				double periods[num_points];
				double per_pos[num_points];
                double per_time[num_points];
				double amplitudes[num_points];
				double amp_pos[num_points];
                //double amp_time[num_points]; // amp_time is not used in the new calculation methods
				memset(periods, 0, sizeof(double) * num_points);
				memset(amplitudes, 0, sizeof(double) * num_points);
				memset(per_pos, 0, sizeof(double) * num_points);
				memset(amp_pos, 0, sizeof(double) * num_points);

				int pers = 0;
				int amps = 0;
				if (num_points >= 3) { 
					// Calculate all the periods and amplitudes
					
 					// Find the first peak and the first trough occurring in the graph
					int cur_point = 0;
					
					for (; cur_point < num_points; cur_point++) {		
						// Check for period
						if (type[cur_point] == 1 && cur_point >= 2) {
							periods[pers] = (crit_points[cur_point] - crit_points[cur_point - 2]) * sd.step_size * sd.big_gran;
							per_pos[pers] = position[cur_point - 2] + (position[cur_point] - position[cur_point - 2]) / 2;
                            per_time[pers] = (crit_points[cur_point] - crit_points[cur_point - 2]) / 2 * sd.step_size * sd.big_gran;
							pers++;
						}
						
						// Check for amplitude
						if (type[cur_point] == 1 && cur_point >= 1 && cur_point < num_points - 1) {
							amplitudes[amps] = cl.cons[mr][crit_points[cur_point]][cell] - (cl.cons[mr][crit_points[cur_point - 1]][cell] + cl.cons[mr][crit_points[cur_point + 1]][cell]) / 2;
							amp_pos[amps] = position[cur_point];
                            //amp_time[amps] = crit_points[cur_point];
							amps++;
						}
					}
					
					bool passed = true;
					// Try to fit out period data to the curve provided by Giudicelli et al.
					if (pers < 3) { // If we don't have at least 3 data points then the oscillations were not good enough
						passed = false;
					} else {
						int first_fit = per_pos[0] * 100 / (sd.width_total - 1); // Find the place on the curve of the first period for comparison purposes
						//if (mr == CMH1){cout<<1<<" "<<1<<endl;}
						for (int i = 1; i < pers; i++) {
							if (per_pos[i] > 0.85 * (sd.width_total - 1)) {
 								break;
 							}

							int percentage = per_pos[i] * 100 / (sd.width_total - 1); // Find the place on the curve of the current period
							double ratio = periods[i] / periods[0]; // The ratio between the current period and the first period
							//if (mr==CMH1){cout<<ratio<<" "<<curve[percentage] / curve[first_fit]<<endl;}
							if (!( (0.9 * curve[percentage] / curve[first_fit]) < ratio && ratio < (1.1 * curve[percentage] / curve[first_fit]))) {   //JY WT.2. checking every period for every cell
								passed = false;
								break;
							}
						}
					}
					if (passed && mr == CMH1) {
						num_cells_passed++;
					}
					
					// the amplitude is the average of the amplitudes of all oscillations

					double amp_cell = 0;
					for (int i = 0; i < amps; i++) {
						amp_cell += amplitudes[i];
					}
					//double period_cell = 0;
					//int rang = (int)(0.85 * pers);
					//int count = 0;
					/*for (; rang < pers; rang++) {
						period_cell += periods[rang];
						count++;
					}*/
					
					if (amps == 0){
						amp_avg+=0;
					} else {
						amp_avg += amp_cell / (amps);
					}
					period_avg += (periods[0]);
				} else {
					amp_avg += 1;
					period_avg += 1;
				}
                // Printing to files for the anterior features
				if (ip.ant_features) {
					for (int j = 0; j < pers; j++) {
						features_files[PERIOD] << per_pos[j] << ",";
					}
					features_files[PERIOD] << endl;
					for (int j = 0; j < pers; j++) {
						features_files[PERIOD] << periods[j] << ",";
					}
					features_files[PERIOD] << endl;
					for (int j = 0; j < amps; j++) {
						features_files[AMPLITUDE] << amp_pos[j] << ",";
					}
					features_files[AMPLITUDE] << endl;
					for (int j = 0; j < amps; j++) {
						features_files[AMPLITUDE] << amplitudes[j] << ",";
					}
					features_files[AMPLITUDE] << endl;
				}

                // Updating mutant data
                for (int j = 0; j < pers; j++) {                                                    // 151221: may be unused
					if (per_time[j] >= anterior_time(sd, md.induction)) {
                    	double half_hour_index = 0.5 * (((int)(per_time[j] - anterior_time(sd, md.induction)) * sd.big_gran / 3000) + 1);
                   		if (per_pos[j] < sd.width_initial) {
                   		    md.feat.period_post_time[index][half_hour_index] = periods[j];   //JY WT.2.
                    	} else {
                    	    md.feat.period_ant_time[index][half_hour_index] = periods[j];    
                    	}
					}
                }
               /* for (int j = 0; j < amps; j++) {
					if (amp_time[j] >= anterior_time(sd, md.induction)) {
                    	double half_hour_index = 0.5 * ((int)((amp_time[j] - anterior_time(sd, md.induction)) * sd.big_gran / 3000) + 1);
                    	if (per_pos[j] < sd.width_initial) {
                    	    md.feat.amplitude_post_time[index][half_hour_index] = amplitudes[j];
                    	} else {
                    	    md.feat.amplitude_ant_time[index][half_hour_index] = amplitudes[j];
                    	}
					}
                }*/

				type.reset(sd.steps_total / (20/sd.step_size));
				crit_points.reset(sd.steps_total / (20/sd.step_size));
			}		
			time_start += sd.steps_split / sd.big_gran; // skip in time until a new column of cells has been formed
		}
		if (ip.ant_features) {
			features_files[PERIOD].close();
			features_files[AMPLITUDE].close();
		}

		amp_avg /= (end_line - start_line) * (end_col - start_col);
		period_avg /= (end_line - start_line) * (end_col - start_col);
		md.feat.period_post[index]=period_avg;
		
		
        md.feat.period_ant[index] = period_avg;
		md.feat.amplitude_ant[index] = amp_avg;  //JY WT.3.  take average of all amplitude for all cell
		if (md.index == MUTANT_WILDTYPE && mr == CMH1) {
			int threshold = 0.7 * (end_line - start_line) * (end_col - start_col); // 151221: originally 80%, now changed to 70%
			md.conds_passed[SEC_ANT][0] = (num_cells_passed >= threshold);
		}
		
		if (md.index == MUTANT_WILDTYPE) {                              //151221: calculate oscillation features for wildtype, including posterior amplitude, anterior amplitude and syncrony score for different species
			
			int time_half = anterior_time(sd,(600+30)/sd.step_size);         //half hours after induction, 10 snapshot in 30 minutes
			int time_half_end = anterior_time(sd,(600+60)/sd.step_size);
			for (;time_half<time_half_end; time_half+=(3/sd.step_size)){
				md.feat.amplitude_post_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0, sd.width_initial);
				md.feat.amplitude_ant_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0.6*sd.width_total, sd.width_total);
				md.feat.amplitude_post[index] +=  avg_amp(sd,cl,index+1,time_half, 0, sd.width_initial);
				//md.feat.sync_score_post[index]+=post_sync(sd,cl, index + 1, time_half);
				md.feat.sync_score_ant[index]+= ant_sync(sd, cl, index + 1, time_half);
			}
			
			//md.feat.sync_score_post[index]/=10;
			md.feat.sync_score_ant[index]/=10;
			if (index == 0) {
				int time_three = anterior_time(sd, (600+180)/sd.step_size);          //three hours after induction, 10 snapshot in 30 minutes
				int time_three_end = anterior_time(sd, (600+210)/sd.step_size);
				for (;time_three<time_three_end; time_three+=(3/sd.step_size)){
					md.feat.amplitude_post_time[index][3]+= avg_amp(sd,cl,index+1,time_three, 0, sd.width_total);
					//md.feat.amplitude_ant_time[index][3]+=avg_amp(sd,cl,index+1,time_three, 0.6*sd.width_total, sd.width_total);
				}

			}

			if (index == 2 || index ==3){
				int time_one = anterior_time(sd, (600+60)/sd.step_size);             //one hour after induction, 10 snapshot in 30 minutes
				int time_one_end = anterior_time(sd, (600+90)/sd.step_size);
				for (;time_one<time_one_end; time_one+=(3/sd.step_size)){
					
					md.feat.amplitude_ant_time[index][1]+=avg_amp(sd,cl,index+1,time_one, 0.6*sd.width_total, sd.width_total);
				}

				int time_two = anterior_time(sd, (600+120)/sd.step_size);            //two hours after induction, 10 snapshot in 30 minutes
				int time_two_end = anterior_time(sd, (600+150)/sd.step_size);            
				for (;time_two<time_two_end; time_two+=(3/sd.step_size)){
					
					md.feat.amplitude_ant_time[index][2]+= avg_amp(sd,cl,index+1,time_two, 0.6*sd.width_total, sd.width_total);
				}
			}

		}

		if (md.index==MUTANT_DELTA){  //151221: calculate oscillation features for delta mutant, including posterior amplitude, anterior amplitude and syncrony score for different species
						


			if (index==2){
				
				int time_half = anterior_time(sd,(600+30)/sd.step_size);         //half hours after induction, 10 snapshot in 30 minutes
				int time_half_end = anterior_time(sd,(600+60)/sd.step_size);
				for (;time_half<time_half_end; time_half+=(3/sd.step_size)){
					md.feat.sync_score_ant[0]+=ant_sync(sd, cl, 0 + 1, time_half);
					md.feat.sync_score_ant[3]+=ant_sync(sd, cl, 3 + 1, time_half);
					//md.feat.sync_score_post[0]+=post_sync(sd,cl, 0 + 1, time_half);
					md.feat.amplitude_post[0] +=  avg_amp(sd,cl,0+1,time_half, 0, sd.width_initial);
					md.feat.amplitude_ant_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0.6*sd.width_total, sd.width_total);
				}
				
				
				md.feat.sync_score_ant[0]/=10;
				md.feat.sync_score_ant[3]/=10;
				//md.feat.sync_score_post[0]/=10;
			}

			
		}

		if (md.index==MUTANT_HER7OVER){    //calculate oscillation features for her7-overexpression mutant, including posterior amplitude, anterior amplitude and syncrony score for different species

			if (index == 0 || index ==5){
				int time_half = anterior_time(sd,(600+30)/sd.step_size);                    //half hours after induction, 10 snapshot in 30 minutes
				int time_half_end = anterior_time(sd,(600+60)/sd.step_size);
				for (;time_half<time_half_end; time_half+=(3/sd.step_size)){
					md.feat.amplitude_post_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0, sd.width_initial);
					
				}
			}

			if (index == 0 || index ==2){
				int time_half = anterior_time(sd,(600+30)/sd.step_size);                       //half hours after induction, 10 snapshot in 30 minutes
				int time_half_end = anterior_time(sd,(600+60)/sd.step_size);
				for (;time_half<time_half_end; time_half+=(3/sd.step_size)){
					//cout<<md.feat.amplitude_ant_time[0][0.5]<<endl;
					md.feat.amplitude_ant_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0.6*sd.width_total, sd.width_total);
				}
			}

			if (index == 3) {
				int time_onehalf = anterior_time(sd,(600+90)/sd.step_size);                 //one and a half hours after induction, 10 snapshot in 30 minutes
				int time_onehalf_end = anterior_time(sd,(600+120)/sd.step_size);
				for (;time_onehalf<time_onehalf_end; time_onehalf+=(3/sd.step_size)){
					md.feat.sync_time[index][1.5]+=ant_sync(sd, cl, index + 1, time_onehalf);
					
				}
				md.feat.sync_time[index][1.5]/=10;
			}
		}

		if (md.index==MUTANT_HER1OVER){       //calculate oscillation features for her1-overexpression mutant, including posterior amplitude, anterior amplitude and syncrony score for different species
			if (index == 1 || index ==5){
				int time_half = anterior_time(sd,(600+30)/sd.step_size);           //half hours after induction, 10 snapshot in 30 minutes
				int time_half_end = anterior_time(sd,(600+60)/sd.step_size);
				for (;time_half<time_half_end; time_half+=(3/sd.step_size)){
					md.feat.amplitude_post_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0, sd.width_initial);
					md.feat.amplitude_ant_time[index][0.5]+=avg_amp(sd,cl,index+1,time_half, 0.6*sd.width_total, sd.width_total);
				}
			}
		}

		if (md.index==MUTANT_DAPT){                       //calculate oscillation features for dapt mutant, including posterior amplitude, anterior amplitude and syncrony score for different species
			if (index == 0){
				int time_three = anterior_time(sd, (600+180)/sd.step_size);          //three hours after induction, 10 snapshot in 30 minutes
				int time_three_end = anterior_time(sd, (600+210)/sd.step_size);
				for (;time_three<time_three_end; time_three+=(3/sd.step_size)){
					md.feat.amplitude_post_time[index][3]+=avg_amp(sd,cl,index+1,time_three, 0, sd.width_total);
					md.feat.sync_time[index][3]+=ant_sync(sd, cl, index + 1, time_three);
					//md.feat.amplitude_ant_time[index][3]+=avg_amp(sd,cl,index+1,time_three, 0.6*sd.width_total, sd.width_total);
				}

				
				md.feat.sync_time[index][3]/=10;
			}

			if (index==2) {
				int time_two = anterior_time(sd, (600+120)/sd.step_size);         //two hours after induction, 10 snapshot in 30 minutes
				int time_two_end = anterior_time(sd, (600+150)/sd.step_size);
				for (;time_two<time_two_end; time_two+=(3/sd.step_size)){
					
					md.feat.amplitude_ant_time[index][2]+=avg_amp(sd,cl,index+1,time_two, 0.6*sd.width_total, sd.width_total);
				}
			}

			if (index==3) {
				int time_three = anterior_time(sd, (600+180)/sd.step_size);        //three hours after induction, 10 snapshot in 30 minutes
				int time_three_end = anterior_time(sd, (600+210)/sd.step_size);
				for (;time_three<time_three_end; time_three+=(3/sd.step_size)){
					md.feat.sync_time[index][3]+=ant_sync(sd, cl, index + 1, time_three);
				}
				md.feat.sync_time[index][3]/=10;
			}
		}

		if (md.index==MUTANT_MESPAOVER){                     //calculate oscillation features for mespa mutant, including posterior amplitude, anterior amplitude and syncrony score for different species
			if (index==3){
				int time_one = anterior_time(sd, (600+60)/sd.step_size);         //one hour after induction, 10 snapshot in 30 minutes
				int time_one_end = anterior_time(sd, (600+90)/sd.step_size);
				for (;time_one<time_one_end; time_one+=(3/sd.step_size)){
					
					md.feat.amplitude_ant_time[index][1]+=avg_amp(sd,cl,index+1,time_one, 0.6*sd.width_total, sd.width_total);
				}
			}
		}
		
		if (md.index==MUTANT_MESPBOVER){                    //calculate oscillation features for mespb mutant, including posterior amplitude, anterior amplitude and syncrony score for different species
			if (index==2 || index == 3){
				int time_one = anterior_time(sd, (600+60)/sd.step_size);            //one hours after induction, 10 snapshot in 30 minutes
				int time_one_end = anterior_time(sd, (600+90)/sd.step_size);
				for (;time_one<time_one_end; time_one+=(3/sd.step_size)){
					
					md.feat.amplitude_ant_time[index][1]+=avg_amp(sd,cl,index+1,time_one, 0.6*sd.width_total, sd.width_total);
				}
			}
		}

		// for sync take 5 snapshots and average sync scores and count waves of mespa and mespb	for wildtype
		/*double sync_avg = 0;
	    int time_full = anterior_time(sd, sd.steps_til_growth + (sd.width_total - sd.width_initial - 1) * sd.steps_split);
	    for (int time = time_full; time < sd.time_end; time += (sd.time_end - 1 - time_full) / 4) {
		    sync_avg += ant_sync(sd, cl, index + 1, time);
			if (md.index == MUTANT_WILDTYPE) {  // wildtype
				wave_testing_mesp(sd, cl, md, time, sd.active_start);
			}
	    }

	    md.feat.sync_score_ant[index] = sync_avg / 5;
        if (md.induction != 0) {
            int time_after_induction = anterior_time(sd, md.induction + 3000 / sd.big_gran);
            for (int i = 0; i < 5; i++) {
                int mr = con[i];
                double half_hour_index = 0.5;
                for (int time = time_after_induction; time < sd.time_end; time += (3000 / sd.big_gran)) {
                    md.feat.sync_time[ind[i]][half_hour_index] = ant_sync(sd, cl, mr, time);
                    half_hour_index += 0.5;
                }
            }
        }*/
		if (ip.ant_features) {
			int time_start = anterior_time(sd, sd.steps_til_growth + (sd.width_total - sd.width_initial - 1) * sd.steps_split);
			for (int col = start_col; col < end_col; col++) {
				if (ip.ant_features) {
					plot_ant_sync(sd, cl, time_start, &features_files[SYNC], col == start_col);
				}
				time_start += sd.steps_split;
			}
			features_files[SYNC].close();
		}
		if (index == IMMESPA) { //151221: complementary score for mespa and mespb
			// for complementary mesp expression take 6 snapshots and average comp score
			//double num = 0;
			
			
		    //int time_full = anterior_time(sd, sd.steps_til_growth + (sd.width_total - sd.width_initial - 1) * sd.steps_split + 9000);
			/*for ( int time= time_full; time < time_full + 3000; time += 600) { // have 6 minutes in between snapshots
                //md.feat.comp_score_ant_mespa += test_complementary(sd, cl, time, CMH1, CMMESPA);
				
                md.feat.comp_score_ant_mespb += test_complementary(sd, cl, time, CMH1, CMMESPB);
				num += 1;
			}*/
			//cout<<md.feat.comp_score_ant_mespa<<endl;
			
			md.feat.comp_score_ant_mespa = comp_score_a / num_cell;
            md.feat.comp_score_ant_mespb = comp_score_b / num_cell;
			
		} 
		//md.feat.sync_score_ant[index] = sync_avg / 5; // JY bug?
	}
	mfree(str_set_num);
}

void osc_features_post (sim_data& sd, input_params& ip, con_levels& cl, features& feat, features& wtfeat, char* filename_feats, int start, int end, int set_num) {   //151221:  we are only using the this for the peaktotrough condition in wildtype mutant. maybe you can delete some unnecessary lines
	/*
	 Calculates the oscillation features: period, amplitude, and peak to trough ratio for a set of concentration levels.
	 The values are calculated using the last peak and trough of the oscillations, since the amplitude of the first few oscillations can be slightly unstable.
	 For the wild type, the peak and trough at the middle of the graph are also calculated in order to ensure that the oscillations are sustained.
	*/

	int strlen_set_num = INT_STRLEN(set_num); // How many bytes the ASCII representation of set_num takes
	//char* str_set_num = (char*)mallocate(sizeof(char) * (strlen_set_num + 1));
    char* str_set_num = (char*) mallocate(2);
	sprintf(str_set_num, "%d", set_num);

	int con[3] = {CMH1, CMH7, CMDELTA};
	int ind[3] = {IMH1, IMH7, IMDELTA};
	static const char* concs[3] = {"mh1", "mh7", "deltac"};
	static const char* feat_names[NUM_FEATURES] = {"period", "amplitude", "sync"};
	ofstream features_files[NUM_FEATURES]; // Array that will hold the files in which to output the period and amplitude

	int num_genes = 3;
	for (int i = 0; i < num_genes; i++) {
		if (ip.post_features) {
			for (int j = 0; j < NUM_FEATURES; j++) {
				char* filename = (char*)mallocate(sizeof(char) * strlen(filename_feats) + strlen("set_") + strlen_set_num + 1 + strlen(feat_names[j]) + 1 + strlen(concs[i]) + strlen("_post.feats") + 1);
				sprintf(filename, "%sset_%s_%s_%s_post.feats", filename_feats, str_set_num, feat_names[j], concs[i]);
				cout << "      ";
				open_file(&(features_files[j]), filename, false);
				mfree(filename);
			}
			
			features_files[PERIOD] << sd.height << "," << sd.width_initial << endl;
			features_files[AMPLITUDE] << sd.height << "," << sd.width_initial << endl;
		}
	
		int mr = con[i];
		int index = ind[i];

		double period_tot = 0;
		double amplitude = 0;
		double peaktotrough_end = 0;
		double peaktotrough_mid = 0; 
		double num_good_somites = 0;

		for (int x = 0; x < sd.height; x++) {
			for (int y = 0; y < sd.width_current; y++) {
				int cell = x * sd.width_total + y;
				growin_array peaks(sd.steps_total / (20/sd.step_size)); 
				growin_array troughs(sd.steps_total / (20/sd.step_size));
				int num_peaks = 0;
				int num_troughs = 0;
				int peaks_period = 0;

				double cell_period = 0;
				bool calc_period = true;

				for (int j = start + 1; j < end - 1; j++) {
					if (abs(num_peaks - num_troughs) > 1) {
						num_peaks = 0;
						break;
					}
					
					//check if the current point is a peak
					if (cl.cons[mr][j - 1][cell] < cl.cons[mr][j][cell] && cl.cons[mr][j][cell] > cl.cons[mr][j + 1][cell]) {
						peaks[num_peaks] = j;
						num_peaks++;
						if (calc_period) {
							peaks_period++;
						}
						
						// add the current period to the average calculation
						if (num_peaks >= 2 && calc_period) {
							double period = (peaks[num_peaks - 1] - peaks[num_peaks - 2]) * sd.step_size * sd.big_gran;
							cell_period += period;
							if (num_peaks >= 4) {
								features_files[PERIOD] << period << " ";
							}
						}
					}
					
					//check if the current point is a trough
					if (cl.cons[mr][j - 1][cell] > cl.cons[mr][j][cell] && cl.cons[mr][j][cell] < cl.cons[mr][j + 1][cell]) {
						troughs[num_troughs] = j;
						num_troughs++;
						
						//check if the amplitude has dropped under 0.3 of the wildtype amplitude
						if (num_troughs >= 2) {
							int last_peak = peaks[num_peaks - 1];
							int last_trough = troughs[num_troughs - 1];
							int sec_last_trough = troughs[num_troughs - 2];
							
							double first_amp = peaks[1] - (troughs[0] + troughs[1]) / 2;
							double cur_amp = cl.cons[mr][last_peak][cell] - (cl.cons[mr][last_trough][cell] + cl.cons[mr][sec_last_trough][cell]) / 2;
							if (num_peaks >= 4) {
								features_files[AMPLITUDE] << cur_amp << " ";
							}
							if (cur_amp < (wtfeat.amplitude_post[mr] > 0 ? 0.3 * wtfeat.amplitude_post[mr] : 0.3 * first_amp)) {
								calc_period = false;
							}
						}
					}
				}
				cell_period /= peaks_period;

				if (num_peaks >= 3) {
					int peak_penult = peaks[num_peaks - 2];
					int trough_ult = troughs[num_peaks - 2];	
					int trough_penult = troughs[num_peaks - 3];	
					int peak_mid = peaks[num_peaks / 2];
					int trough_mid = troughs[num_peaks / 2];

					period_tot += cell_period;
					amplitude += (cl.cons[mr][peak_penult][cell] - (cl.cons[mr][trough_penult][cell] + cl.cons[mr][trough_ult][cell]) / 2);
					peaktotrough_end += cl.cons[mr][trough_ult][cell] > 1 ? cl.cons[mr][peak_penult][cell] / cl.cons[mr][trough_ult][cell] : cl.cons[mr][peak_penult][cell];
					peaktotrough_mid += cl.cons[mr][trough_mid][cell] > 1 ? cl.cons[mr][peak_mid][cell] / cl.cons[mr][trough_mid][cell] : cl.cons[mr][peak_mid][cell];

				} else {
					period_tot += (period_tot < INFINITY ? INFINITY : 0);
					amplitude ++;
					peaktotrough_end ++;
					peaktotrough_mid ++;
				}
				num_good_somites += num_troughs - 1;
				features_files[PERIOD] << endl;
				features_files[AMPLITUDE] << endl;
			}
		}
		
		features_files[PERIOD].close();
		features_files[AMPLITUDE].close();
		int cells = sd.height * sd.width_current;
		period_tot /= cells;
		amplitude /= cells;
		peaktotrough_end /= cells;
		peaktotrough_mid /= cells;
		num_good_somites /= cells;

		//feat.period_post[index] = period_tot;
		feat.amplitude_post[index] = amplitude;
		feat.peaktotrough_end[index] = peaktotrough_end;
		feat.peaktotrough_mid[index] = peaktotrough_mid;
		feat.num_good_somites[index] = num_good_somites;
		
		//feat.sync_score_post[index] = post_sync(sd, cl, mr, (start + end) / 2, end);
	}
	mfree(str_set_num);
}

double avg_amp (sim_data& sd, con_levels& cl, int con, int time, int start , int end){              //151221: calculate the average concentration value, and use it as amplitude
	int pos_start = cl.active_start_record[time];
	int pos_cur = 0;
	double conslevel = 0;

	for (int x = 0; x < sd.height; x++) {
		for (int y=start; y <end; y++){
			if (pos_start - y < 0){
				pos_cur = pos_start -2*y + sd.width_total;
			} else {
				pos_cur = pos_start -2*y;
			}
			int cell = x * sd.width_total + y + pos_cur;
			conslevel += cl.cons[con][time][cell];
		}
	}
	return conslevel / (sd.height*(end-start));
}

double ant_sync (sim_data& sd, con_levels& cl, int con, int time) {  //151221: calculate syncronization score
	if (sd.height == 1) {
		return 1; // for 1d arrays there is no synchronization between rows 
	}

	double first_row[sd.width_total];
	double cur_row[sd.width_total];
	int pos_start = cl.active_start_record[time];
	int pos_first = 0;
	int pos_cur = 0;

	for (int y = 0; y < sd.width_total; y++) {
		if (con == 3 || con ==4){
			if (pos_start - y < 0){
				pos_first = pos_start -2*y + sd.width_total;
			} else {
				pos_first = pos_start -2*y;
			}
		}
		first_row[y] = cl.cons[con][time][y + pos_first];
	}

	double pearson_sum = 0;
	for (int x = 1; x < sd.height; x++) {
		for (int y = 0; y < sd.width_total; y++) {
			if (con == 3 || con ==4){
				if (pos_start - y < 0){
					pos_cur = pos_start -2*y + sd.width_total;
				} else {
					pos_cur = pos_start -2*y;
				}
			}
			int cell = x * sd.width_total + y + pos_cur;
			cur_row[y] = cl.cons[con][time][cell];
		}
		if (con == 3 || con ==4){
			
			pearson_sum += pearson_correlation(first_row, cur_row, (int)(0.6*sd.width_total), sd.width_total );   //mespa and mespb only express in anterior
		} else {
			
			pearson_sum += pearson_correlation(first_row, cur_row, 0, sd.width_total);
		}
	}

	return pearson_sum / (sd.height - 1); 
}

void plot_ant_sync (sim_data& sd, con_levels& cl, int time_start, ofstream* file_pointer, bool first_col) {
	int col = cl.active_start_record[time_start];
	
	double first_row[sd.width_total * sd.steps_split];
	double other_row[sd.width_total * sd.steps_split];
	memset(first_row, 0, sizeof(double) * sd.width_total * sd.steps_split);
	memset(other_row, 0, sizeof(double) * sd.width_total * sd.steps_split);
	
	first_row[0] = cl.cons[CMH1][time_start][col];	
	int time = time_start + 1;
	for (; cl.cons[BIRTH][time][col] == cl.cons[BIRTH][time - 1][col]; time++) {
		first_row[time - time_start] = cl.cons[CMH1][time][col];
	}
	int time_end = time;
	int interval = INTERVAL / sd.step_size;
	int num_points = (time_end - time_start - interval) / (interval / 2); 
	
	if (first_col) {
		*file_pointer << sd.height - 1 << "," << INTERVAL << "," << sd.steps_split * sd.small_gran << endl;
	}

	double sync_avg[num_points];
	memset(sync_avg, 0, sizeof(double) * num_points);
	for (int x = 1; x < sd.height; x++) {
		int cell = x * sd.width_total + col;
		for (int time = time_start + 1; cl.cons[BIRTH][time][cell] == cl.cons[BIRTH][time - 1][cell]; time++) {
			other_row[time - time_start] = cl.cons[CMH1][time][cell];
		}
		
		for (int time = time_start; time <= time_end - interval; time += interval / 2) {
			sync_avg[(time - time_start) / (interval / 2)] += pearson_correlation(first_row, other_row, time - time_start, time - time_start + interval);
		}
	}
	
	for (int i = 0; i < num_points; i++) {
		sync_avg[i] /= (sd.height - 1);
		*file_pointer << sync_avg[i] << ",";
	}
	*file_pointer << endl;
}


double post_sync (sim_data& sd, con_levels& cl, int con, int start, int end) {  //151221: not used
	double comp_cell[end - start + 1];
	double cur_cell[end - start + 1];
	
	int middle_cell = (sd.height / 2) * sd.width_total + (sd.width_current / 2);
	
	for (int j = start; j < end; j++) {
		comp_cell[j - start] = cl.cons[con][j][middle_cell];
	}
	
	double pearson_sum = 0;
	for (int x = 0; x < sd.height; x++) {
		for (int y = 0; y < sd.width_initial; y++) {
			int cell = x * sd.width_total + y;
			
			if (cell != middle_cell) {
				for (int j = start; j < end; j++) {
					cur_cell[j - start] = cl.cons[con][j][cell];
				}
				pearson_sum += pearson_correlation(comp_cell, cur_cell, 0, end - start);
			}
		}
	}
	
	return pearson_sum / ((sd.height * sd.width_initial) - 1);
}

double pearson_correlation (double* x, double* y, int start, int end) {
	double x_avg = 0;
	double y_avg = 0;	
	double sigma_x2 = 0;
	double sigma_y2 = 0;
	double sigma_xy = 0;
	
	for (int j = start; j < end; j++) {
		x_avg += x[j];
		y_avg += y[j];
		
		
	}
	x_avg /= (end - start);
	y_avg /= (end - start);
	
	for (int j = start; j < end; j++) {
		sigma_xy += (x[j] - x_avg) * (y[j] - y_avg);
		sigma_x2 += SQUARE(x[j] - x_avg);
		sigma_y2 += SQUARE(y[j] - y_avg);
	}
	
	sigma_x2 = sqrt(sigma_x2);
	sigma_y2 = sqrt(sigma_y2);
	
	if (sigma_x2 == 0 || sigma_y2 == 0) {
		
		
		return 1;
	} else {	
		
		return sigma_xy / ((sigma_x2 * sigma_y2));
	}
}

int wave_testing (sim_data& sd, con_levels& cl, mutant_data& md, int time, int con, int active_start) { //JY WT.4.5.6.7 151221: counting number of waves
	// average the rows to create one array
	double conc[sd.width_total];
	memset(conc, 0, sizeof(double) * sd.width_total);

	for (int x = 0; x < sd.width_total; x++) {
		double avg = 0;
		for (int y = 0; y < sd.height; y++) {
			int cell = y * sd.width_total + WRAP(active_start - x, sd.width_total);
			avg += cl.cons[con][time][cell];
		}
		conc[x] = avg / sd.height;
	}

	// find the highest peak in the entire psm to set the threshold for a signal
	double thresh = 0;
	for (int x = 0; x < sd.width_total; x++) {
		if (conc[x] > thresh) {
			thresh = conc[x];
		}
	}
	thresh /= 2;

	int num_waves = 0;
	pair <int, int> waves[3];
	for (int wave = 0; wave < 3; wave++) {
		waves[wave].first = 0;
		waves[wave].second = sd.width_total;
	}

	// count the number of waves in the anterior
	for (int x = 0; x < sd.width_total; x++) {
		// check for wave start
		if (conc[x] >= thresh && (x == 0 || conc[x - 1] < thresh)) {
			if (num_waves == 3) {
				num_waves++;
				break;
			}
			waves[num_waves].first = (x == 0 ? 0 : x - 1);
		}

		// check for wave end
		if (conc[x] < thresh && x > 0 && (conc[x - 1] >= thresh)) {
			if (num_waves == 3) {
				num_waves++;
				break;
			}
			waves[num_waves].second = x;			
			num_waves++;
		}
	}
	
	int wlength_post = 5, wlength_ant = 2;
	if (num_waves <= 3) {
		for (int wave = 0; wave < num_waves; wave++) {
			int start = waves[wave].first;
			int end = waves[wave].second;
			int mid = (end - start) / 2;
			if (mid > sd.width_initial && mid < 0.8 * sd.width_total) {
				wlength_post = end - start + 1;
			}
			if (mid >= 0.8 * sd.width_total) {
				wlength_ant = end - start + 1;
			}
		}
	}
	int result = md.wave_test(waves, num_waves, md, wlength_post, wlength_ant); //JY WT.5.6.7
	return result;
}


int wave_testing_her1 (sim_data& sd, con_levels& cl, mutant_data& md, int time, int active_start) { //151221: counting number of waves of her1 expression for her1 mutant, notused
	// average the rows to create one array
	double conc[sd.width_total];
	memset(conc, 0, sizeof(double) * sd.width_total);
	int cur_score = 0;
	
	for (int her1or7 = 0; her1or7 <1 ; her1or7++){
		for (int x = 0; x < sd.width_total; x++) {
			double avg = 0;
			for (int y = 0; y < sd.height; y++) {
				int cell = y * sd.width_total + WRAP(active_start - x, sd.width_total);
				if (her1or7 ==0){
					avg += cl.cons[CMH1][time][cell];
				} else {
					avg += cl.cons[CMH7][time][cell];
				}	
			}
			conc[x] = avg / sd.height;
		}
	
	// find the highest peak in the entire psm to set the threshold for a signal
		double thresh = 0;
		for (int x = 0; x < sd.width_total; x++) {
			if (conc[x] > thresh) {
				thresh = conc[x];
			}
		}
		thresh /= 2;

		int num_waves = 0;
		pair <int, int> waves[3];
		for (int wave = 0; wave < 3; wave++) {
			waves[wave].first = 0;
			waves[wave].second = sd.width_total;
		}

		// count the number of waves in the anterior
		for (int x = 0; x < sd.width_total; x++) {
		// check for wave start
			if (conc[x] >= thresh && (x == 0 || conc[x - 1] < thresh)) {
				if (num_waves == 3) {
					num_waves++;
					break;
				}
				waves[num_waves].first = (x == 0 ? 0 : x - 1);
			}

		// check for wave end
			if (conc[x] < thresh && x > 0 && (conc[x - 1] >= thresh)) {
				if (num_waves == 3) {
					num_waves++;
					break;
				}
				waves[num_waves].second = x;			
				num_waves++;
			}
		}
		cur_score = test_her1_wave(waves, num_waves, md, 0, 0);
	}
	return cur_score;
}

void wave_testing_mesp (sim_data& sd, con_levels& cl, mutant_data& md, int time, int active_start) { //151221: counting the number of waves of mesp gene expression
	// average the rows to create one array
	double conc[sd.width_total];
	memset(conc, 0, sizeof(double) * sd.width_total);
	
	
	for (int mespaorb = 0; mespaorb <2 ; mespaorb++){
		for (int x = 0; x < sd.width_total; x++) {
			double avg = 0;
			for (int y = 0; y < sd.height; y++) {
				int cell = y * sd.width_total + WRAP(active_start - x, sd.width_total);
				if (mespaorb ==0){
					avg += cl.cons[CMMESPA][time][cell];
				} else {
					avg += cl.cons[CMMESPB][time][cell];
				}	
			}
			conc[x] = avg / sd.height;
		}
	
	// find the highest peak in the entire psm to set the threshold for a signal
		double thresh = 0;
		for (int x = 0; x < sd.width_total; x++) {
			if (conc[x] > thresh) {
				thresh = conc[x];
			}
		}
		thresh /= 2;

		int num_waves = 0;
		pair <int, int> waves[3];
		for (int wave = 0; wave < 3; wave++) {
			waves[wave].first = 0;
			waves[wave].second = sd.width_total;
		}

		// count the number of waves in the anterior
		for (int x = 0; x < sd.width_total; x++) {
		// check for wave start
			if (conc[x] >= thresh && (x == 0 || conc[x - 1] < thresh)) {
				if (num_waves == 3) {
					num_waves++;
					break;
				}
				waves[num_waves].first = (x == 0 ? 0 : x - 1);
			}

		// check for wave end
			if (conc[x] < thresh && x > 0 && (conc[x - 1] >= thresh)) {
				if (num_waves == 3) {
					num_waves++;
					break;
				}
				waves[num_waves].second = x;			
				num_waves++;
			}
		}
		int wlength_post=5;
		int wlength_ant=3;
		md.conds_passed[SEC_ANT][8] = 1;
		if (mespaorb == 0) {
			md.conds_passed[SEC_ANT][6] = md.conds_passed[SEC_ANT][6]&&(1<=num_waves && num_waves<=2);
			for (int wave = 0; wave < num_waves; wave++) {
				int start = waves[wave].first;
				int end = waves[wave].second;
				int mid = (end - start) / 2;
				if (mid > sd.width_initial && mid < 0.8 * sd.width_total) {
					wlength_post = end - start + 1;
					if (wlength_post<3 || wlength_post>5){
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&0;
						break;
					} else {
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&1;
					}
				}
				if (mid >= 0.8 * sd.width_total) {
					wlength_ant = end - start + 1;
					if (wlength_ant<2 || wlength_post>3){
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&0;
						break;
					} else {
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&1;
					}
				}
			}
		} else {
			md.conds_passed[SEC_ANT][7] = md.conds_passed[SEC_ANT][7]&&(2<=num_waves && num_waves<=3);
			for (int wave = 0; wave < num_waves; wave++) {
				int start = waves[wave].first;
				int end = waves[wave].second;
				int mid = (end - start) / 2;
				if (mid > sd.width_initial && mid < 0.8 * sd.width_total) {
					wlength_post = end - start + 1;
					if (wlength_post<3 || wlength_post>5){
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&0;
						break;
					} else {
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&1;
					}
				}
				if (mid >= 0.8 * sd.width_total) {
					wlength_ant = end - start + 1;
					if (wlength_ant<2 || wlength_post>3){
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&0;
						break;
					} else {
						md.conds_passed[SEC_ANT][8] = md.conds_passed[SEC_ANT][8]&&1;
					}
				}
			}
		}
	}
	return;
}
