#include <bits/stdc++.h>
using namespace std;

// Map urgency to numeric values
int urgencyValue(const string& u) {
    if (u == "U1") return 4;
    if (u == "U2") return 3;
    if (u == "U3") return 2;
    return 1; // U4
}

struct Patient {
    string pid;
    string urgency;       // "U1".."U4"
    double waitMin;
    double severity;
    int originalIndex;    // arrival order
    double cost;          // computed priority cost

    void computeCost(double wU, double wW, double wS) {
        cost = wU * urgencyValue(urgency)
             + wW * waitMin
             + wS * severity;
    }
};

// ---------------------------------------------
// COMPARATOR: supports "no U1 after U3/U4" rule
// ---------------------------------------------
int mediCmp(const Patient &a, const Patient &b) {
    // Hard rule: U1 must NOT appear after U3/U4
    if (a.urgency == "U1" && (b.urgency == "U3" || b.urgency == "U4"))
        return -1;
    if (b.urgency == "U1" && (a.urgency == "U3" || a.urgency == "U4"))
        return 1;

    // Otherwise: sort by cost descending
    if (a.cost > b.cost) return -1;
    if (a.cost < b.cost) return 1;

    // Stable tie-breaker: earlier arrival first
    if (a.originalIndex < b.originalIndex) return -1;
    if (a.originalIndex > b.originalIndex) return 1;

    return 0;
}

// ------------------------------------------
// STABLE MERGE SORT
// ---------------------------------------------
vector<Patient> mergeSort(const vector<Patient>& arr) {
    int n = arr.size();
    if (n <= 1) return arr;

    int mid = n / 2;
    vector<Patient> left(arr.begin(), arr.begin() + mid);
    vector<Patient> right(arr.begin() + mid, arr.end());

    left = mergeSort(left);
    right = mergeSort(right);

    vector<Patient> merged;
    int i = 0, j = 0;

    while (i < left.size() && j < right.size()) {
        if (mediCmp(left[i], right[j]) <= 0)
            merged.push_back(left[i++]);
        else
            merged.push_back(right[j++]);
    }
    while (i < left.size()) merged.push_back(left[i++]);
    while (j < right.size()) merged.push_back(right[j++]);

    return merged;
}

// ---------------------------------------------
// GREEDY TOP-k (stable)
// ---------------------------------------------
vector<Patient> greedyTopK(vector<Patient>& pts, int k) {
    sort(pts.begin(), pts.end(), [](auto &a, auto &b) {
        if (a.cost != b.cost) return a.cost > b.cost;
        return a.originalIndex < b.originalIndex;
    });
    vector<Patient> topk(pts.begin(), pts.begin() + k);
    return topk;
}

// ---------------------------------------------
// MAIN PIPELINE
// ---------------------------------------------
vector<Patient> mediSortPipeline(
        vector<Patient> patients,
        double wU,
        double wW,
        double wS,
        double shortlistPct
    ) {

    int N = patients.size();

    // Step 1: compute costs

    for ( int i = 0 ; i < N ; i++){
        patients[i].computeCost( wU , wW , wS);
    }

    // Step 2: number of greedy-chosen patients
    int k = max(1, (int)ceil(shortlistPct * N));

    // Step 3: make a copy for greedy selection
    vector<Patient> ptsCopy = patients;
    vector<Patient> greedy = greedyTopK(ptsCopy, k);

    // Remove greedy from patients
    set<pair<string,int>> greedySet;
    for (auto &g : greedy)
        greedySet.insert({g.pid, g.originalIndex});

    vector<Patient> remaining;
    for (auto &p : patients) {
        if (!greedySet.count({p.pid, p.originalIndex}))
            remaining.push_back(p);
    }

    // Step 4: stable merge sort on remaining
    vector<Patient> sortedRemaining = mergeSort(remaining);

    // Step 5: combine
    vector<Patient> finalQueue = greedy;
    finalQueue.insert(finalQueue.end(),
                      sortedRemaining.begin(),
                      sortedRemaining.end());

    return finalQueue;
}

// ---------------------------------------------
// PRINT TABLE
// ---------------------------------------------
void printQueue(const vector<Patient>& q) {
    cout << "Rank | Patient | Urgency | Wait | Severity | Cost\n";
    cout << "-------------------------------------------------------\n";
    int r = 1;
    for (auto &p : q) {
        cout << setw(4) << r++ << " | "
             << setw(7) << p.pid << " | "
             << setw(7) << p.urgency << " | "
             << setw(4) << p.waitMin << " | "
             << setw(8) << p.severity << " | "
             << p.cost << "\n";
    }
}

// ---------------------------------------------
// EXAMPLE RUN
// ---------------------------------------------
int main() {
    // P -> Patient name
    // U -> Urgency
    // W -> Wait time
    // S -> Severity
    // C -> Cost

    vector<Patient> sample = {
        {"P1","U2",15,80,0},
        {"P2","U1",5,95,1},
        {"P3","U3",35,40,2},
        {"P4","U4",10,10,3},
        {"P5","U2",60,30,4},
        {"P6","U1",2,60,5},
        {"P7","U3",120,50,6},
        {"P8","U2",25,70,7},
        {"P9","U4",5,20,8},
        {"P10","U2",45,85,9}
    };

    auto finalQ = mediSortPipeline(
        sample,
        200.0,   // wU
        0.8,     // wW
        0.6,     // wS
        0.20     // shortlist 20%
    );

    printQueue(finalQ);

    return 0;
}
