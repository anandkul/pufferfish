#ifndef __CANONICAL_KMER_HPP__
#define __CANONICAL_KMER_HPP__

#include "jellyfish/mer_dna.hpp"

/**
 * This class wraps a pair of jellifish k-mers
 * (i.e., mer_dna objects).  It maintains both a
 * k-mer and its reverse complement at all times
 * to make the operation of retreiving the canonical
 * k-mer efficent.
 */
using my_mer = jellyfish::mer_dna_ns::mer_base_static<uint64_t, 1>;
class CanonicalKmer {
private:
  my_mer fw_;
  my_mer rc_;
public:
  CanonicalKmer()=default;
  CanonicalKmer(CanonicalKmer&& other) = default;
  CanonicalKmer(CanonicalKmer& other) = default;
  CanonicalKmer(const CanonicalKmer& other) = default;
  CanonicalKmer& operator=(CanonicalKmer& other) = default;

  static void k(int kIn) { my_mer::k(kIn); }
  static int k() { return my_mer::k(); }

  inline bool fromStr(const std::string& s) {
    auto k = my_mer::k();
    if (s.length() < k) { return false; }
    for (size_t i = 0; i < k; ++i) {
      fw_.shift_right(s[i]);
      rc_.shift_left(my_mer::complement(s[i]));
    }
    return true;
  }

  inline void fromNum(uint64_t w) {
    fw_.word__(0) = w;
    rc_ = fw_.get_reverse_complement();
  }

  inline auto shiftFw(int c) -> decltype(this->fw_.shift_right(c)) {
    rc_.shift_left(my_mer::complement(c));
    return fw_.shift_right(c);
  }

  inline auto shiftBw(int c) -> decltype(this->fw_.shift_left(c)) {
    rc_.shift_right(my_mer::complement(c));
    return fw_.shift_left(c);
  }

  inline auto shiftFw(char c) -> decltype(this->fw_.shift_right(c)){
    int x = my_mer::code(c);
    if(x == -1)
      return 'N';
    rc_.shift_left(my_mer::complement(x));
    return my_mer::rev_code(fw_.shift_right(x));
  }

  inline auto shiftBw(char c) -> decltype(this->fw_.shift_left(c)){
    int x = my_mer::code(c);
    if(x == -1)
      return 'N';
    rc_.shift_right(my_mer::complement(x));
    return my_mer::rev_code(fw_.shift_left(x));
  }

  inline const uint64_t getCanonicalWord() {
    return (fw_.word(0) < rc_.word(0)) ? fw_.word(0) : rc_.word(0);
  }

  inline const my_mer& getCanonical() {
    return (fw_.word(0) < rc_.word(0)) ? fw_ : rc_;
  }

  inline const my_mer& fwMer() const {
    return fw_;
  }

  inline const my_mer& rcMer() const {
    return rc_;
  }

  inline const uint64_t fwWord() {
    return fw_.word(0);
  }

  inline const uint64_t rcWord() {
    return rc_.word(0);
  }

  inline std::string to_str() { return fw_.to_str(); }

  bool operator==(const CanonicalKmer& rhs) const { return this->fw_ == rhs.fw_; }
  bool operator!=(const CanonicalKmer& rhs) const { return !this->operator==(rhs); }
  bool operator<(const CanonicalKmer& rhs) const { return this->fw_ < rhs.fw_; }
  bool operator<=(const CanonicalKmer& rhs) const {return *this < rhs || *this == rhs;}
  bool operator>(const CanonicalKmer& rhs) const {return !(*this <= rhs);}
  bool operator>=(const CanonicalKmer& rhs) const {return !(*this < rhs);}
  bool is_homopolymer() const { return fw_.is_homopolymer(); }
};

#endif // __CANONICAL_KMER_HPP__
