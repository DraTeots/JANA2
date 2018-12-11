//
//    File: JTestMain.cc
// Created: Fri Dec  7 08:42:59 EST 2018
// Creator: davidl (on Linux jana2.jlab.org 3.10.0-957.el7.x86_64 x86_64)
//
// ------ Last repository commit info -----
// [ Date ]
// [ Author ]
// [ Source ]
// [ Revision ]
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Jefferson Science Associates LLC Copyright Notice:  
// Copyright 251 2014 Jefferson Science Associates LLC All Rights Reserved. Redistribution
// and use in source and binary forms, with or without modification, are permitted as a
// licensed user provided that the following conditions are met:  
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer. 
// 2. Redistributions in binary form must reproduce the above copyright notice, this
//    list of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.  
// 3. The name of the author may not be used to endorse or promote products derived
//    from this software without specific prior written permission.  
// This material resulted from work developed under a United States Government Contract.
// The Government retains a paid-up, nonexclusive, irrevocable worldwide license in such
// copyrighted data to reproduce, distribute copies to the public, prepare derivative works,
// perform publicly and display publicly and to permit others to do so.   
// THIS SOFTWARE IS PROVIDED BY JEFFERSON SCIENCE ASSOCIATES LLC "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
// JEFFERSON SCIENCE ASSOCIATES, LLC OR THE U.S. GOVERNMENT BE LIABLE TO LICENSEE OR ANY
// THIRD PARTES FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Description
//
// This is used as an overall orchestrator for the various testing modes of the JTest
// plugin. This allows it to do more advanced testing by setting the JTEST:MODE and
// other config. parameters. This could have all been embedded in the JEventProcessor
// class, but I decided to put it here to make things a little cleaner.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <thread>
#include <chrono>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <JApplication.h>
#include <JEventSourceGeneratorT.h>

#include "JTestMain.h"
#include "JEventSource_jana_test.h"
#include "JEventProcessor_jana_test.h"
#include "JFactoryGenerator_jana_test.h"

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventSourceGeneratorT<JEventSource_jana_test>());
	app->Add(new JFactoryGenerator_jana_test());
	app->Add(new JEventProcessor_jana_test());
	app->GetJEventSourceManager()->AddEventSource("dummy");

	new JTestMain(app);
}
} // "C"

//---------------------------------
// JTestMain    (Constructor)
//---------------------------------
JTestMain::JTestMain(JApplication *app)
{
	mApp = app;


	uint32_t kMinThreads=0;
	uint32_t kMaxThreads=0;
	string kThreadSet;

	gPARMS->SetDefaultParameter("JTEST:MODE", mMode, "JTest plugin Testing mode. 0=basic, 1=scaling");
	gPARMS->SetDefaultParameter("JTEST:NSAMPLES", mNsamples, "JTest plugin number of samples to take for each test");
	gPARMS->SetDefaultParameter("JTEST:MINTHREADS", kMinThreads, "JTest plugin minimum number of threads to test");
	gPARMS->SetDefaultParameter("JTEST:MAXTHREADS", kMaxThreads, "JTest plugin maximum number of threads to test");

	// insert continuous range of NThreads to test
	if( kMinThreads>0 && kMaxThreads>0){
		for(uint32_t nthreads=kMinThreads; nthreads<=kMaxThreads; nthreads++) mThreadSet.insert(nthreads);
	}

	switch( mMode ){
		case MODE_BASIC:
			break;
		case MODE_SCALING:
			gPARMS->SetParameter("NEVENTS", 0);
			mThread = new std::thread(&JTestMain::TestThread, this);
			break;
	}
}

//---------------------------------
// ~JTestMain    (Destructor)
//---------------------------------
JTestMain::~JTestMain()
{
	// Clean up thread if it was launched
	if(mThread != nullptr){
		mQuit = true;
		mThread->join();
		delete mThread;
		mThread = nullptr;
	}
}

//---------------------------------
// TestThread
//
// This method will be called as a separate thread to orchestrate
// the test depending on the testing mode.
//---------------------------------
void JTestMain::TestThread(void)
{
	//- - - - - - - - - - - - - - - - - - - - - - - -
	// Scaling test

	auto tm = mApp->GetJThreadManager();

	// Turn ticker off so we can better control the screen
	mApp->SetTicker( false );

	// Wait for events to start flowing indicating the source is primed
	for(int i=0; i<60; i++){
		cout << "Waiting for event source to start producing ... rate: " << mApp->GetInstantaneousRate() << endl;
		std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
		auto rate = mApp->GetInstantaneousRate();
		if( rate > 10.0 ) {
			cout << "Rate: " << rate << "Hz   -  ready to begin test" <<endl;
			break;
		}
	}

	// Loop over all thread settings in set
	cout << "Testing " << mThreadSet.size() << " Nthread settings with " << mNsamples << " samples each" << endl;
	map< uint32_t, vector<float> > rates;
	for( auto nthreads : mThreadSet ){
		cout << "Setting NTHREADS = " << nthreads << " ..." <<endl;
		tm->SetNJThreads( nthreads );

		// Loop for at most 60 seconds waiting for the number of threads to update
		for(int i=0; i<60; i++){
			std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
			if( tm->GetNJThreads() == nthreads ) break;
		}

		// Acquire mNsamples instantaneous rate measurements. The
		// GetInstantaneousRate method will only update every 0.5
		// seconds so we just wait for 1 second between samples to
		// ensure independent measurements.
		double sum  = 0;
		double sum2 = 0;
		for(uint32_t isample=0; isample<mNsamples; isample++){
			std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
			auto rate = mApp->GetInstantaneousRate();
			rates[nthreads].push_back(rate);

			sum  += rate;
			sum2 += rate*rate;
			double N = (double)(isample+1);
			double avg = sum/N;
			double rms = sqrt( (sum2 + N*avg*avg - 2.0*avg*sum)/N );

			cout << "nthreads=" << tm->GetNJThreads() << "  rate=" << rate << "Hz";
			if( N>1 ) cout << "  (avg = " << avg << " +/- " << rms/sqrt(N) << " Hz)";
			cout << endl;
		}
	}

	cout << "Testing finished" << endl;
	mApp->Quit();
}

