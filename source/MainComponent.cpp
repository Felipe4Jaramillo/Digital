#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : state(Stopped)
{
    //Metodo de juce para hacer visible un componenete
    addAndMakeVisible(&openButton); //el & indica a un objeto o variable especifica
    //Texto del boton
    openButton.setButtonText("open...");

    //Funcion lamda
    //PAsarle una funcion a otra funcion
    //Sintaxis
    //[Donde esta la funcion] {la funcion]
    openButton.onClick = [this] {openButtonClick(); };

    //Metodo de juce para hacer visible un componenete
    addAndMakeVisible(&playButton); //el & indica a un objeto o variable especifica
 
    playButton.setButtonText("Play");

    playButton.onClick = [this] {playButtonClick(); };

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);

    playButton.setEnabled(false);

    //Metodo de juce para hacer visible un componenete
    addAndMakeVisible(&stopButton); //el & indica a un objeto o variable especifica

    stopButton.setButtonText("Stop");

    stopButton.onClick = [this] {stopButtonClick(); };

    stopButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);


    stopButton.setEnabled(false);

    addAndMakeVisible(&granSlider);

    //granSlider.onValueChange = [this] {granKnob(); };

    granSlider.setRange(minGranTime, maxGranTime);

    granSlider.addListener(this);

    granSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    granSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::black);
    granSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::white);

    granSlider.isHorizontal();

    granSlider.setEnabled(false);


     addAndMakeVisible(&startSlider);

    //granSlider.onValueChange = [this] {granKnob(); };


    startSlider.addListener(this);

    startSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    startSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::black);
    startSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::white);

    
    startSlider.isVertical();

    startSlider.setEnabled(false);

    //Que formatos vamos a reproducir
    formatManager.registerBasicFormats();

    transportSource.addChangeListener(this); 




    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void MainComponent::changeState(TransportState newState) 
{
    //Revisa que no repitamos el mismo estado

    if (state != newState) 
    {
        state = newState;

        switch (state)
        {
        case Stopped:
            stopButton.setEnabled(false);
            playButton.setEnabled(true);
            startSlider.setEnabled(true);
            startPosition = (float)rand() / RAND_MAX * fileDuration - tail;

            //Este setPosition determina donde queda luego de haber parado
            //ransportSource.setPosition(startPosition);
            break;

        case Starting:         
            playButton.setEnabled(false);
            granSlider.setEnabled(true);
            startSlider.setEnabled(false);

            transportSource.start();
            break;

        case Playing:
            transportSource.setPosition(startPosition);
            stopButton.setEnabled(true);
            break;

        case Stopping:
            transportSource.stop();
            granSlider.setEnabled(false);
            break;
        }
    }
}

void MainComponent::openButtonClick()
{
    //Abrir ventana para seleccionar archivo
    //Buscar un archivo wav
    //Cargar el archivo wav

    //En la ventana de busqueda solo aparecen archivos wav
    chooser = std::make_unique<juce::FileChooser>("Seleccione un archivo wav", juce::File{}, "*.wav");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;


    //Se escogio el archivo
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {

            //Crear un objeto interno de juce para el archivo
            auto file = fc.getResult();

            //Si ese objeto no es tipo file
            if (file != juce::File{})                                                
            {
                //VAmos a acomodarlo para JUCE
                auto* reader = formatManager.createReaderFor(file);                 

                //Si no existe un archivo, entonces el nuevo archivo va a reproduccion
                if (reader != nullptr)
                {
                    //Enviamos el archivo a reproduccion en transportSource
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                    newSource -> setLooping(true);                                        
                   
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       
                    playButton.setEnabled(true);
                    startSlider.setEnabled(true);
                    //Este setPosition determina la posición inicial
                    transportSource.setPosition(startPosition);
                    readerSource.reset(newSource.release());
                    fileDuration = transportSource.getLengthInSeconds();
                    startPosition = (float)rand() / RAND_MAX * fileDuration - tail;
                   // startSlider.setRange(0.0, fileDuration);
                   // startSlider.repaint();
                }
            }
    });
}

void MainComponent::playButtonClick()
{
    changeState(Starting);
}

void MainComponent::stopButtonClick()
{
    changeState(Stopping);
}

void MainComponent::granKnob()
{
    granTime = granTime + minGranTime;
}
//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    numSamples = samplesPerBlockExpected;
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{

    //Si el archivo no es valido vamos a pasar silencio
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();

        return;
    }

    if (transportSource.getCurrentPosition() > (startPosition + granTime))
    {
        transportSource.setPosition(startPosition);
    }

    //de lo contrario vamos a meter el trasporSource (archivo wav) en el buffer
    transportSource.getNextAudioBlock(firstBuffer);

    for(int samp = 0; samp < bufferToFill.buffer->getNumSamples(); samp++)
    {
        for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); chan++)
        {
            bufferToFill.buffer->addSample(chan, samp, firstBuffer.buffer->getSample(chan, samp) + secondBuffer.buffer->getSample(chan, samp));
        }
    }


}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    openButton.setBounds(20, 20, getWidth() - 50, getHeight() / 6);
    playButton.setBounds(20, openButton.getBottom(), getWidth() - 50, getHeight() / 6);
    stopButton.setBounds(20, playButton.getBottom(), getWidth() - 50, getHeight() / 6);

    granSlider.setBounds(20, stopButton.getBottom(), getWidth() - 50, getHeight() / 8);
    //startSlider.setBounds(-80, granSlider.getBottom(), getWidth() - 50, getHeight() / 6);
}
