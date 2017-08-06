/******************************************************************************/
//        File: SourceInterface.cpp
/******************************************************************************/

#include "SourceInterface.h"

/******************************************************************************/
//        Function: SourceInterface
/******************************************************************************/
SourceInterface::SourceInterface()
{
}

/******************************************************************************/
//        Function: ~SourceInterface
/******************************************************************************/
SourceInterface::~SourceInterface()
{
}

/******************************************************************************/
//        Function: Init
/******************************************************************************/
void SourceInterface::Init()
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::Init - For ControlsInst");
    std::string domain1 = "local";
    std::string connection1 = "dbusconnection";
    std::string instance1 = "AudioPresCtrl.controlsInst";  //For Settings use - AudioPresCtrl.settingsInst

    audioControlsAvailable = false;
    #ifdef CONNECT_SERVICES
        audioControlsProxy = ResourceMaster::getInstance()->getRuntime()->buildProxy<audioPresCtrlControlsProxy>(domain1, instance1, connection1);
    #else
        audioControlsProxy = nullptr;
    #endif

    if (audioControlsProxy != nullptr)
    {
        LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::Init: AudioControlProxy is Found");
        audioControlsProxy->getProxyStatusEvent().subscribe(std::bind(&SourceInterface::statusEventListenerAudioControl, this, _1));
        audioControlsProxy->getEntSourceListAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveEntSourceList, this, _1));
        audioControlsProxy->getActiveAudioSourceAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveActiveAudioSource, this, _1));
    }

    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::Init - For SettingsInst");

    std::string domain2 = "local";
    std::string connection2 = "dbusconnection";
    std::string instance2 = "AudioPresCtrl.settingsInst";
    settingsProxyAvailable = false;

    #ifdef CONNECT_SERVICES
        audioSettingsProxy = ResourceMaster::getInstance()->getRuntime()->buildProxy<v1_0::com::harman::audio::audioPresCtrl::audioPresCtrlSettingsProxy>(domain2,instance2,connection2);
    #else
        audioSettingsProxy = nullptr;
    #endif

    if (audioSettingsProxy != nullptr)
    {
        LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "AudioResourceImpl::AudioResourceImpl: Audio Settings proxy Found");
        audioSettingsProxy->getProxyStatusEvent().subscribe(std::bind(&SourceInterface::statusEventListenerAudioSettings,this,_1));
    }
}

void SourceInterface::statusEventListenerAudioControl(CommonAPI::AvailabilityStatus status)
{
    if(status == CommonAPI::AvailabilityStatus::AVAILABLE )
    {
        audioControlsAvailable = true;
        if(audioControlsProxy != nullptr)
        {
            LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::statusEventListenerAudioControl: AudioControlProxy attach successful");
        }
    }
    else
    {
       audioControlsAvailable = false;
    }
}

/******************************************************************************/
//        Function: SendRequestInformationSource
/******************************************************************************/
void SourceInterface::SendRequestInformationSource()
{
    //TODO: Request for Active Ent Source
    if (audioControlsProxy != nullptr)
        audioControlsProxy->getActiveEntSrcAttribute().getValueAsync(std::bind(&SourceInterface::getActiveEntSrcValueAsyncCb, this, _1, _2));
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::SendRequestInformationSource");
    if (audioControlsProxy != nullptr)
        audioControlsProxy->SetEntertainmentSrcAsync(audioPresCtrlTypes::eEntertainmentSrcs::SRC_USB1, std::bind(&SourceInterface::SetEntertainmentSrcAsyncCb, this, _1 , _2));
}

void SourceInterface::SetEntertainmentSrcAsyncCb(const CommonAPI::CallStatus& status, const audioPresCtrlTypes::eAudioPresErrors& error)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::SetEntertainmentSrcAsyncCb - Status: %d, Error: %d", status, error);
}

void SourceInterface::getActiveEntSrcValueAsyncCb(const CommonAPI::CallStatus& status, uint64_t value)
{
    //audioPresCtrlTypes::eEntertainmentSrcs entSrc = (audioPresCtrlTypes::eEntertainmentSrcs::Literal)value;
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::getActiveEntSrcValueAsyncCb - Status: %d, ActiveEntSrcValue: %d", status, value);
}

/******************************************************************************/
//        Function: SendReleaseInformationSource
/******************************************************************************/
void SourceInterface::SendReleaseInformationSource()
{
}

/******************************************************************************/
//        Function: SendSetEntertainmentSource
/******************************************************************************/
void SourceInterface::SendSetEntertainmentSource()
{
    if (audioControlsProxy != nullptr)
        audioControlsProxy->SetEntertainmentSrcAsync(audioPresCtrlTypes::eEntertainmentSrcs::SRC_AUDIO_AUX, std::bind(&SourceInterface::SetEntertainmentSrcAsyncCb, this, _1 , _2));
}

/******************************************************************************/
//        Function: ReceiveActiveAudioSource
/******************************************************************************/
void SourceInterface::ReceiveActiveAudioSource(audioPresCtrlTypes::SourceId sourceId)
{
    QVariantMap data;
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::ReceiveActiveAudioSource - sourceId: %d", sourceId);
    ResourceUpdate("STR_ACTIVEAUDIOSOURCE", QVariant(data));
    return;
}

