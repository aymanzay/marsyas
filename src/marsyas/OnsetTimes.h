/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.uvic.ca>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef MARSYAS_ONSETTIMES_H
#define MARSYAS_ONSETTIMES_H

#include "MarSystem.h"	

namespace Marsyas
{
/** 
    \class OnsetTimes
	\ingroup Processing Basic
    \brief Retrieves M first beat times (phases, in "frames"-ticks), retrieved from
	peaks from an onset function.

	Input: Supposed to have PeakerOnsets, or similar, as input.
	Output: [x|Phasei|x|Phasei+1|  ...  |x|PhaseM]

	- \b mrs_natural/n1stOnsets [w] : Nr. of first detected onsets (= M phases).
	- \b mrs_natural/nPeriods [w] : Nr. of considered period hypotheses (N) - to avoid Fanout crash (requires equal number of columns in each line)! 
	- \b mrs_natural/lookAheadSamples [w] : Nr. of frames to adjust the lookahead used in PeakerOnset.
*/


class OnsetTimes: public MarSystem
{
private: 
  //Add specific controls needed by this MarSystem.
	void addControls();
	MarControlPtr ctrl_n1stOnsets_;
	MarControlPtr ctrl_lookAheadSamples_;
	MarControlPtr ctrl_nPeriods_;

	mrs_natural nPeriods_;
	mrs_natural lookAhead_;
	mrs_natural acc_;
	mrs_natural n_;
	mrs_natural t_;
	mrs_natural count_;
	void myUpdate(MarControlPtr sender);

public:
  OnsetTimes(std::string name);
	OnsetTimes(const OnsetTimes& a);
  ~OnsetTimes();
  MarSystem* clone() const;  
  
  void myProcess(realvec& in, realvec& out);
};

}//namespace Marsyas

#endif
