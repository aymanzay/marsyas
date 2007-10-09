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

#ifndef MARSYAS_ENHADRESSSTEREOSPECTRUM_H
#define MARSYAS_ENHADRESSSTEREOSPECTRUM_H

#include "MarSystem.h"

namespace Marsyas
{
	/**
	\class enhADRessStereoSpectrum
	\ingroup Analysis
	\brief Takes the output of the enhADRess
	and outputs the panning coefficient for each spectral bin (N/2+1 bins). 
	*/


class enhADRessStereoSpectrum: public MarSystem
{
private:
	mrs_natural N2_;

	void addControls();
	void myUpdate(MarControlPtr sender);

public:
	enhADRessStereoSpectrum(std::string name);
	enhADRessStereoSpectrum(const enhADRessStereoSpectrum& a);
	~enhADRessStereoSpectrum();
	MarSystem* clone() const;

	void myProcess(realvec& in, realvec& out);
};

}

#endif

