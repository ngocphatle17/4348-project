#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>

using namespace std;

// generate timsestamp in the format
string get_timestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", ltm);
    return string(buffer);
}

int main(int argc, char* argv[]) {
    // ensure correct number of arguments are provided
    if (argc != 2) {
        cerr << "Usage: ./logger <logfile>" << endl;
        return 1;
    }

    string logfile = argv[1]; // store
    ofstream log_file(logfile.c_str(), ios::app);
    
    // terminates if file is failed to open
    if (!log_file) {
        cerr << "Error opening log file!" << endl;
        return 1;
    }

    string line;
    while (getline(cin, line)) { // read input ilnes from the standard input
        if (line == "QUIT") break;
        
        // extract action and messages
        size_t space_pos = line.find(' ');
        string action = (space_pos != string::npos) ? line.substr(0, space_pos) : line;
        string message = (space_pos != string::npos) ? line.substr(space_pos + 1) : "";
        
        // write timestamp
        log_file << get_timestamp() << " [" << action << "] " << message << endl;
    }

    log_file.close();
    return 0;
}
