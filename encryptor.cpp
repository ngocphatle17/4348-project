#include <iostream>
#include <string>

using namespace std;

string passkey = "";  // Stores the passkey

// Vigenère Cipher Function
string vigenere_cipher(string text, string key, bool encrypt) {
    string result = "";
    int key_len = key.length();
    for (size_t i = 0; i < text.length(); i++) {
        char base = 'A';
        int text_pos = text[i] - base;
        int key_pos = key[i % key_len] - base;
        char new_char = encrypt ? 
            (text_pos + key_pos) % 26 + base : 
            (text_pos - key_pos + 26) % 26 + base;
        result += new_char;
    }
    return result;
}

int main() {
    string command, argument;
    
    while (getline(cin, command, ' ')) {  // Fix: Read the first word as the command
        getline(cin, argument);  // Fix: Read the rest as the argument

        if (command == "PASS") {
            passkey = argument;  // Store passkey
            cout << "RESULT" << endl;
        } 
        else if (command == "ENCRYPT") {
            if (passkey.empty()) {
                cout << "ERROR Password not set" << endl;
            } else {
                cout << "RESULT " << vigenere_cipher(argument, passkey, true) << endl;
            }
        } 
        else if (command == "DECRYPT") {
            if (passkey.empty()) {
                cout << "ERROR Password not set" << endl;
            } else {
                cout << "RESULT " << vigenere_cipher(argument, passkey, false) << endl;
            }
        } 
        else if (command == "QUIT") {
            break;
        }
    }

    return 0;
}
