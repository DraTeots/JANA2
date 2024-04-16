// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <DatamodelGlue.h>
#include <JANA/JEventSource.h>
#include "CollectionTabulators.h"


struct MyTimesliceSource : public JEventSource {

    PodioOutput<ExampleHit> m_hits_out {this, "hits"};

    MyTimesliceSource() {
        SetLevel(JEventLevel::Timeslice);
        SetTypeName("MyTimesliceSource");
    }

    void Open() override { }

    void GetEvent(std::shared_ptr<JEvent> event) override {

        auto ts_nr = event->GetEventNumber();
        auto hits_out  = std::make_unique<ExampleHitCollection>();

        // ExampleHit(unsigned long long cellID, double x, double y, double z, double energy, std::uint64_t time);
        hits_out->push_back(ExampleHit(ts_nr, 0, 22, 22, 22, 0));
        hits_out->push_back(ExampleHit(ts_nr, 0, 49, 49, 49, 1));
        hits_out->push_back(ExampleHit(ts_nr, 0, 7.6, 7.6, 7.6, 2));

        LOG_DEBUG(GetLogger()) << "MyTimesliceSource: Timeslice " << event->GetEventNumber() << "\n"
            << TabulateHits(hits_out.get())
            << LOG_END;

        m_hits_out() = std::move(hits_out);
    }
};