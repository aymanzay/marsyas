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

#include "BeatAgent.h"
#include <string.h>

using namespace std;
using namespace Marsyas;

#define NONE 0.0
#define BEAT 1.0
#define EVAL 2.0

#define INNER 3.0
#define OUTTER 4.0

BeatAgent::BeatAgent(string name):MarSystem("BeatAgent", name)
{
  addControls();
  beatCount_ = 0;
  score_ = 0;
  curBeatPointValue_ = 0;
  myIndex_ = -1;
  fraction_ = 0.0;
}

BeatAgent::BeatAgent(const BeatAgent& a) : MarSystem(a)
{
  // For any MarControlPtr in a MarSystem 
  // it is necessary to perform this getctrl 
  // in the copy constructor in order for cloning to work 
	ctrl_identity_ = getctrl("mrs_string/identity");
	ctrl_timming_ = getctrl("mrs_natural/timming");
	ctrl_agentControl_ = getctrl("mrs_realvec/agentControl");
	ctrl_scoreFunc_ = getctrl("mrs_string/scoreFunc");
	ctrl_lftOutterMargin_ = getctrl("mrs_real/lftOutterMargin");
	ctrl_rgtOutterMargin_ = getctrl("mrs_real/rgtOutterMargin");
	ctrl_innerMargin_ = getctrl("mrs_real/innerMargin");

	beatCount_ = a.beatCount_;
	lastBeatPoint_ = a.lastBeatPoint_;
	score_ = a.score_;
	curBeatPointValue_ = a.curBeatPointValue_;
	myIndex_ = a.myIndex_;
}

BeatAgent::~BeatAgent()
{
}

MarSystem* 
BeatAgent::clone() const 
{
  return new BeatAgent(*this);
}

void 
BeatAgent::addControls()
{
  //Add specific controls needed by this MarSystem.
	addctrl("mrs_string/identity", "AgentX", ctrl_identity_);
	addctrl("mrs_natural/timming", 0, ctrl_timming_);
	addctrl("mrs_realvec/agentControl", realvec(), ctrl_agentControl_);
	addctrl("mrs_string/scoreFunc", "regular", ctrl_scoreFunc_);
	setctrlState("mrs_string/scoreFunc", true);
	addctrl("mrs_real/lftOutterMargin", 0.2, ctrl_lftOutterMargin_);
	setctrlState("mrs_real/lftOutterMargin", true);
	addctrl("mrs_real/rgtOutterMargin", 0.3, ctrl_rgtOutterMargin_);
	setctrlState("mrs_real/rgtOutterMargin", true);
	addctrl("mrs_real/innerMargin", 3.0, ctrl_innerMargin_);
	setctrlState("mrs_real/innerMargin", true);
}

void
BeatAgent::myUpdate(MarControlPtr sender)
{
	MRSDIAG("BeatAgent.cpp - BeatAgent:myUpdate");
	
	ctrl_onSamples_->setValue(6, NOUPDATE);
	ctrl_onObservations_->setValue(1, NOUPDATE);
	ctrl_osrate_->setValue(ctrl_israte_, NOUPDATE);
		
	//history_.create(1000,2);
	lastBeatPoint_ = inSamples_-1;

	myIndex_ = getChildIndex();
	if(myIndex_ == -1)
		MRSWARN("Agent Index Not Found!");

	scoreFunc_ = ctrl_scoreFunc_->to<mrs_string>();

	lftOutterMargin_ = ctrl_lftOutterMargin_->to<mrs_real>();
	rgtOutterMargin_ = ctrl_rgtOutterMargin_->to<mrs_real>();
	innerMargin_ = ctrl_innerMargin_->to<mrs_real>();
}



