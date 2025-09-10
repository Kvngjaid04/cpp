/*  Author: Ireoluwa 
 * Date: 2025-09-10
 * Purpose: Simulate a 20-team football league season with realistic scoring, form tracking, cross table, stats, and European qualification.
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <random>
using namespace std;

// Team structure to store stats
struct Team {
    string name;
    int points;
    int goalsFor;
    int goalsAgainst;
    int wins;
    int draws;
    int losses;
    double winRate; // probability baseline for winning a match
    double lambda;  // expected goals per match
    vector<char> form; // last 5 results
};

// Globals
vector<vector<string>> crossTable(20, vector<string>(20, ""));

// Function prototypes
void initTeams(vector<Team>& teams);
void simulateSeason(vector<Team>& teams);
void playMatch(Team& home, Team& away, int hi, int ai, default_random_engine& gen);
int poissonGoals(double lambda, default_random_engine& gen);
void printTable(vector<Team>& teams);
void printCrossTable(vector<Team>& teams);
void printStats(vector<Team>& teams);
void assignEuropeanPlaces(vector<Team>& teams);

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(0)));
    default_random_engine gen(time(0));

    // Declaring Variables
    vector<Team> teams;

    // Initialize Variables
    initTeams(teams);

    // Processing - simulate the whole season
    simulateSeason(teams);

    // Displaying final league table
    printTable(teams);
    printCrossTable(teams);
    printStats(teams);
    assignEuropeanPlaces(teams);

    return 0;
}

void initTeams(vector<Team>& teams) {
    for (char c = 'A'; c <= 'T'; c++) {
        Team t;
        t.name = string(1, c);
        t.points = 0;
        t.goalsFor = 0;
        t.goalsAgainst = 0;
        t.wins = t.draws = t.losses = 0;
        t.form = {};

        if (c >= 'A' && c <= 'H') {
            t.winRate = 48 + rand() % 8; // 48–55%
            t.lambda = 2.0 + (rand() % 11) / 10.0; // 2.0–3.0
        } else if (c >= 'P' && c <= 'T') {
            t.winRate = 40 + rand() % 6; // 40–45%
            t.lambda = 0.5 + (rand() % 6) / 10.0; // 0.5–1.0
        } else {
            t.winRate = 45 + rand() % 9; // 45–53%
            t.lambda = 1.3 + (rand() % 8) / 10.0; // 1.3–2.0
        }
        t.winRate /= 100.0; // make into probability

        teams.push_back(t);
    }
}

void simulateSeason(vector<Team>& teams) {
    int n = teams.size();
    default_random_engine gen(time(0));
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            // Each pair plays twice: home and away
            playMatch(teams[i], teams[j], i, j, gen);
            playMatch(teams[j], teams[i], j, i, gen);
        }
    }
}

int poissonGoals(double lambda, default_random_engine& gen) {
    poisson_distribution<int> dist(lambda);
    return dist(gen);
}

void playMatch(Team& home, Team& away, int hi, int ai, default_random_engine& gen) {
    double homeAdvantage = 0.2; // slight boost to home team goals

    int homeGoals = poissonGoals(home.lambda + homeAdvantage, gen);
    int awayGoals = poissonGoals(away.lambda, gen);

    // Rare explosion: 1–2% chance for a crazy match (5–10 goals)
    if ((rand() % 100) < 2) homeGoals = 5 + rand() % 6;
    if ((rand() % 100) < 2) awayGoals = 5 + rand() % 6;

    // Update goals
    home.goalsFor += homeGoals;
    home.goalsAgainst += awayGoals;
    away.goalsFor += awayGoals;
    away.goalsAgainst += homeGoals;

    string score = to_string(homeGoals) + "-" + to_string(awayGoals);
    crossTable[hi][ai] = score;

    // Decide result
    if (homeGoals > awayGoals) {
        home.points += 3;
        home.wins++;
        away.losses++;
        home.form.push_back('W');
        away.form.push_back('L');
    } else if (awayGoals > homeGoals) {
        away.points += 3;
        away.wins++;
        home.losses++;
        home.form.push_back('L');
        away.form.push_back('W');
    } else {
        home.points++;
        away.points++;
        home.draws++;
        away.draws++;
        home.form.push_back('D');
        away.form.push_back('D');
    }

    if (home.form.size() > 5) home.form.erase(home.form.begin());
    if (away.form.size() > 5) away.form.erase(away.form.begin());
}

void printTable(vector<Team>& teams) {
    // Sort table by points, then goal difference, then goals for
    sort(teams.begin(), teams.end(), [](const Team& a, const Team& b) {
        if (a.points != b.points) return a.points > b.points;
        int gdA = a.goalsFor - a.goalsAgainst;
        int gdB = b.goalsFor - b.goalsAgainst;
        if (gdA != gdB) return gdA > gdB;
        return a.goalsFor > b.goalsFor;
    });

    cout << left << setw(5) << "Pos" << setw(6) << "Team" << setw(8) << "Points" << setw(6) << "W"
         << setw(6) << "D" << setw(6) << "L" << setw(8) << "GF" << setw(8) << "GA"
         << setw(8) << "GD" << "Last5" << endl;
    cout << "-----------------------------------------------------------------------\n";

    for (int i = 0; i < teams.size(); i++) {
        auto& t = teams[i];
        int gd = t.goalsFor - t.goalsAgainst;
        cout << left << setw(5) << (i + 1) << setw(6) << t.name << setw(8) << t.points << setw(6) << t.wins
             << setw(6) << t.draws << setw(6) << t.losses << setw(8) << t.goalsFor
             << setw(8) << t.goalsAgainst << setw(8) << gd;

        for (char r : t.form) cout << r;
        cout << endl;
    }
}

void printCrossTable(vector<Team>& teams) {
    cout << "\nCross Table (Scores)\n";
    cout << setw(4) << " ";
    for (auto& t : teams) cout << setw(4) << t.name;
    cout << endl;

    for (int i = 0; i < teams.size(); i++) {
        cout << setw(4) << teams[i].name;
        for (int j = 0; j < teams.size(); j++) {
            if (i == j) cout << setw(4) << " ";
            else cout << setw(4) << crossTable[i][j];
        }
        cout << endl;
    }
}

void printStats(vector<Team>& teams) {
    cout << "\nTeam Statistics\n";
    for (auto& t : teams) {
        double avgPts = (double)t.points / 38.0;
        double avgGF = (double)t.goalsFor / 38.0;
        double avgGA = (double)t.goalsAgainst / 38.0;
        cout << t.name << ": AvgPts=" << fixed << setprecision(2) << avgPts
             << " AvgGF=" << avgGF << " AvgGA=" << avgGA << endl;
    }
}

void assignEuropeanPlaces(vector<Team>& teams) {
    cout << "\nQualifications & Relegation\n";
    cout << "🏆 League Champion: " << teams[0].name << endl;

    cout << "UEFA Champions League group stage: ";
    for (int i = 0; i < 4; i++) cout << teams[i].name << " ";
    cout << "\n";

    // Random FA Cup + League Cup winners
    int faCup = rand() % 20;
    int lgCup = rand() % 20;

    cout << "FA Cup Winner: " << teams[faCup].name << endl;
    cout << "League Cup Winner: " << teams[lgCup].name << endl;

    cout << "Europa League group stage: " << teams[4].name << " (5th place), " << teams[faCup].name << " (FA Cup)\n";
    cout << "Europa Conference League: " << teams[lgCup].name << " (League Cup)\n";

    cout << "Relegation: ";
    for (int i = 17; i < 20; i++) cout << teams[i].name << " ";
    cout << "⚠️\n";
}
