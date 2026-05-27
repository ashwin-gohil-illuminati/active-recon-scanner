//#include "Scanner.h"
#include "ThreadManager.h"
#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;
//sem_t connection_sem; //set it up later

// Function to split a string by a delimiter
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Function to check if a string is a valid IP address
bool isValidIP(const string &ip) {
    vector<string> parts = split(ip, '.');
    
    if (parts.size() != 4) {
        return false;
    }
    
    for (const string &part : parts) {
        try {
            int num = stoi(part);
            if (num < 0 || num > 255) {
                return false;
            }
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

// Function to convert IP string to 32-bit integer
uint32_t ipToInt(const string &ip) {
    vector<string> parts = split(ip, '.');
    uint32_t result = 0;
    
    for (int i = 0; i < 4; i++) {
        result = (result << 8) + stoi(parts[i]);
    }
    
    return result;
}

// Function to convert 32-bit integer to IP string
string intToIP(uint32_t ipInt) {
    string result;
    
    for (int i = 3; i >= 0; i--) {
        result += to_string((ipInt >> (i * 8)) & 0xFF);
        if (i > 0) {
            result += ".";
        }
    }
    
    return result;
}

// Function to expand a range of IPs
vector<string> expandIPRange(const string &startIP, const string &endIP) {
    vector<string> result;
    
    // If endIP is just a number (short end), construct the full end IP
    string fullEndIP = endIP;
    if (endIP.find('.') == string::npos) {
        vector<string> startParts = split(startIP, '.');
        if (startParts.size() != 4) {
            throw invalid_argument("Invalid start IP for range");
        }
        fullEndIP = startParts[0] + "." + startParts[1] + "." + startParts[2] + "." + endIP;
    }
    
    // Validate both IPs
    if (!isValidIP(startIP) || !isValidIP(fullEndIP)) {
        throw invalid_argument("Invalid IP range: " + startIP + "-" + endIP);
    }
    
    // Convert IPs to integers
    uint32_t startInt = ipToInt(startIP);
    uint32_t endInt = ipToInt(fullEndIP);
    
    // Ensure start is less than or equal to end
    if (startInt > endInt) {
        swap(startInt, endInt);
    }
    
    // Generate all IPs in the range
    for (uint32_t ipInt = startInt; ipInt <= endInt; ipInt++) {
        result.push_back(intToIP(ipInt));
    }
    
    return result;
}

// Function to parse command line arguments
vector<string> parseArguments(int argc, char *argv[]) {
    vector<string> ips;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        // Check if it's a comma-separated list
        if (arg.find(',') != string::npos) {
            vector<string> commaSeparated = split(arg, ',');
            for (string &ip : commaSeparated) {
                // Remove any leading/trailing whitespace
                ip.erase(0, ip.find_first_not_of(" \t"));
                ip.erase(ip.find_last_not_of(" \t") + 1);
                
                if (isValidIP(ip)) {
                    ips.push_back(ip);
                } else {
                    cerr << "Warning: Invalid IP in list: " << ip << endl;
                }
            }
        }
        // Check if it's a range with a dash
        else if (arg.find('-') != string::npos) {
            vector<string> rangeParts = split(arg, '-');
            if (rangeParts.size() == 2) {
                try {
                    vector<string> expandedRange = expandIPRange(rangeParts[0], rangeParts[1]);
                    ips.insert(ips.end(), expandedRange.begin(), expandedRange.end());
                } catch (const exception &e) {
                    cerr << "Warning: " << e.what() << endl;
                }
            }
        }
        // Otherwise, treat it as a single IP
        else if (isValidIP(arg)) {
            ips.push_back(arg);
        } else {
            cerr << "Warning: Invalid IP: " << arg << endl;
        }
    }
    
    return ips;
}

int main(int argc, char *argv[]){
    
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <IP address or range>" << endl;
        cerr << "Examples:" << endl;
        cerr << "  " << argv[0] << " 127.0.0.1" << endl;
        cerr << "  " << argv[0] << " 127.0.0.1-35" << endl;
        cerr << "  " << argv[0] << " 192.168.0.10-192.168.0.12" << endl;
        cerr << "  " << argv[0] << " \"127.0.0.1, 192.168.0.1\"" << endl;
        return 1;
    }
    
    // Declare the vector in the main scope, before the try block.
    // This ensures it remains accessible after the try-catch block completes.
    vector<string> ips;

    try {
        // Assign the result of the parsing function inside the try block.
        ips = parseArguments(argc, argv);
    } catch (const exception &e) {
        // If an exception occurs, 'ips' will remain empty.
        // The program handles the error and exits.
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    // Now you can safely use 'ips' AFTER the try-catch block
    // because its scope extends to the end of main().
    if (ips.empty()) {
        cout << "No valid IP addresses were provided." << endl;
    } else {
        cout << "Parsed IP addresses:" << endl;
        for (const string &ip : ips) {
            cout << ip << endl;
        }
    }
    
    cout<<ips.size()<<endl;

    Scanner::initFile("PortScanResults.html");

    
    ThreadManager tmObject = ThreadManager();
    tmObject.populate_ips(ips);
    ips.clear();
    pthread_t tmThread;
    pthread_create(&tmThread, nullptr, ThreadManager::watchThreads, &tmObject); //@this point, control leaves main
    pthread_join(tmThread, nullptr);

    Scanner::finalizeFile();


    return 0;
}