mrs_real
BeatAgent::calcDScoreCorrSquare(realvec& in)
{
	mrs_real dScore = 0.0;

	//outterLeft Tolerance:
	for(mrs_natural t = lastBeatPoint_ - outterWinLft_; t < lastBeatPoint_ - innerWin_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / outterWinRgt_;
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / (((outterWinRgt_+outterWinLft_)/2)+0.5);
		//dScore += -1 * pow((fraction_)* in(t),2) ;
		dScore += -1 * pow((fraction_),2) * in(t);
	}

	//innerTolerance:
	for(mrs_natural t = lastBeatPoint_ - innerWin_; t <= lastBeatPoint_ + innerWin_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / (((outterWinRgt_+outterWinLft_)/2)+0.5);
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / (((outterWinRgt_+outterWinLft_)/2)+0.5);
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / outterWinRgt_;
		dScore += pow((1 - fraction_),2) * in(t);
		//dScore += pow((1 - fraction_) * in(t),2);
	}

	//outterRight Tolerance:
	for(mrs_natural t = (lastBeatPoint_ + innerWin_)+1; t <= lastBeatPoint_ + outterWinRgt_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / outterWinLft_;
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / outterWinRgt_;
		//dScore += -1 * pow((fraction_)* in(t),2) ;
		dScore += -1 * pow((fraction_),2) * in(t);
	}

	//if(strcmp(identity_.c_str(), "Agent0") == 0);
	//	cout << "MAX: " << max << "; ERRROR: " << error_ << "; dSCORE: " << dScore << endl;

	//dScore /= (outterWinLft_+outterWinRgt_); //normalized by the full window size
	//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
	//return dScore * sqrt((mrs_real)period_) / (outterWinLft_+outterWinRgt_);
	return dScore;
}

mrs_real
BeatAgent::calcDScoreCorr(realvec& in, mrs_natural maxInd)
{
	mrs_real dScore = 0.0;

	//outterLeft Tolerance:
	for(mrs_natural t = lastBeatPoint_ - outterWinLft_; t < lastBeatPoint_ - innerWin_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / outterWinRgt_;
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / outterWinRgt_;
		dScore += (-1 * fraction_) * in(t);//pow(in(t),2);
	}

	//innerTolerance:
	for(mrs_natural t = lastBeatPoint_ - innerWin_; t <= lastBeatPoint_ + innerWin_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / (((outterWinRgt_+outterWinLft_)/2)+0.5);
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / (((outterWinRgt_+outterWinLft_)/2)+0.5);
		dScore += (1 - fraction_) * in(t);//pow(in(t),2);
	}

	//outterRight Tolerance:
	for(mrs_natural t = (lastBeatPoint_ + innerWin_)+1; t <= lastBeatPoint_ + outterWinRgt_; t++)
	{
		fraction_ = (mrs_real) abs(error_) / outterWinLft_;
		//fraction_ = (mrs_real) abs(lastBeatPoint_-t) / outterWinRgt_;
		dScore += (-1 * fraction_) * in(t);//pow(in(t),2);
	}
	
	//dScore /= (outterWinLft_+outterWinRgt_); //normalized by the full window size
	//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
	//return dScore * sqrt((mrs_real)period_) / (outterWinLft_+outterWinRgt_);
	return dScore;
}

mrs_natural
BeatAgent::getChildIndex()
{
	//check for parent:
	MarSystem* parent = this->getParent();
	myIndex_ = -1;
	if(parent)
	{
		vector<MarSystem*> siblings = parent->getChildren();
		for(mrs_natural i = 0; i < siblings.size(); i++)
		{
			if(this == siblings[i])
			{
				myIndex_ = i;
				break;
			}
		}
	}
	return myIndex_;
}

void
BeatAgent::fillOutput(realvec& out, mrs_real flag, mrs_real period, mrs_real curBeat, 
					  mrs_real tolerance, mrs_real error, mrs_real score)
{
	out(0) = flag;
	out(1) = period;
	out(2) = curBeat;
	out(3) = tolerance;
	out(4) = error;
	out(5) = score;
}

