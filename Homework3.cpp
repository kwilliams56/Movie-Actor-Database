/*
 Name: Kravion Williams
 Email: Kwilliams56@crimson.ua.edu
 Course Section: Fall 2025 CS 201
 Homework #: 3
*/

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

// Trim whitespace from both ends
static inline string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split actor names by comma or semicolon
static vector<string> splitActors(const string &s) {
    vector<string> actors;
    string token;
    for (char c : s) {
        if (c == ',' || c == ';') {
            if (!token.empty()) {
                actors.push_back(trim(token));
                token.clear();
            }
        } else token.push_back(c);
    }
    if (!token.empty()) actors.push_back(trim(token));
    return actors;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <db file> <query file>" << endl;
        return -1;
    }

    string dbFileName = argv[1];
    string queryFileName = argv[2];

    ifstream dbfile(dbFileName);
    if (!dbfile.is_open()) {
        cout << "Unable to open file: " << dbFileName << endl;
        return -1;
    }

    cout << "==============================================================" << endl;
    cout << "*** Reading DB file: " << dbFileName << " ***" << endl;

    auto startBuild = chrono::steady_clock::now();

    unordered_map<string, unordered_set<string>> movieToActorsSet;
    unordered_map<string, unordered_set<string>> actorToMoviesSet;

    regex delim("/");
    string line;
    int movieCount = 0;
    long long recordCount = 0;  // total movie–actor pairs

    while (getline(dbfile, line)) {
        line = trim(line);
        if (line.empty()) continue;

        auto begin = sregex_token_iterator(line.begin(), line.end(), delim, -1);
        auto end = sregex_token_iterator();
        if (begin == end) continue;

        string movie = trim(*begin);
        ++begin;

        string actorLine;
        for (auto it = begin; it != end; ++it) {
            if (!actorLine.empty()) actorLine += " ";
            actorLine += *it;
        }

        vector<string> actors = splitActors(actorLine);
        for (auto &actor : actors) {
            if (actor.empty()) continue;
            movieToActorsSet[movie].insert(actor);
            actorToMoviesSet[actor].insert(movie);
            ++recordCount;
        }
        ++movieCount;
    }
    dbfile.close();

    // Convert sets → sorted vectors for stable output
    unordered_map<string, vector<string>> movieToActors;
    unordered_map<string, vector<string>> actorToMovies;
    for (auto &m : movieToActorsSet) {
        movieToActors[m.first] = vector<string>(m.second.begin(), m.second.end());
        sort(movieToActors[m.first].begin(), movieToActors[m.first].end());
    }
    for (auto &a : actorToMoviesSet) {
        actorToMovies[a.first] = vector<string>(a.second.begin(), a.second.end());
        sort(actorToMovies[a.first].begin(), actorToMovies[a.first].end());
    }

    auto endBuild = chrono::steady_clock::now();
    double buildTime = chrono::duration<double, milli>(endBuild - startBuild).count();

    cout << "*** Done reading DB file ***" << endl;
    cout << "Number of movies: " << movieCount << endl;
    cout << "Number of records (movie-actor pairs): " << recordCount << endl;

    // --- Process Queries ---
    ifstream queryFile(queryFileName);
    if (!queryFile.is_open()) {
        cout << "Unable to open file: " << queryFileName << endl;
        return -1;
    }

    cout << "==============================================================" << endl;
    cout << "*** Reading query file: " << queryFileName << " ***" << endl;

    string query;
    int queryCount = 0, matches = 0;

    auto startSearch = chrono::steady_clock::now();

    while (getline(queryFile, query)) {
        query = trim(query);
        if (query.empty()) continue;
        ++queryCount;

        auto mIt = movieToActors.find(query);
        if (mIt != movieToActors.end()) {
            cout << "Actors in \"" << query << "\":" << endl;
            for (auto &a : mIt->second)
                cout << "  " << a << endl;
            ++matches;
            continue;
        }

        auto aIt = actorToMovies.find(query);
        if (aIt != actorToMovies.end()) {
            cout << "Movies featuring \"" << query << "\":" << endl;
            for (auto &m : aIt->second)
                cout << "  " << m << endl;
            ++matches;
            continue;
        }

        cout << "\"" << query << "\" Not Found" << endl;
    }
    queryFile.close();

    auto endSearch = chrono::steady_clock::now();
    double searchTime = chrono::duration<double, milli>(endSearch - startSearch).count();
    double totalTime = chrono::duration<double, milli>(endSearch - startBuild).count();

    // --- Final Summary ---
    cout << "==============================================================" << endl;
    cout << "******************** FINAL SUMMARY ****************************" << endl;
    cout << "Number of movies: " << movieCount << endl;
    cout << "Number of records (movie-actor pairs): " << recordCount << endl;
    cout << "Number of queries: " << queryCount << endl;
    cout << "Number of matches found: " << matches << endl;
    cout << "--------------------------------------------------------------" << endl;
    cout << "Time to create data structure (ms): " << buildTime << endl;
    cout << "Time to search (ms): " << searchTime << endl;
    cout << "Total time (ms): " << totalTime << endl;
    cout << "==============================================================" << endl;

    return 0;
}
