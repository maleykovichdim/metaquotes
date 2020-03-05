#pragma once
//class for matching string to simple pattern (?=any one smbl, * any symbols, letters)


#include <iostream>
#include <string.h>
#include <string>

//#include <vector>
#include "Vect.h"

namespace clog_reader {


	//base struct for checking
	struct Consilience {

		Consilience() :m_next(0) {}

		//main overloaded function for checking
		virtual bool consilience(const char * pattern, const char * input) const
		{
			return !std::strcmp(pattern, input);
		}

		//function for next checking in chung
		bool next(const char * pattern, const char * input) const
		{
			if (!m_next) return false;
			return m_next->consilience(pattern, input);
		}

		const Consilience * m_next; //  link for next check
	};



	//container for consilence
	class ConsilienceVectCon : public Consilience {
		typedef Vect<Consilience *> VectCon;
		VectCon ranges;
	public:

		//main overloaded function for checking, go through ranges
		virtual bool consilience(const char * pattern, const char * input) const {
			for (auto i = ranges.begin(); i != ranges.end(); ++i) {
				if ((*i)->consilience(pattern, input)) return true;
			}
			return false;
		}

		// adding pointer for ranges into vector
		void add(Consilience * m) {
			ranges.push_back(m);
			m->m_next = this;//current Cons will be next for this
		}

		~ConsilienceVectCon() {
			for (auto i = ranges.begin(); i != ranges.end(); ++i)
			{
				if ((*i) && ((*i)->m_next == this)) (*i)->m_next = 0;
			}
		}

	};


	// ? case of consilience -> we only one letter or sign
	struct ConsilienceQuestion : public Consilience {
		virtual bool consilience(const char * pattern, const char * input) const {
			if (pattern[0] != '?')
				return false;
			//if (next(pattern + 1, input))// uncomment for rule 0 or 1 sign
			//	return true;
			if (next(pattern + 1, input + 1))
				return true;
			return false;
		}
	};

	//empty consilience
	struct ConsilienceEmpty : public Consilience {
		virtual bool consilience(const char * pattern, const char * input) const {
			if (pattern[0] == 0 && (input[0] == 0 || input[0] == 0x0A || input[0] == 0x0D))//if (pattern[0] == 0 && input[0] == 0
				return true;
			return false;
		}
	};


	//consilience for asterisk *
	struct ConsilienceAsterisk : public Consilience {
		virtual bool consilience(const char * pattern, const char * input) const {
			if (pattern[0] != '*')
				return false;
			if (pattern[1] == 0) {//end of searching
				return true;
			}
			for (int i = 0; input[i] != 0; ++i) {
				if (next(pattern + 1, input + i))//next
					return true;
			}
			return false;
		}
	};

	struct ConsilienceSymbol : public Consilience {
		virtual bool consilience(const char * pattern, const char * input) const {
			if (pattern[0] != input[0])
				return false;
			return next(pattern + 1, input + 1);
		}
	};

	class DefaultConsilience : public ConsilienceVectCon {
		ConsilienceEmpty    empty;
		ConsilienceQuestion question;
		ConsilienceAsterisk asterisk;
		ConsilienceSymbol   symbol;
	public:
		DefaultConsilience() {
			add(&empty);
			add(&question);
			add(&asterisk);
			add(&symbol);
		}
	};
}






