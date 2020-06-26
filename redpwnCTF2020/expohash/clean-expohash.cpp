/*
redpwnCTF 2020 Writeup
Challenge: expohash
Category: Misc
Points: 491

This was the only algo problem of the CTF.
There's an array A with 10^5 numbers, and we are given xor sums of 10^5 intervals in A.
We need to find any values of A that work for the list of intervals.

(Solution is next paragraph)
I solved this with a few hours left in the CTF.
I tried a few approaches, including sorting by interval size and then trying to backtrack once I had multiple options.
That took a ridiculously long time, and didn't work.
I also tried searching for the solution online.
This problem seems standard enough that someone must have solved it.
But I couldn't find anything, although that might just be my bad searching skills.
I finally thought of the solution after contemplating the problem for a few days (and trying to implement some rather hopeless ideas).
Now to the solution...

We can simplify each interval so that no two intervals end in the same spot.
If any two intervals do end in the same spot, we can change the larger one to end right before the smaller one.
Formally, if we have L1 < L2 and R1 = R2, then we can change R1 = L2 - 1 and V1 to V1 xor V2.
In this way, we can ensure every interval has a unique endpoint.

Then, we can go from left to right and fill in the values of the password.
Since at most one interval ends at any spot, we can use that spot to make sure the interval has the correct value.
The code to do so is below. It implements a segment tree (which is really overkill), but prefix sums should work fine too.
The segment tree is left over from a different failed approach from earlier.

To solve the challenge from the nc server, I plugged in this code to a netcat wrapper I wrote in python.
 */


/*
 * expohash.cpp
 *
 *  Created on: Jun 22, 2020
 *      Author: Stanley
 */

#include <fstream> // use files for debugging since it takes a long time to interact with server
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

using namespace std;

typedef long long ll;
typedef unsigned long long ull;

struct Interval {
	int left, right;
	ll val;

	void print() {
		cout << left << ' ' << right << ' ' << val << ' ' << endl;
	}
};
bool comp_right(const Interval &a, const Interval &b) {
	if (a.right == b.right) return a.left < b.left;
	return a.right < b.right;
}

const int N = 1e5;
const int M = 1e5;

// Segtree for xor range sums
int segtree[N * 2];
void update(int pos, int val) {
	pos += N;
	segtree[pos] = val;

	while (pos > 1) {
		pos >>= 1;
		segtree[pos] = segtree[2 * pos] ^ segtree[2 * pos + 1];
	}
}

int query(int left, int right) {
	left += N;
	right += N + 1;

	int ret = 0;
	while (left < right) {
		if (left & 1) {
			ret ^= segtree[left++];
		}
		if (right & 1) {
			ret ^= segtree[--right];
		}
		left >>= 1;
		right >>= 1;
	}

	return ret;
}

int pass[N];
int interval_end[N];
const int MX = 1001001001;

bool DEBUG = false;

int main() {
	if (DEBUG) {
		freopen(string("example_input.txt").c_str(), "r", stdin); // for debugging
	}

	vector<Interval> vals(M);
	for (int i = 0; i < M; i++) {
		cin >> vals[i].left >> vals[i].right >> vals[i].val;
		vals[i].left--;
		vals[i].right--;
		assert(vals[i].val <= (1LL << 31));
		// make sure value is less than 2^31, fortunately it never is
		// that would be annoying since the values of the password must be <= 2^31
	}

	// make sure endpoints are all unique
	bool change = false;
	int ct = 0;
	do {
		change = false;
		if (DEBUG && (++ct & 255) == 0) {
			cout << ct << ' ' << vals.size() << '\n';
		}

		// we only care about endpoints
		sort(vals.begin(), vals.end(), comp_right);
		while (vals.back().right == MX) vals.pop_back(); // remove duplicates
		for (int i=0; i+1<vals.size(); i++) {
			if (vals[i].right == vals[i+1].right) {
				// if endpoints are same, change larger interval to end just before the smaller one
				if (vals[i].left < vals[i+1].left) {
					vals[i].right = vals[i+1].left - 1;
					vals[i].val ^= vals[i+1].val;
				}
				else if (vals[i+1].left > vals[i].left) {
					vals[i+1].right = vals[i].left - 1;
					vals[i+1].val ^= vals[i].val;
				}
				else {
					// this will create duplicates, which should be removed
					vals[i].left = MX;
					vals[i].right = MX;
					assert(vals[i].val == vals[i+1].val);
				}
				change = true;
			}
		}
	}
	while (change);

	// mark where each interval ends
	fill(interval_end, interval_end+N, -1);
	for (int i=0; i<vals.size(); i++) {
		// no interval should end at the same spot, otherwise the first part failed
		assert(interval_end[vals[i].right] == -1);
		interval_end[vals[i].right] = i;
	}

	// set the password
	for (int i=0; i<N; i++) {
		if (interval_end[i] == -1) {
			pass[i] = 0;
		}
		else {
    		int cur_val = query(vals[interval_end[i]].left, vals[interval_end[i]].right);
			pass[i] = vals[interval_end[i]].val ^ cur_val;
		}
		update(i, pass[i]);
	}


	for (int i=0; i<N; i++) {
		cout << pass[i] << '\n';
	}
}
