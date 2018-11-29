#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class SequencialRainfallSimulator {
private:
	int total_time;
	int absorb_rate;
	int land_size;
	string file_name;
	vector<vector<int> > dir = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	vector<vector<int> > landscape;
	vector<vector<double> > current_amount;
	vector<vector<double> > absorb_amount;
	vector<vector<double> > trickle_in;
	vector< vector< vector<int> > > trickle_off_dir;


public:
	bool Parse(int argc, char *argv[]) {
		try {
			if (argc != 6) {
				cout << "The number of arguments is wrong." << endl;
				return false;
			}
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
	return false;
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

    }







};



int main(int argc, char *argv[]) {
	SequencialRainfallSimulator mysimulator;
	if (!mysimulator.Parse(argc, argv)) 
		return 0;
	if (!mysimulator.ReadFile()) 
		return 0;
	mysimulator.Preprocessing();





	return 0;
}