void 
BeatAgent::myProcess(realvec& in, realvec& out)
{
	//Output Format: [Beat/Eval/None|Period|CurBeat|Inner/Outter|Error|Score] -> OnSamples = 6
	
	agentControl_ = ctrl_agentControl_->to<mrs_realvec>();

	//t_ is always updated with the actual referee's timming on the previous tick + 1
	t_ = (mrs_natural) agentControl_(myIndex_, 3) + 1;

	//At first no beat info is considered - while no beat detected:
	fillOutput(out, NONE, 0.0, 0.0, 0.0, 0.0, 0.0);

	identity_ = ctrl_identity_->to<mrs_string>();
	
	isNewOrUpdated_ = (mrs_bool) agentControl_(myIndex_, 0);
	period_ = (mrs_natural) agentControl_(myIndex_, 1);
	phase_ = (mrs_natural) agentControl_(myIndex_, 2);
	
	outterWinLft_ = (mrs_natural) ceil(period_ * lftOutterMargin_); //(% of IBI)
	outterWinRgt_ = (mrs_natural) ceil(period_ * rgtOutterMargin_); //(% of IBI)
	innerWin_ = (mrs_natural) innerMargin_;
	//innerWin_ = (mrs_natural) min(4.0, ceil(period_ * innerMargin_));
	
	mrs_natural curBeatPoint = inSamples_-1;
	mrs_real max = 0;
	mrs_natural max_i = 0;
	
	//If the agent was just created or its hypothesis updated:
	if(isNewOrUpdated_)
		curBeat_ = phase_;
	//If the agent already existed and is keeping its hypothesis:
	else
		curBeat_ = prevBeat_ + period_;

	//Considers beat hypothesis every phase + period
	if(t_ == curBeat_)
	{
		//cout << identity_ << " -> BEAT (" << beatCount_ << ") -> " << t_ << endl;
		
		//Beat Info filling the remaining indexes of output with undef. value
		fillOutput(out, BEAT, -1.0, -1.0, -1.0, -1.0, -1.0);

		curBeatPointValue_ = in(curBeatPoint);

		//history_(beatCount_,0) = t_;
		//history_(beatCount_,1) = curBeatPointValue_;	

		lastBeatPoint_ = curBeatPoint - outterWinRgt_;

		beatCount_++;

		return;
	}

	mrs_natural evalPoint = curBeat_ + outterWinRgt_;
	//Evaluates each beat at the end of its beat position + outterWindow tolerance:
	if(t_ == evalPoint)
	{
		max_i = lastBeatPoint_;

		for(mrs_natural t = lastBeatPoint_ - outterWinLft_; t <= lastBeatPoint_ + outterWinRgt_; t++)
		{
			if(max < in(t))
			{
				max = in(t);
				max_i = t;
			}
		}

		error_ = max_i - lastBeatPoint_;

		//cout << identity_ << " -> MAX -> pred: " << in(lastBeatPoint_) << " ; act: " << max << endl;

		//Classification of last beat based on above evaluation:

		//cout << identity_ << " -> Score: " << score_ << endl;
		if(strcmp(scoreFunc_.c_str(), "squareCorr") == 0)
			score_ = calcDScoreCorrSquare(in); 

		else if(strcmp(scoreFunc_.c_str(), "correlation") == 0)
			score_ = calcDScoreCorr(in, max_i); 

		//mrs_real phase;
		//1st Condition: is beat  inside innerWindow?
		if(max_i >= lastBeatPoint_ - innerWin_ && max_i <= lastBeatPoint_)
		{
			if(strcmp(scoreFunc_.c_str(), "regular") == 0)
			{
				fraction_ = (mrs_real) abs(error_) / outterWinLft_;
				//score_ = (1 - fraction_);
				score_ = (1 - fraction_) * max;

				//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
				//score*= sqrt((mrs_real)period_);
			}
			
			MRSDIAG("BeatAgent::myProcess() - Beat Inside innerWindow!");
			//cout << identity_ << "(" << t_ <<") -> Beat (" << curBeat_ << ") Inside innerWindow! with error: ";

			//Evaluation info:
			//phase = actualBeatPoint = max_i
			fillOutput(out, EVAL, period_, curBeat_, INNER, error_, score_);
		}
		else if(max_i > lastBeatPoint_ && max_i <= lastBeatPoint_ + innerWin_)
		{
			if(strcmp(scoreFunc_.c_str(), "regular") == 0)
			{
				fraction_ = (mrs_real) abs(error_) / outterWinRgt_;
				score_ = (1 - fraction_) * max;
				//score_ = (1 - fraction_);

				//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
				//score*= sqrt((mrs_real)period_);
			}
			
			MRSDIAG("BeatAgent::myProcess() - Beat Inside innerWindow!");
			//cout << identity_ << "(" << t_ <<") -> Beat (" << curBeat_ << ") Inside innerWindow! with error: ";

			//Evaluation info:
			//phase = actualBeatPoint = max_i
			fillOutput(out, EVAL, period_, curBeat_, INNER, error_, score_);
		}

		//2nd Condition: is beat insdie outterWindow but outside innerWindow?
		else 
		{
			if((max_i >= lastBeatPoint_ - outterWinLft_) && (max_i < lastBeatPoint_ - innerWin_))
			{	
				if(strcmp(scoreFunc_.c_str(), "regular") == 0)
				{
					fraction_ = (mrs_real) abs(error_) / outterWinLft_;
					score_ = -1 * fraction_ * max;
					//score_ = (1 - fraction_) * max;

					//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
					//score*= sqrt((mrs_real)period_);
				}
			}
			if((max_i > lastBeatPoint_ + innerWin_) && (max_i <= lastBeatPoint_ + outterWinRgt_))
			{	
				if(strcmp(scoreFunc_.c_str(), "regular") == 0)
				{
					fraction_ = (mrs_real) abs(error_) / outterWinRgt_;
					score_ = -1 * fraction_ * max;
					//score_ = (1 - fraction_) * max;

					//multiplied by sqrt(period) for disinflating the faster agents (with smaller periods) [!]
					//score*= sqrt((mrs_real)period_);
				}
			}

			//Evaluation info:
			//phase = actualBeatPoint = max_i
			//cout << identity_ << "(" << t_ <<") -> Beat (" << curBeat_ << ") Outside innerWindow! with error: ";
			MRSDIAG("BeatAgent::myProcess() - Beat Inside OutterWindow but outside innerWindow!");

			fillOutput(out, EVAL, period_, curBeat_, OUTTER, error_, score_);
		}
			
		//cout << error_ << " (pred: " << curBeat_ << "-" << in(lastBeatPoint_) << " act: " << curBeat_+error_ << 
		//"-" << max << ") -> " << " dS: " << score_ << " NextBeat(ifNotChanged): " << curBeat_+period_ << endl;

		/*
		for(mrs_natural i = 0; i < beatCount_; i++)
		{
			cout << identity_ << " -> "History Phase(" << i << "): " << history_(i, 0) << endl;
			cout << identity_ << " ->  "History Value(" << i << "): " << history_(i, 1) << endl;
		}
		*/

		//Updates previous Beat
		prevBeat_ = curBeat_;

		//Disbales new/updated flag:
		agentControl_(myIndex_, 0) = 0.0;
		updctrl(ctrl_agentControl_, agentControl_);
	}

	
	//MATLAB_PUT(in, "FluxTrack");
	//MATLAB_EVAL("plot(FluxTrack);");
	//MATLAB_EVAL("plot(FluxTrack/max(FluxTrack))");
	//MATLAB_EVAL("hold on;");
	/*MATLAB_PUT(out, "BeatAgent");
	MATLAB_PUT(lastBeatPoint_ + 2, "t");
	MATLAB_PUT(lastBeatPoint_ - outterWinLft_ + 2, "t1");
	MATLAB_PUT(lastBeatPoint_ + outterWinRgt_ + 2, "t2");
	MATLAB_EVAL("BeatAgentTS = [BeatAgentTS, BeatAgent];");
	MATLAB_EVAL("stem(t1,1, 'g');");
	MATLAB_EVAL("stem(t,1, 'r');");
	MATLAB_EVAL("stem(t2,1, 'g');");
	MATLAB_EVAL("hold off;");
	*/
}
