#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, juce::ChangeListener, juce::Slider::Listener

{
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };


public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //Override: hacemos que el metodo existente pero queremos que haga lo que nosotros queramos
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        //Asegurarce que recibimos el tipo de archivo correcto
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
            {
                changeState(Playing);
            }
            else
            {
                changeState(Stopped);
            }
        }
    }

    void openButtonClick();
    void playButtonClick();
    void stopButtonClick();

    void granKnob();

   
    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &granSlider)
        {
            granTime = granSlider.getValue();
        }
        if (slider == &startSlider)
        {
            startPosition = startSlider.getValue();
        }
    }
    
   
    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void changeState(TransportState newState);

    juce::AudioFormatManager formatManager;

    //Pointer: direccion en memoria
    //unique_ptr: direccion en memoria que se administra sola
    //AudioFormatReaderSource objeto que lee archivo de audio
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    //Identifica la ubicacion de reproduccion
    juce::AudioTransportSource transportSource;



private:

    TransportState state;

    std::unique_ptr<juce::FileChooser> chooser;

    //Boton para abrir el archivo de audio
    juce::TextButton openButton;
    //boton de paly
    juce::TextButton playButton;
    //boton de stop
    juce::TextButton stopButton;

    juce::Slider granSlider;
    juce::Slider startSlider;

    double totalDuration;
    float startPosition = 40.0;
    double minGranTime = 0.02;
    double maxGranTime = 3.0;
    double granTime = 0.02;
    double fileDuration = 0.f;


    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
