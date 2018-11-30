#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <climits>
#include <algorithm>

using namespace std;

	int thread_num;
	pthread_mutex_t** locks;

	int total_time;
	double absorb_rate;
	int land_size;
	string file_name;
	vector<vector<int> > dir = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	vector<vector<int> > landscape;
	vector<vector<double> > current_amount;
	vector<vector<double> > absorb_amount;
	vector<vector<double> > trickle_in;
	vector< vector< vector<int> > > trickle_off_dir;
	int curr_timestep;
	bool all_done;

	double calc_time(struct timespec start, struct timespec end) {
		double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
		double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;
		if (end_sec < start_sec) {
			return 0;
		} else {
			return end_sec - start_sec;
		}
	}

	void PrintResult(struct timespec &start_time, struct timespec &end_time) {
		double elapsed_s = calc_time(start_time, end_time) / 1000000000.0;

		cout << "Rainfall simulation completed in "  << curr_timestep << " time steps" << endl;
		cout << "Runtime = " << elapsed_s << " seconds" << endl;
		cout << endl;
		cout << "The following grid shows the number of raindrops absorbed at each point: " << endl;
		for (auto row : absorb_amount) {
			for (auto value : row) {
				cout << setw(8) << setprecision(6) << value;
			}
			cout << endl;
		}
	}

	void PrintCurrentAmount() {
		for (auto row: current_amount) {
			for (auto value: row)
				cout << value << " " ;
			cout << endl;
		}
	}


	bool Parse(int argc, char *argv[]) {
		try {
			if (argc != 6) {
				cout << "The number of arguments is wrong." << endl;
				return false;
			}
			thread_num = stoi(argv[1]);
			total_time = stoi(argv[2]);
			absorb_rate = stod(argv[3]);
			land_size = stoi(argv[4]);
			file_name = argv[5];
			return true;
  		}
  		catch (std::exception& e) {
			cout << e.what() << endl;
			return false;
		}
	}

	bool ReadFile() {
		try {
			landscape.assign(land_size, vector<int>(land_size, 0));
			std::ifstream ifs(file_name);
			for (int i=0; i<land_size; i++) {
				for (int j=0; j<land_size; j++) {
					ifs >> landscape[i][j];
				}
			}
			ifs.close();
			return true;
		} catch (std::exception& e) {
			cout << e.what() << endl;
			return false;
		}
	}

	void Preprocessing() {
		current_amount.assign(land_size, vector<double>(land_size, 0.0));
		absorb_amount.assign(land_size, vector<double>(land_size, 0.0));
    	trickle_in.assign(land_size, vector<double>(land_size, 0.0));

		trickle_off_dir.assign(land_size, vector< vector<int> >(land_size, vector<int>(0) ));
		
		for (int i=0; i<land_size; i++) {
			for (int j=0; j<land_size; j++) {
				int min_height = INT_MAX;
				for (int k=0; k<4; k++) {
				if (i+dir[k][0]>=0 && i+dir[k][0]<land_size && j+dir[k][1]>=0 && j+dir[k][1]<land_size) {
						if (landscape[i+dir[k][0]][j+dir[k][1]] < min_height)
							min_height = landscape[i+dir[k][0]][j+dir[k][1]];
					}
				}

				if (min_height >= landscape[i][j])
					continue;

				for (int k=0; k<4; k++) {
					if (i+dir[k][0]>=0 && i+dir[k][0]<land_size && j+dir[k][1]>=0 && j+dir[k][1]<land_size) {
						if (landscape[i+dir[k][0]][j+dir[k][1]] == min_height)
							trickle_off_dir[i][j].push_back(k);
					}
				}
			//cout << i << " " << j << " " << trickle_off_dir[i][j].size() << endl;
			}
		}
	}


	void CreateMutex() {
		locks = (pthread_mutex_t**) malloc(land_size * sizeof(pthread_mutex_t*));
		for(int i=0; i<land_size; i++)  
			locks[i] = (pthread_mutex_t*) malloc(land_size * sizeof(pthread_mutex_t));
		
		for (int i = 0; i < land_size; i++) {
			for (int j = 0; j<land_size; j++) {
				pthread_mutex_init(&locks[i][j], NULL);
			}
		}
	}


	 void* InitTrickleInHelper(void* arg) {
		int id = *(int *)arg;
		int start = id * (land_size / thread_num);
		int end = (id + 1) * (land_size / thread_num);

		for (int i=start; i<end; i++) {
			for (int j=0; j<land_size; j++) {
				trickle_in[i][j] = 0.0;
			}
		}
		return NULL;
	}

	void InitTrickleIn() {
		pthread_t* threads = (pthread_t *) malloc(thread_num * sizeof(pthread_t));
		for (int i = 0; i < thread_num; i++) {    
			int *id = (int *) malloc(sizeof(int));  
			*id = i;    
			pthread_create(&threads[i], NULL, InitTrickleInHelper, (void *)(id));  
		}
		for (int i = 0; i < thread_num; i++) {    
			pthread_join(threads[i], NULL);  
		}

	}

	void* FirstTraverseHelper(void* arg) {
		int id = *(int *)arg;
		int start = id * (land_size / thread_num);
		int end = (id + 1) * (land_size / thread_num);

    	for (int i=start; i<end; i++) {
    		for (int j=0; j<land_size; j++) {
    			if (curr_timestep <= total_time)
    				current_amount[i][j] += 1.0;

				absorb_amount[i][j] += min(current_amount[i][j], absorb_rate);
				current_amount[i][j] -= min(current_amount[i][j], absorb_rate);
				if (trickle_off_dir[i][j].size() > 0) {
					double trickle_off_amount = min(current_amount[i][j], 1.0);
					current_amount[i][j] -= trickle_off_amount;
					for (size_t k=0; k<trickle_off_dir[i][j].size(); k++) {
						pthread_mutex_lock(&locks[i+dir[trickle_off_dir[i][j][k]][0]][j+dir[trickle_off_dir[i][j][k]][1]]);
						trickle_in[i+dir[trickle_off_dir[i][j][k]][0]][j+dir[trickle_off_dir[i][j][k]][1]] += \
						trickle_off_amount / (trickle_off_dir[i][j].size()*1.0);
						pthread_mutex_unlock(&locks[i+dir[trickle_off_dir[i][j][k]][0]][j+dir[trickle_off_dir[i][j][k]][1]]);
					}
				}
			}
		}
		return NULL;
	}


	void FirstTraverse() {
		pthread_t* threads = (pthread_t *) malloc(thread_num * sizeof(pthread_t));
		for (int i = 0; i < thread_num; i++) {    
			int *id = (int *) malloc(sizeof(int));  
			*id = i;    
			pthread_create(&threads[i], NULL, FirstTraverseHelper, (void *)(id));  
		}
		for (int i = 0; i < thread_num; i++) {    
			pthread_join(threads[i], NULL);  
		}
	}

	void* SecondTraverseHelper(void* arg) {
		int id = *(int *)arg;
		int start = id * (land_size / thread_num);
		int end = (id + 1) * (land_size / thread_num);

		for (int i=start; i<end; i++) {
			for (int j=0; j<land_size; j++) {
				current_amount[i][j] += trickle_in[i][j];
				if (current_amount[i][j] > 0.0) all_done = false;
			}
		}
		return NULL;
	}

	void SecondTraverse() {
		pthread_t* threads = (pthread_t *) malloc(thread_num * sizeof(pthread_t));
		for (int i = 0; i < thread_num; i++) {    
			int *id = (int *) malloc(sizeof(int));  
			*id = i;    
			pthread_create(&threads[i], NULL, SecondTraverseHelper, (void *)(id));  
		}
		for (int i = 0; i < thread_num; i++) {    
			pthread_join(threads[i], NULL);  
		}

	}


	void Simulate() {
		struct timespec start_time, end_time;

		CreateMutex();

		clock_gettime(CLOCK_MONOTONIC, &start_time);

    	curr_timestep = 1;
    	while (true) {
    		InitTrickleIn();
    		FirstTraverse();
			all_done = true;
			SecondTraverse();
			if (all_done && curr_timestep>total_time) break;
			curr_timestep++;
		}

		clock_gettime(CLOCK_MONOTONIC, &end_time);
		PrintResult(start_time, end_time);
	}






int main(int argc, char *argv[]) {
	if (!Parse(argc, argv)) 
		return 0;
	if (!ReadFile()) 
		return 0;
	Preprocessing();
	Simulate();

	return 0;
}