/******************************************************************************/
//        Function: ReceiveEntSourceList
/******************************************************************************/
void SourceInterface::ReceiveEntSourceList(audioPresCtrlTypes::EntSourceList sourceList)
{
    QVariantMap data;
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::ReceiveEntSourceList - sourceList Size: %d", sourceList.size());

    //for (auto it=sourceList.begin(); it != sourceList.end(); it++)
    for (std::unordered_map<audioPresCtrlTypes::eEntertainmentSrcs, audioPresCtrlTypes::eSourceAvailablity>::const_iterator it = sourceList.begin(); it != sourceList.end(); ++it)
    {
        data[QString::number(it->first)] = QVariant(it->second);
        LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::ReceiveEntSourceList - SourceID: %d, Availability: %d", it->first, it->second);
    }

    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::ReceiveEntSourceList - Send ResourceUpdate");

    ResourceUpdate("STR_ENTSOURCELIST", QVariant(data));

    return;
}

/******************************************************************************/
//        Function: SendSetBass
/******************************************************************************/
void SourceInterface::SendSetBass(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendSetBass :: %d",data.toInt());
    if( audioSettingsProxy != nullptr)
    {
       audioSettingsProxy->getBassAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: SendSetMidRange
/******************************************************************************/
void SourceInterface::SendSetMidRange(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendSetMidRange :: %d",data.toInt());
    if( audioSettingsProxy != nullptr)
    {
        audioSettingsProxy->getMidAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: SendSetTreble
/******************************************************************************/
void SourceInterface::SendSetTreble(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendSetTreble :: %d",data.toInt());
    if( audioSettingsProxy != nullptr)
    {
      audioSettingsProxy->getTrebleAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: SendSetBalance
/******************************************************************************/
void SourceInterface::SendSetBalance(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendSetBalance :: %d",data.toInt());

    if( audioSettingsProxy != nullptr)
    {
      audioSettingsProxy->getBalanceAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: SendSetFade
/******************************************************************************/
void SourceInterface::SendSetFade(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendSetFade :: %d",data.toInt());

    if( audioSettingsProxy != nullptr)
    {
      audioSettingsProxy->getFadeAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: SendsetSCV
/******************************************************************************/
void SourceInterface::SendsetSCV(QVariant data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SendsetSCV :: %d",data.toInt());

    if( audioSettingsProxy != nullptr)
    {
      audioSettingsProxy->getAVCModeAttribute().setValueAsync(data.toInt(),std::bind(&SourceInterface::setResponse,this,_1,_2));
    }
}

/******************************************************************************/
//        Function: ReceiveTREBLE
/******************************************************************************/
void SourceInterface::ReceiveTREBLE(int16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveTREBLE :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_TREBLE", QVariant(data));
    return;
}

/******************************************************************************/
//        Function: ReceiveMIDRANGE
/******************************************************************************/
void SourceInterface::ReceiveMIDRANGE(int16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveMIDRANGE :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_MIDRANGE", QVariant(data));
    return;
}

/******************************************************************************/
//        Function: ReceiveBASS
/******************************************************************************/
void SourceInterface::ReceiveBASS(int16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveBASS :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_BASS", QVariant(data));
    return;
}

/******************************************************************************/
//        Function: ReceiveBALANCE
/******************************************************************************/
void SourceInterface::ReceiveBALANCE(int16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveBALANCE :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_BALANCE", QVariant(data));

    return;
}

/******************************************************************************/
//        Function: ReceiveFADE
/******************************************************************************/
void SourceInterface::ReceiveFADE(int16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveFADE :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_FADE", QVariant(data));

    return;
}

/******************************************************************************/
//        Function: ReceiveSCV
/******************************************************************************/
void SourceInterface::ReceiveSCV(uint16_t data)
{
    LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "ReceiveSCV :: %d",QVariant(data).toInt());
    ResourceUpdate("STR_SCV", QVariant(data));

    return;
}
void SourceInterface::statusEventListenerAudioSettings(CommonAPI::AvailabilityStatus status)
{
    if(status == CommonAPI::AvailabilityStatus::AVAILABLE )
    {
        settingsProxyAvailable = true;
        if( audioSettingsProxy != nullptr)
        {
           audioSettingsProxy->getTrebleAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveTREBLE,this,_1));
           audioSettingsProxy->getMidAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveMIDRANGE,this,_1));
           audioSettingsProxy->getBassAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveBASS,this,_1));
           audioSettingsProxy->getBalanceAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveBALANCE,this,_1));
           audioSettingsProxy->getFadeAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveFADE,this,_1));
           audioSettingsProxy->getAVCModeAttribute().getChangedEvent().subscribe(std::bind(&SourceInterface::ReceiveSCV,this,_1));
        }
    }
    else
    {
       settingsProxyAvailable = false;
    }
}

void SourceInterface::setResponse(CommonAPI::CallStatus status, int value)
{
    if ( status == CommonAPI::CallStatus::SUCCESS)
    {
        LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::setResponse SUCCESS %d ",value );
    }
    else
    {
        LOG_INFO(Log::e_LOG_CONTEXT_AUDIO, "SourceInterface::setResponse FAIL %d", value);
    }
}
