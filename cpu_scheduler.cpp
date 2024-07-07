#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>

class Process
{
public:
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;

    Process(int p, int at, int bt) : pid(p), arrival_time(at), burst_time(bt),
                                     remaining_time(bt), completion_time(0),
                                     waiting_time(0), turnaround_time(0), response_time(-1) {}
};

class Scheduler
{
private:
    std::vector<Process> processes;
    std::vector<Process> completed_processes;
    int current_time;

public:
    Scheduler(const std::vector<Process> &procs) : processes(procs), current_time(0) {}

    void fcfs()
    {
        std::sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
                  { return a.arrival_time < b.arrival_time; });

        for (auto &process : processes)
        {
            if (current_time < process.arrival_time)
            {
                current_time = process.arrival_time;
            }
            process.completion_time = current_time + process.burst_time;
            process.turnaround_time = process.completion_time - process.arrival_time;
            process.waiting_time = process.turnaround_time - process.burst_time;
            process.response_time = current_time - process.arrival_time;
            current_time = process.completion_time;
            completed_processes.push_back(process);
        }
    }

    void sjf()
    {
        std::vector<Process> remaining_processes = processes;
        while (!remaining_processes.empty())
        {
            std::vector<Process *> available_processes;
            for (auto &p : remaining_processes)
            {
                if (p.arrival_time <= current_time)
                {
                    available_processes.push_back(&p);
                }
            }

            if (available_processes.empty())
            {
                current_time++;
                continue;
            }

            auto it = std::min_element(available_processes.begin(), available_processes.end(),
                                       [](const Process *a, const Process *b)
                                       { return a->burst_time < b->burst_time; });

            Process *process = *it;
            process->completion_time = current_time + process->burst_time;
            process->turnaround_time = process->completion_time - process->arrival_time;
            process->waiting_time = process->turnaround_time - process->burst_time;
            process->response_time = current_time - process->arrival_time;
            current_time = process->completion_time;
            completed_processes.push_back(*process);

            remaining_processes.erase(std::remove_if(remaining_processes.begin(), remaining_processes.end(),
                                                     [process](const Process &p)
                                                     { return p.pid == process->pid; }),
                                      remaining_processes.end());
        }
    }

    void srtf()
    {
        std::vector<Process> remaining_processes = processes;
        while (!remaining_processes.empty())
        {
            std::vector<Process *> available_processes;
            for (auto &p : remaining_processes)
            {
                if (p.arrival_time <= current_time)
                {
                    available_processes.push_back(&p);
                }
            }

            if (available_processes.empty())
            {
                current_time++;
                continue;
            }

            auto it = std::min_element(available_processes.begin(), available_processes.end(),
                                       [](const Process *a, const Process *b)
                                       { return a->remaining_time < b->remaining_time; });

            Process *process = *it;
            if (process->response_time == -1)
            {
                process->response_time = current_time - process->arrival_time;
            }
            process->remaining_time--;
            current_time++;

            if (process->remaining_time == 0)
            {
                process->completion_time = current_time;
                process->turnaround_time = process->completion_time - process->arrival_time;
                process->waiting_time = process->turnaround_time - process->burst_time;
                completed_processes.push_back(*process);

                remaining_processes.erase(std::remove_if(remaining_processes.begin(), remaining_processes.end(),
                                                         [process](const Process &p)
                                                         { return p.pid == process->pid; }),
                                          remaining_processes.end());
            }
        }
    }

    void round_robin(int time_quantum)
    {
        std::vector<Process> remaining_processes = processes;
        std::queue<Process *> queue;

        while (!remaining_processes.empty() || !queue.empty())
        {
            for (auto it = remaining_processes.begin(); it != remaining_processes.end();)
            {
                if (it->arrival_time <= current_time)
                {
                    queue.push(&(*it));
                    it = remaining_processes.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (queue.empty())
            {
                current_time++;
                continue;
            }

            Process *process = queue.front();
            queue.pop();

            if (process->response_time == -1)
            {
                process->response_time = current_time - process->arrival_time;
            }

            int execution_time = std::min(time_quantum, process->remaining_time);
            process->remaining_time -= execution_time;
            current_time += execution_time;

            if (process->remaining_time == 0)
            {
                process->completion_time = current_time;
                process->turnaround_time = process->completion_time - process->arrival_time;
                process->waiting_time = process->turnaround_time - process->burst_time;
                completed_processes.push_back(*process);
            }
            else
            {
                queue.push(process);
            }

            for (auto it = remaining_processes.begin(); it != remaining_processes.end();)
            {
                if (it->arrival_time <= current_time)
                {
                    queue.push(&(*it));
                    it = remaining_processes.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    void print_results()
    {
        double avg_turnaround_time = 0, avg_waiting_time = 0, avg_response_time = 0;
        int total_burst_time = 0;

        for (const auto &p : completed_processes)
        {
            avg_turnaround_time += p.turnaround_time;
            avg_waiting_time += p.waiting_time;
            avg_response_time += p.response_time;
            total_burst_time += p.burst_time;
        }

        int n = completed_processes.size();
        avg_turnaround_time /= n;
        avg_waiting_time /= n;
        avg_response_time /= n;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Average Turnaround Time: " << avg_turnaround_time << std::endl;
        std::cout << "Average Waiting Time: " << avg_waiting_time << std::endl;
        std::cout << "Average Response Time: " << avg_response_time << std::endl;
        std::cout << "CPU Utilization: " << 100.0 * total_burst_time / current_time << "%" << std::endl;
    }
};

int main()
{
    std::vector<Process> processes = {
        Process(1, 0, 10),
        Process(2, 1, 6),
        Process(3, 3, 2),
        Process(4, 5, 4)};

    std::cout << "First Come First Serve (FCFS)" << std::endl;
    Scheduler fcfs_scheduler(processes);
    fcfs_scheduler.fcfs();
    fcfs_scheduler.print_results();

    std::cout << "\nShortest Job First (SJF)" << std::endl;
    Scheduler sjf_scheduler(processes);
    sjf_scheduler.sjf();
    sjf_scheduler.print_results();

    std::cout << "\nShortest Remaining Time First (SRTF)" << std::endl;
    Scheduler srtf_scheduler(processes);
    srtf_scheduler.srtf();
    srtf_scheduler.print_results();

    std::cout << "\nRound Robin (RR) with Time Quantum = 2" << std::endl;
    Scheduler rr_scheduler(processes);
    rr_scheduler.round_robin(2);
    rr_scheduler.print_results();

    return 0;
}