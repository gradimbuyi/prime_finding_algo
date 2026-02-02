#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace std;

const int MAX_N = 100000000;
const int NUM_THREADS = 8;
const int BLOCK_SIZE = 1000000;

vector<int> sieve(int n) {
    vector<char> is_prime(n + 1, true);

    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= n; i++) {
        if (is_prime[i] == true) {
            for (int j = i * i; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }

    vector<int> primes;
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) primes.push_back(i);
    }

    return primes;
}

void segmented_sieve(int start, int end, const vector<int> &primes, vector<int> &results, int &count, long long &sum) {
    vector<char> is_prime(end - start + 1, true);

    for (int prime : primes) {
        if ((long long)prime * prime > end) break;
        long long first = max((long long) prime * prime, ((start + prime - 1)/ prime) * (long long) prime);
        for (long long i = first; i <= end; i += prime) {
            is_prime[i - start] = false;
        }
    }

    for (int i = 0; i < (int) is_prime.size(); i++) {
        int value = start + i;
        if (value >= 2 && is_prime[i]) {
            results.push_back(value);
            count++;
            sum += value;
        }
    }
}

void sequential_run(const vector<int> &small_primes) {
    vector<int> primes;
    int count = 0;
    long long sum = 0;

    auto start_time = chrono::high_resolution_clock::now();
    segmented_sieve(1, MAX_N, small_primes, primes, count, sum);

    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration<double>(end_time - start_time).count();

    cout << "Sequential Run Results: " << "\n";
    cout << "Time: " << (elapsed_time * 1000) << "ms, Count: " << count << " Sum of Primes: " << sum << "\n";
}


int main() {
    int limit = sqrt(MAX_N);

    vector<int> small_primes = sieve(limit);
    vector<thread> threads;
    threads.reserve(NUM_THREADS);
    vector<vector<int>> thread_primes(NUM_THREADS);
    vector<int> thread_counts(NUM_THREADS, 0);
    vector<long long> thread_sum(NUM_THREADS, 0);

    int section = MAX_N / NUM_THREADS;

    sequential_run(small_primes);

    auto start_time = chrono::high_resolution_clock::now();

    for(int i = 0; i < NUM_THREADS; i++) {
        int start = i * section + 1;
        int end   = (i == NUM_THREADS - 1) ? MAX_N : (i + 1) * section;
        threads.emplace_back(segmented_sieve, start, end, cref(small_primes), ref(thread_primes[i]), ref(thread_counts[i]), ref(thread_sum[i]));
    }

    for (auto &thread : threads) {
        thread.join();
    }

    auto end_time = chrono::high_resolution_clock::now();
    double time_elapsed = chrono::duration<double>(end_time - start_time).count();

    int count = 0;
    long long sum = 0;
    vector<int> primes;

    for(int i = 0; i < NUM_THREADS; i++) {
        count += thread_counts[i];
        sum   += thread_sum[i];
        primes.insert(primes.end(), thread_primes[i].begin(), thread_primes[i].end());
    }

    sort(primes.begin(), primes.end());

    ofstream out("primes.txt");
    out << (time_elapsed * 1000) << "ms " << count << " " << sum << "\n";

    for (int i = primes.size() - 10; i < primes.size(); i++) {
        out << primes[i];
        if (i + 1 < primes.size()) out << " ";
    }

    out << "\n";
    out.close();

    return 0;
}