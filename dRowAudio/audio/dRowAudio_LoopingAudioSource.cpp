/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

BEGIN_JUCE_NAMESPACE

//==============================================================================
LoopingAudioSource::LoopingAudioSource (PositionableAudioSource* const inputSource,
										const bool deleteInputWhenDeleted)
    : input (inputSource, deleteInputWhenDeleted),
      isLoopingBetweenTimes (false),
      loopStartSample (0),
      loopEndSample (0),
      tempBuffer (2, 512)
{
    jassert (inputSource != 0);
    
    tempInfo.numSamples = tempBuffer.getNumSamples();
    tempInfo.startSample = 0;
    tempInfo.buffer = &tempBuffer;
}

LoopingAudioSource::~LoopingAudioSource()
{
}

//==============================================================================
void LoopingAudioSource::setLoopTimes (double startTime, double endTime)
{
    jassert (endTime > startTime); // end time has to be after start!
    
    {
        const ScopedLock sl (loopPosLock);
        
        loopStartTime = startTime;
        loopEndTime = endTime;
        
        loopStartSample = startTime * currentSampleRate;
        loopEndSample = endTime * currentSampleRate;
    }

    // need to update read position based on new limits
    setNextReadPosition (getNextReadPosition());
}

void LoopingAudioSource::setLoopBetweenTimes (bool shouldLoop)
{
    isLoopingBetweenTimes = shouldLoop;
}

bool LoopingAudioSource::getLoopBetweenTimes()
{
    return isLoopingBetweenTimes;
}

//==============================================================================
void LoopingAudioSource::prepareToPlay (int samplesPerBlockExpected,
                                        double sampleRate)
{
    currentSampleRate = sampleRate;
	input->prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    if (tempBuffer.getNumSamples() < samplesPerBlockExpected)
    {
        tempBuffer.setSize (2, samplesPerBlockExpected);
        tempInfo.numSamples = tempBuffer.getNumSamples();
    }
}

void LoopingAudioSource::releaseResources()
{
	input->releaseResources();
}

void LoopingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples > 0)
    {
        if (isLoopingBetweenTimes)
        {
            const ScopedLock sl (loopPosLock);

            const int64 newStart = getNextReadPosition();
            const int64 newEnd = loopStartSample + ((newStart + info.numSamples) % loopEndSample);
            
            if (newEnd > newStart)
            {
                input->getNextAudioBlock (info);
            }
            else
            {
                const int64 numEndSamps = loopEndSample - newStart;
                const int64 numStartSamps = newEnd - loopStartSample;
                
                tempInfo.startSample = 0;
                tempInfo.numSamples = numEndSamps;
                input->getNextAudioBlock (tempInfo);
                
                tempInfo.startSample = numEndSamps;
                tempInfo.numSamples = numStartSamps;
                input->setNextReadPosition (loopStartSample);
                input->getNextAudioBlock (tempInfo);

                tempInfo.numSamples = info.numSamples;
                
                for (int i = 0; i < info.buffer->getNumChannels(); ++i)
                    info.buffer->copyFrom (i, info.startSample, 
                                           tempBuffer,
                                           i, 0, tempInfo.numSamples);
            }
        }
        else
        {
            input->getNextAudioBlock (info);
        }
    }    
}

//==============================================================================
void LoopingAudioSource::setNextReadPosition (int64 newPosition)
{
    const ScopedLock sl (loopPosLock);
    
    if (isLoopingBetweenTimes 
        && getNextReadPosition() > loopStartSample
        && getNextReadPosition() < loopEndSample)
    {
        const int64 numLoopSamples = loopEndSample - loopStartSample;
        
        if (newPosition > loopEndSample)
            newPosition = loopStartSample + ((newPosition - loopEndSample) % numLoopSamples);
        else if (newPosition < loopStartSample)
            newPosition = loopEndSample - ((loopStartSample - newPosition) % numLoopSamples);
    }
    
    input->setNextReadPosition (newPosition);
}

int64 LoopingAudioSource::getNextReadPosition() const
{
    return input->getNextReadPosition();
}

int64 LoopingAudioSource::getTotalLength() const
{
    return input->getTotalLength();
}

bool LoopingAudioSource::isLooping() const
{
    return input->isLooping();
}

END_JUCE_NAMESPACE