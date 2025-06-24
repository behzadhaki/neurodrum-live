/*
  ==============================================================================

    InferenceThreadJob.h
    Created: 2 Oct 2021 1:54:57pm
    Author:  Andrew Fyfe

  ==============================================================================
*/

#pragma once

#pragma once
#include "shared_plugin_helpers/shared_plugin_helpers.h"
#include "PluginProcessor.h"

using namespace juce;

class NewPluginTemplateAudioProcessor; // forward declare

class InferenceThreadJob : public ThreadPoolJob
{
public:
    
    explicit InferenceThreadJob(NewPluginTemplateAudioProcessor& gluProcessor);
    virtual ~InferenceThreadJob();
    virtual auto runJob() -> JobStatus;
    
private:
    NewPluginTemplateAudioProcessor& mProcessor;
    
    // Prevent uncontrolled usage
    InferenceThreadJob(const InferenceThreadJob&);
    InferenceThreadJob& operator=(const InferenceThreadJob&);
};
