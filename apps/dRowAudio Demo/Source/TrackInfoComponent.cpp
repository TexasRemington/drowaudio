/*
  ==============================================================================

    TrackInfoComponent.cpp
    Created: 8 Oct 2011 4:26:24pm
    Author:  David Rowland

  ==============================================================================
*/

#include "TrackInfoComponent.h"

TrackInfoComponent::TrackInfoComponent (AudioFilePlayer& audioFilePlayer_)
    : audioFilePlayer (audioFilePlayer_)
{
    audioFilePlayer.addListener (this);
}

TrackInfoComponent::~TrackInfoComponent()
{
}

void TrackInfoComponent::resized()
{
//    const int w = getWidth();
//    const int h = getHeight();
}

void TrackInfoComponent::paint (Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    g.setColour (Colours::white);
    g.setFont (20);
    
    ValueTree trackInfo (audioFilePlayer.getLibraryEntry());
    
    if (trackInfo.isValid())
    {
        String infoText;
        infoText << trackInfo[Columns::columnNames[Columns::Artist]].toString() << "\n";
        infoText << trackInfo[Columns::columnNames[Columns::Song]].toString();
        
        g.drawFittedText (infoText,
                          0, 0, w, h,
                          Justification (Justification::topLeft),
                          2);        
    }
    else
    {
        String displayText;
        if (audioFilePlayer.getTotalLength() > 0)
            displayText = "No Track Info Available";
        else
            displayText = "Drop Tracks Here to Begin...";
        
        g.drawFittedText (displayText,
                          0, 0, w, h,
                          Justification (Justification::centred),
                          2);
    }
}

void TrackInfoComponent::fileChanged (AudioFilePlayer* player)
{
    if (player == &audioFilePlayer)
    {
        repaint();
    }
}