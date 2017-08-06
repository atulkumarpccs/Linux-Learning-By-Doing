/******************************************************************************/
//        File: SourceInterface.h
/******************************************************************************/

#ifndef _SOURCEINTERFACE_H_
#define _SOURCEINTERFACE_H_
#include <functional>
using namespace std::placeholders;

class SourceInterface : public QObject
{
    Q_OBJECT

public:
     SourceInterface();
     ~SourceInterface();

    void Init();
    void SendSetBass(QVariant data);
    void SendSetMidRange(QVariant data);
    void SendSetTreble(QVariant data);
    void SendSetBalance(QVariant data);
    void SendSetFade(QVariant data);
    void SendsetSCV(QVariant data);

    void ReceiveTREBLE(int16_t data);
    void ReceiveMIDRANGE(int16_t data);
    void ReceiveBASS(int16_t data);
    void ReceiveBALANCE(int16_t data);
    void ReceiveFADE(int16_t data);
    void ReceiveSCV(uint16_t data);
	
	void SendRequestInformationSource();
    void SendReleaseInformationSource();
    void SendSetEntertainmentSource();
    void ReceiveActiveAudioSource(audioPresCtrlTypes::SourceId sourceId);
    void ReceiveEntSourceList(audioPresCtrlTypes::EntSourceList sourceList);

public:
  signals:
    void ResourceUpdate(QString propertyName, QVariant value);

private:
    std::shared_ptr<audioPresCtrlControlsProxy<>> audioControlsProxy;
    std::shared_ptr<audioPresCtrlSettingsProxy<>> audioSettingsProxy;

    bool settingsProxyAvailable;
    void statusEventListenerAudioSettings( CommonAPI::AvailabilityStatus status );
    void setResponse(CommonAPI::CallStatus status, int value);

    bool audioControlsAvailable;
    void statusEventListenerAudioControl(CommonAPI::AvailabilityStatus status);
    void SetEntertainmentSrcAsyncCb(const CommonAPI::CallStatus& status, const audioPresCtrlTypes::eAudioPresErrors& error);
    void getActiveEntSrcValueAsyncCb(const CommonAPI::CallStatus& status, uint64_t value);
};

#endif /* _SOURCEINTERFACE_H_ */
