#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

struct Process {
    int id;
    int arrival;
    int burst;
    int priority;
    int start = -1;
    int finish = -1;
    int remaining;
    bool completed = false;
};

struct Metrics {
    string algorithm;
    double throughput;
    double cpu_util;
    double avg_tat;
    double avg_rt;
    double avg_wt;
};

// Calculate
Metrics calculate_metrics(const vector<Process>& processes, const string& algo, int total_time) {
    Metrics m;
    m.algorithm = algo;
    int total_tat = 0, total_rt = 0, total_wt = 0;
    int total_burst = 0;

    for (const auto& p : processes) {
        total_burst += p.burst;
        int tat = p.finish - p.arrival;
        int rt = p.start - p.arrival;
        int wt = tat - p.burst;

        total_tat += tat;
        total_rt += rt;
        total_wt += wt;
    }

    m.throughput = static_cast<double>(processes.size()) / total_time;
    m.cpu_util = (static_cast<double>(total_burst) / total_time) * 100;
    m.avg_tat = static_cast<double>(total_tat) / processes.size();
    m.avg_rt = static_cast<double>(total_rt) / processes.size();
    m.avg_wt = static_cast<double>(total_wt) / processes.size();

    return m;
}

// FCFS
Metrics FCFS(vector<Process> processes) {
    int time = 0;
    queue<int> ready_queue;
    vector<bool> added(processes.size(), false);

    while (true) {
        // Add new-arrived process
        for (size_t i = 0; i < processes.size(); ++i) {
            if (!added[i] && processes[i].arrival <= time) {
                ready_queue.push(i);
                added[i] = true;
            }
        }

        if (!ready_queue.empty()) {
            int idx = ready_queue.front();
            ready_queue.pop();

            processes[idx].start = time;
            time += processes[idx].burst;
            processes[idx].finish = time;
            processes[idx].completed = true;
        }
        else {
            time++;
        }

        // Check if all process are completed
        bool all_done = true;
        for (const auto& p : processes) {
            if (!p.completed) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;
    }

    return calculate_metrics(processes, "FCFS", time);
}

// SJF调度算法（非抢占）
Metrics SJF(vector<Process> processes) {
    int time = 0;
    vector<bool> added(processes.size(), false);
    auto cmp = [&](int a, int b) {
        return processes[a].burst > processes[b].burst;
        };
    priority_queue<int, vector<int>, decltype(cmp)> ready_queue(cmp);

    while (true) {
        // Add new-arrived process
        for (size_t i = 0; i < processes.size(); ++i) {
            if (!added[i] && processes[i].arrival <= time) {
                ready_queue.push(i);
                added[i] = true;
            }
        }

        if (!ready_queue.empty()) {
            int idx = ready_queue.top();
            ready_queue.pop();

            processes[idx].start = time;
            time += processes[idx].burst;
            processes[idx].finish = time;
            processes[idx].completed = true;
        }
        else {
            time++;
        }

        // Check if all process are completed
        bool all_done = true;
        for (const auto& p : processes) {
            if (!p.completed) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;
    }

    return calculate_metrics(processes, "SJF", time);
}

// SRTF
Metrics SRTF(vector<Process> processes) {
    int time = 0;
    vector<int> remaining(processes.size());
    vector<bool> added(processes.size(), false);
    auto cmp = [&](int a, int b) {
        return remaining[a] > remaining[b];
        };
    priority_queue<int, vector<int>, decltype(cmp)> ready_queue(cmp);

    for (size_t i = 0; i < processes.size(); ++i) {
        remaining[i] = processes[i].burst;
    }

    while (true) {
        // Add new-arrived process
        for (size_t i = 0; i < processes.size(); ++i) {
            if (!added[i] && processes[i].arrival <= time) {
                ready_queue.push(i);
                added[i] = true;
            }
        }

        if (!ready_queue.empty()) {
            int idx = ready_queue.top();
            ready_queue.pop();

            if (processes[idx].start == -1) {
                processes[idx].start = time;
            }

            time++;
            remaining[idx]--;

            if (remaining[idx] == 0) {
                processes[idx].finish = time;
                processes[idx].completed = true;
            }
            else {
                ready_queue.push(idx);
            }
        }
        else {
            time++;
        }

        // Check if all process are completed
        bool all_done = true;
        for (const auto& p : processes) {
            if (!p.completed) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;
    }

    return calculate_metrics(processes, "SRTF", time);
}

// RR
Metrics RR(vector<Process> processes, int quantum) {
    int time = 0;
    queue<int> ready_queue;
    vector<int> remaining(processes.size());
    vector<bool> added(processes.size(), false);

    for (size_t i = 0; i < processes.size(); ++i) {
        remaining[i] = processes[i].burst;
    }

    while (true) {
        // Add new-arrived process
        for (size_t i = 0; i < processes.size(); ++i) {
            if (!added[i] && processes[i].arrival <= time) {
                ready_queue.push(i);
                added[i] = true;
            }
        }

        if (!ready_queue.empty()) {
            int idx = ready_queue.front();
            ready_queue.pop();

            if (processes[idx].start == -1) {
                processes[idx].start = time;
            }

            int exec_time = min(quantum, remaining[idx]);
            remaining[idx] -= exec_time;
            time += exec_time;

            if (remaining[idx] == 0) {
                processes[idx].finish = time;
                processes[idx].completed = true;
            }
            else {
                ready_queue.push(idx);
            }
        }
        else {
            time++;
        }

        // Check if all process are completed
        bool all_done = true;
        for (const auto& p : processes) {
            if (!p.completed) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;
    }

    return calculate_metrics(processes, "RR", time);
}

// Priority Scheduling
Metrics Priority(vector<Process> processes) {
    int time = 0;
    vector<bool> added(processes.size(), false);
    auto cmp = [&](int a, int b) {
        return processes[a].priority > processes[b].priority;
        };
    priority_queue<int, vector<int>, decltype(cmp)> ready_queue(cmp);

    while (true) {
        // Add new-arrived process
        for (size_t i = 0; i < processes.size(); ++i) {
            if (!added[i] && processes[i].arrival <= time) {
                ready_queue.push(i);
                added[i] = true;
            }
        }

        if (!ready_queue.empty()) {
            int idx = ready_queue.top();
            ready_queue.pop();

            processes[idx].start = time;
            time += processes[idx].burst;
            processes[idx].finish = time;
            processes[idx].completed = true;
        }
        else {
            time++;
        }

        // Check if all process are completed
        bool all_done = true;
        for (const auto& p : processes) {
            if (!p.completed) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;
    }

    return calculate_metrics(processes, "Priority", time);
}

// Print and Save
void save_results(const vector<Metrics>& results, const string& filename) {
    ofstream file(filename);
    file << "Algorithm,Job Throughput,CPU Utilization,Average Turnaround Time,Average Response Time,Average Waiting Time\n";
    for (const auto& m : results) {
        cout << "Algorithm:" << m.algorithm << endl
            << "Job Throughput：" << m.throughput << endl
            << "CPU Utilization：" << m.cpu_util << endl
            << "Average Turnaround Time: " << m.avg_tat << endl
            << "Average Response Time: " << m.avg_rt << endl
            << "Average Waiting Time: " << m.avg_wt << endl;
        cout << endl;
        file << m.algorithm << ","
            << m.throughput << ","
            << m.cpu_util << "%,"
            << m.avg_tat << ","
            << m.avg_rt << ","
            << m.avg_wt << "\n";
    }
}

int main() {
    vector<Process> processes = {
        {0, 0, 8, 3}, {1, 1, 4, 1},
        {2, 2, 9, 4}, {3, 3, 5, 2}
    };

    vector<Metrics> results;
    results.push_back(FCFS(processes));
    results.push_back(SJF(processes));
    results.push_back(SRTF(processes));
    results.push_back(RR(processes, 2));
    results.push_back(Priority(processes));

    save_results(results, "results.csv");
    return 0;
}
