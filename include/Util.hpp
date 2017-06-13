#ifndef _UTIL__H
#define _UTIL__H


#include <type_traits>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include "string_view.hpp"
#include "core/range.hpp"

#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"

namespace util {
using namespace std;


struct cmpByPair {
						    bool operator() (std::pair<std::string, bool> a, std::pair<std::string, bool> b) const {
											return (a.first != b.first) ? (a.first < b.first) : (a.second < b.second);

						    }
				};

// We need a wraper that will provide a "type" field
template<typename> struct Void { typedef void type; };

// The "default" type T does not have a key
template<typename T, typename Sfinae = void>
struct has_key: std::false_type {};

// Use decltype to access the key_type field of the 
// provided type T if it exists.  If T has a key_type trait
// then this will succeed, and we can instantiate this struct.
// If T doesn't have a key_type trait, then SFINAE means that that
// struct won't be instantiated and we'll fall back to the default
// above.
template<typename T>
struct has_key<
    T
    , typename Void<
        decltype( typename T::key_type{} )
    >::type
>: std::true_type {};


/*
Print a map-like container
 */
template <typename T, bool b>
struct ContainerPrinter{
	static string str(T& container) {
		ostringstream s;
 	    /* 
  	     Loop over and print out the map using the new c++11
   	     style for loop.  The new-style loop will work for any 
	     type that has begin() and end() methods that return an iterator,
	     OR if there are _free_ functions begin() and end() which
	     consume the type and return an iterator.
   	    */
		s << "[";
		for (auto& e : container) {
			s << e.first << " : " << e.second << ", ";
		}
		s.seekp(-2, ios_base::end);
		s << "]";
    	return s.str();
    }
};

/*
Print a list-like container
 */
template <typename T>
struct ContainerPrinter <T, false> {
	static string str(T& container) {
		ostringstream s;
		s << "[";
		for (auto& e : container) {
			s << e << ", ";
		}
		s.seekp(-2, ios_base::end);
		s << "]";
	    return s.str();
    }
};

/*
Compile-time selection between list-like and map-like printing.
 */
template <typename T>
string str(T& container) {
	return ContainerPrinter<T, has_key<T>::value>::str(container);
}

  class IndexOptions {
  public:
    uint32_t k{31};
    std::string gfa_file;
    std::string cfile;
    std::string rfile;
    std::string outdir;
  };

  class TestOptions {
  public:
  };

  class ValidateOptions {
  public:
    std::string indexDir;
    std::string refFile;
  };

  // For the time being, assume < 4B contigs
  // and that each contig is < 4B bases
  struct Position {
    //std::string transcript_id;
    uint32_t transcript_id_;
    uint32_t pos_;
    //bool orien;
    Position() {
      transcript_id_ = std::numeric_limits<decltype(transcript_id_)>::max();
      pos_ = std::numeric_limits<decltype(pos_)>::max();
    }

    Position(uint32_t tid, uint32_t tpos, bool torien) {
      transcript_id_ = tid;
      pos_ = tpos;
      setOrientation(torien);
      //orien = torien;
    }

    void setOrientation(bool orientation) {
      if (orientation) {
        pos_ |= 1 << 31;
      } else {
        pos_ &= 0x7FFFFFFF;
      }
    }

    inline uint32_t transcript_id() { return transcript_id_; }
    inline uint32_t pos() { return (pos_ & 0x7FFFFFFF); }
    inline bool orientation() { return (pos_ & 0x80000000) > 0; }

    template <class Archive>
    void serialize(Archive& ar) {
      ar(transcript_id_, pos_);
    }

  private:
    //uint32_t orientMask_
  };

  struct RefPos {
    uint32_t pos;
    bool isFW;
  };
    
  // Structure to hold a list of "projected" (i.e. reference) hits
  // for a k-mer
  struct ProjectedHits {
    // The relative position of the k-mer inducing this hit on the
    // contig
    uint32_t contigPos_;
    // How the k-mer inducing this hit maps to the contig
    // true for fw, false for rc
    bool contigOrientation_; 
    uint32_t contigLen_;
    uint32_t k_;
    core::range<std::vector<util::Position>::iterator> refRange;

    inline bool empty() { return refRange.empty(); }

    RefPos decodeHit(util::Position& p) {
      // true if the contig is fowrard on the reference
      bool contigFW = p.orientation();
      // we are forward with respect to the reference if :
      // (1) contigFW and contigOrientation_
      // (2) !contigFW and !contigOrientation_
      // we are reverse complement with respect to the reference if :
      // (3) configFW and !contigOrientation_
      // (4) !configFW and contigOrientation_

      // if we're in the forward orientation, then our position is
      // just the contig offset plus or relative position
      uint32_t rpos{0};
      bool rfw{false};
      if (contigFW and contigOrientation_) {
        // kmer   :          AGC
        // contig :      ACTTAGC
        // ref    :  GCA[ACTTAGC]CA
        rpos = p.pos() + contigPos_;
        rfw = true;
      } else if (contigFW and !contigOrientation_) {
        // kmer   :          GCT
        // contig :      ACTTAGC
        // ref    :  GCA[ACTTAGC]CA
        rpos = p.pos() + contigPos_;
        rfw = false;
      } else if (!contigFW and contigOrientation_) {
        // kmer   :          AGT
        // contig :      GCTAAGT
        // ref    :  GCA[ACTTAGC]CA
        rpos = p.pos() + contigLen_ - (contigPos_ + k_);
        rfw = false;
      } else if (!contigFW and !contigOrientation_) {
        // kmer   :          ACT
        // contig :      GCTAAGT
        // ref    :  GCA[ACTTAGC]CA
        rpos = p.pos() + contigLen_ - (contigPos_ + k_);
        rfw = true;
      }
      return {rpos, rfw};
    }
  };

  char complement(char& c);
  std::string revcomp(std::string s);
  bool isRevcomp(std::string s);
  std::vector<std::pair<std::string, bool> > explode(const stx::string_view str, const char& ch);
	bool is_number(const std::string& s);
  // Avoiding un-necessary stream creation + replacing strings with string view
  // is a bit > than a 2x win!
  // implementation from : https://marcoarena.wordpress.com/tag/string_view/
  std::vector<stx::string_view> split(stx::string_view str, char delims);
}

#endif // _UTIL__H
