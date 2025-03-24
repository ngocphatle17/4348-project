#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <cstdlib>  

using namespace std;

void write_to_pipe(int fd, const string &message) {
    string formatted_message = message + "\n";  // Ensure newline is sent
    write(fd, formatted_message.c_str(), formatted_message.size());
}

string read_from_pipe(int fd) {
    char buffer[1024]; // Hold new message
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        return string(buffer);
    }
    return ""; // empty if no data is read
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./driver <logfile>" << endl; // ensure the program uses the correct one.
        return 1;
    }

    string logfile = argv[1]; // store log file name
  
    // declare pipes
    int log_pipe[2], enc_pipe_in[2], enc_pipe_out[2];
    pipe(log_pipe);
    pipe(enc_pipe_in);
    pipe(enc_pipe_out);
    
    // Logger
    pid_t logger_pid = fork();
    if (logger_pid == 0) {
        dup2(log_pipe[0], STDIN_FILENO); // redirect standard input to read from pipe
        close(log_pipe[1]);
        execl("./logger", "logger", logfile.c_str(), NULL); // execute logger
        cerr << "Error: Failed to launch logger!" << endl;
        exit(1);
    }
    close(log_pipe[0]);
    
    // Encryptor
    pid_t enc_pid = fork();
    if (enc_pid == 0) {
        dup2(enc_pipe_in[0], STDIN_FILENO); // redirect standard input to read from pipe
        dup2(enc_pipe_out[1], STDOUT_FILENO); // redirect standard output to write to pipe
        close(enc_pipe_in[1]);
        close(enc_pipe_out[0]);
        execl("./encryptor", "encryptor", NULL);  // execute encryptor
        cerr << "Error: Failed to launch encryptor!" << endl;
        exit(1);
    }
    
    // close any unused programs
    close(enc_pipe_in[0]);
    close(enc_pipe_out[1]);
    
    write_to_pipe(log_pipe[1], "START Driver Started.");

    vector<string> history;
    string command, input;

    while (true) {
        cout << "Enter command (password/encrypt/decrypt/history/quit): " << flush;  // ensures prompt appears
        
        // handle input failure
        if (!getline(cin, command)) {  // prevents infinite loop if input fails
            cerr << "[ERROR] Failed to read input. Exiting." << endl;
            break;
        }

        if (command.empty()) continue;  // ignore empty input

        if (command == "password") {
            cout << "Enter passkey: " << flush;  // ensures prompt appears immediately
            
            if (!getline(cin, input)) {
                cerr << "[ERROR] Failed to read passkey input." << endl;
                break;
            }

            write_to_pipe(enc_pipe_in[1], "PASS " + input);
            write_to_pipe(log_pipe[1], "PASSKEY Passkey set.");
            cout << "[DEBUG] Passkey set to: " << input << endl;  // Debugging message
        } 
        else if (command == "encrypt") {
            cout << "Enter text: " << flush;
            if (!getline(cin, input)) break;

            write_to_pipe(enc_pipe_in[1], "ENCRYPT " + input);
            string response = read_from_pipe(enc_pipe_out[0]);

            cout << response << endl;
            history.push_back(input);
            write_to_pipe(log_pipe[1], "ENCRYPT " + input);

            if (response.find("RESULT") != string::npos) {
                write_to_pipe(log_pipe[1], response);  // Log encryption result
                cout << "[DEBUG] Logged encryption result: " << response << endl;
            }
        } 
        else if (command ==      "decrypt") {
            cout << "Enter encrypted text: " << flush;
            if (!getline(cin, input)) break;

            write_to_pipe(enc_pipe_in[1], "DECRYPT " + input);
            string response = read_from_pipe(enc_pipe_out[0]);

            cout << response << endl;
            history.push_back(input);
            write_to_pipe(log_pipe[1], "DECRYPT " + input);

            if (response.find("RESULT") != string::npos) {
                write_to_pipe(log_pipe[1], response);  // Log decryption result
                cout << "[DEBUG] Logged decryption result: " << response << endl;
            }
        } 
        else if (command == "history") {
            for (size_t i = 0; i < history.size(); i++) {
                cout << i + 1 << ". " << history[i] << endl;
            }
        } 
        else if (command == "quit") {
            write_to_pipe(enc_pipe_in[1], "QUIT");
            write_to_pipe(log_pipe[1], "QUIT");
            break;
        } 
        else {
            cout << "Invalid command! Try again." << endl;
        }
    }

    waitpid(logger_pid, NULL, 0);
    waitpid(enc_pid, NULL, 0);
    return 0;
}
