#pragma once

#include <JuceHeader.h>
#include<random>


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

        if (source == &transportSourceTwo)
        {
            if (transportSourceTwo.isPlaying())
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
    void openButtonClickTwo();
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
        transportSourceTwo.releaseResources();
    }

    void changeState(TransportState newState);

    juce::AudioFormatManager formatManager;
    juce::AudioFormatManager formatManagerTwo;

    //Pointer: direccion en memoria
    //unique_ptr: direccion en memoria que se administra sola
    //AudioFormatReaderSource objeto que lee archivo de audio
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSourceTwo;
    
    std::unique_ptr<juce::AudioFormatReaderSource> newSource;
    std::unique_ptr<juce::AudioFormatReaderSource> newSource2;

    //Identifica la ubicacion de reproduccion
    juce::AudioTransportSource transportSource;
    juce::AudioTransportSource transportSourceTwo;



private:

    TransportState state;

    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<juce::FileChooser> chooserTwo;

    juce::MixerAudioSource mixer;

    //Boton para abrir el archivo de audio
    juce::TextButton openButton;
    juce::TextButton openButtonTwo;
    //boton de paly
    juce::TextButton playButton;
    //boton de stop
    juce::TextButton stopButton;

    juce::Slider granSlider;
    juce::Slider startSlider;

    //juce::AudioSourceChannelInfo firstBuffer
    //{
    //    juce::AudioBuffer<float>(2, 1024)
    //};

    //juce::AudioSourceChannelInfo secondBuffer
    //{
    //    juce::AudioBuffer<float>(2, 1024)
    //};

    juce::AudioBuffer<float> firstBuffer;
    juce::AudioBuffer<float> secondBuffer;

    juce::AudioSourceChannelInfo chanInfo1{ firstBuffer };
    juce::AudioSourceChannelInfo chanInfo2{ secondBuffer };

    //MixerAudioSource mixer;

    double totalDuration;
    float startPosition = 0.0;
    float startPositionTwo = 0.0;
    double minGranTime = 0.02;
    double maxGranTime = 0.1;
    double granTime = 0.02;
    double fileDuration = 0.f;
    double fileDurationTwo = 0.f;

    int numChannels = 2;
    int numSamples = 0;

    float tail = 0.5f;


    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
