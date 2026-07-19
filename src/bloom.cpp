/*
This is a counting bloom filter implementation.
https://en.wikipedia.org/wiki/Bloom_filter#Counting_Bloom_filters

we also use the Kirsch-Mitzenmacher optimization
https://www.eecs.harvard.edu/~michaelm/postscripts/tr-02-05.pdf

This is the hash used:
https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
*/

#include <array>
#include <cassert>
#include <vector>

// the size in bits of the bloom filter
constexpr int M = 64;
// number of index values to use
constexpr int K = 4;

// bloom filter
std::vector<uint8_t> bits(M, 0);

// NOTE what tests has this hash passed and why is
// it a good fit for this use case?
inline uint64_t splitmix64_hash(uint64_t x) {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

// g_i(x) = h_1(x) + i * h_2(x) mod m
// legacy
// uint64_t kirsch_mitz_hash(uint64_t x, uint64_t i){
//   uint64_t hash = splitmix64_hash(x);
//   uint64_t hash_one = hash & 0xFFFFFFFF;
//   uint64_t hash_two = hash >> 32;
//   uint64_t result = hash_one + (i * hash_two);
//   return result;
// }

// uses the kirsch mitz optimization
// g_i(x) = h_1(x) + i * h_2(x) mod m
std::array<uint64_t, K> generate_indices(int key) {
  std::array<uint64_t, K> indices;

  // generate the hash
  uint64_t hash = splitmix64_hash(key);
  uint32_t hash_one = hash & 0xFFFFFFFF;
  uint32_t hash_two = hash >> 32;

  for (size_t i = 0; i < K; i++) {
    indices[i] = ((hash_one + i * hash_two) % M);
  }

  return indices;
}

void insert(int addr) {
  auto indices = generate_indices(addr);
  for (size_t i = 0; i < indices.size(); i++) {
    uint64_t index = indices[i];
    bits[index] += 1;
  }
}

// requires check_existance(addr) == true
void remove_exis(int addr) {
  auto indices = generate_indices(addr);
  for (size_t i = 0; i < indices.size(); i++) {
    uint64_t index = indices[i];
    bits[index] -= 1;
  }
}

bool check_existance(int addr) {
  auto indices = generate_indices(addr);
  for (int i = 0; i < indices.size(); i++) {
    int index = indices[i];
    if (bits[index] == 0) {
      return false;
    }
  }
  return true;
}
