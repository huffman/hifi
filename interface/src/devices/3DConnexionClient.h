//  3DConnexionClient.h
//  interface/src/devices
//
//  Created by Marcel Verhagen on 09-06-15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_3DConnexionClient_h
#define hifi_3DConnexionClient_h

#if 0
#include <QObject>
#include <QLibrary>
#include <controllers/UserInputMapper.h>

#include "InterfaceLogging.h"

#ifndef HAVE_3DCONNEXIONCLIENT
class ConnexionClient : public QObject {
    Q_OBJECT
public:
    static ConnexionClient& getInstance();
    void init() {};
    void destroy() {};
    bool Is3dmouseAttached() { return false; };
public slots:
    void toggleConnexion(bool shouldEnable) {};
};
#endif // NOT_HAVE_3DCONNEXIONCLIENT

#ifdef HAVE_3DCONNEXIONCLIENT
// the windows connexion rawinput
#ifdef Q_OS_WIN

#include "I3dMouseParams.h"
#include <QAbstractNativeEventFilter>
#include <QAbstractEventDispatcher>
#include <Winsock2.h>
#include <windows.h>

// windows rawinput parameters
class MouseParameters : public I3dMouseParam {
public:
    MouseParameters();

    // I3dmouseSensor interface
    bool IsPanZoom() const;
    bool IsRotate() const;
    Speed GetSpeed() const;

    void SetPanZoom(bool isPanZoom);
    void SetRotate(bool isRotate);
    void SetSpeed(Speed speed);

    // I3dmouseNavigation interface
    Navigation GetNavigationMode() const;
    Pivot GetPivotMode() const;
    PivotVisibility GetPivotVisibility() const;
    bool IsLockHorizon() const;

    void SetLockHorizon(bool bOn);
    void SetNavigationMode(Navigation navigation);
    void SetPivotMode(Pivot pivot);
    void SetPivotVisibility(PivotVisibility visibility);

    static bool Is3dmouseAttached();

private:
    MouseParameters(const MouseParameters&);
    const MouseParameters& operator = (const MouseParameters&);

    Navigation fNavigation;
    Pivot fPivot;
    PivotVisibility fPivotVisibility;
    bool fIsLockHorizon;

    bool fIsPanZoom;
    bool fIsRotate;
    Speed fSpeed;
};

class ConnexionClient : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    ConnexionClient() {};

    static ConnexionClient& getInstance();
    void init();
    void destroy();
    bool Is3dmouseAttached();
    
    ConnexionClient* client;

    I3dMouseParam& MouseParams();
    const I3dMouseParam& MouseParams() const;

    virtual void Move3d(HANDLE device, std::vector<float>& motionData);
    virtual void On3dmouseKeyDown(HANDLE device, int virtualKeyCode);
    virtual void On3dmouseKeyUp(HANDLE device, int virtualKeyCode);

    virtual bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) Q_DECL_OVERRIDE
    {
        MSG* msg = static_cast< MSG * >(message);
        return RawInputEventFilter(message,  result);
    }

public slots:
    void toggleConnexion(bool shouldEnable);

signals:
    void Move3d(std::vector<float>& motionData);
    void On3dmouseKeyDown(int virtualKeyCode);
    void On3dmouseKeyUp(int virtualKeyCode);

private:
    bool InitializeRawInput(HWND hwndTarget);

    bool RawInputEventFilter(void* msg, long* result);

    void OnRawInput(UINT nInputCode, HRAWINPUT hRawInput);
    UINT GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
    bool TranslateRawInputData(UINT nInputCode, PRAWINPUT pRawInput);
    void On3dmouseInput();

    class TInputData {
    public:
        TInputData() : fAxes(6) {}

        bool IsZero() {
            return (0.0f == fAxes[0] && 0.0f == fAxes[1] && 0.0f == fAxes[2] &&
                0.0f == fAxes[3] && 0.0f == fAxes[4] && 0.0f == fAxes[5]);
        }

        int fTimeToLive; // For telling if the device was unplugged while sending data
        bool fIsDirty;
        std::vector<float> fAxes;

    };

    HWND fWindow;

    // Data cache to handle multiple rawinput devices
    std::map< HANDLE, TInputData> fDevice2Data;
    std::map< HANDLE, unsigned long> fDevice2Keystate;

    // 3dmouse parameters
    MouseParameters f3dMouseParams;     // Rotate, Pan Zoom etc.

    // use to calculate distance traveled since last event
    DWORD fLast3dmouseInputTime;
};

// the osx connexion api
#else

#include <glm/glm.hpp>
#include "ConnexionClientAPI.h"

class ConnexionClient : public QObject {
    Q_OBJECT
public:
    static ConnexionClient& getInstance();
    void init();
    void destroy();
    bool Is3dmouseAttached();
public slots:
    void toggleConnexion(bool shouldEnable);
};

#endif // __APPLE__

#endif // HAVE_3DCONNEXIONCLIENT


// connnects to the userinputmapper
class ConnexionData : public QObject, public controller::InputDevice {
    Q_OBJECT

public:
    static ConnexionData& getInstance();
    ConnexionData();
    enum PositionChannel {
        TRANSLATE_X,
        TRANSLATE_Y,
        TRANSLATE_Z,
        ROTATE_X,
        ROTATE_Y,
        ROTATE_Z,
    };

    enum ButtonChannel {
        BUTTON_1 = 1,
        BUTTON_2 = 2,
        BUTTON_3 = 3
    };

    typedef std::unordered_set<int> ButtonPressedMap;
    typedef std::map<int, float> AxisStateMap;

    float getButton(int channel) const;
    float getAxis(int channel) const;

    controller::Input makeInput(ConnexionData::PositionChannel axis);
    controller::Input makeInput(ConnexionData::ButtonChannel button);
    virtual void buildDeviceProxy(controller::DeviceProxy::Pointer proxy) override;
    virtual QString getDefaultMappingConfig() override;
    virtual void update(float deltaTime, bool jointsCaptured) override;
    virtual void focusOutEvent() override;

    glm::vec3 cc_position;
    glm::vec3 cc_rotation;
    int clientId;

    void setButton(int lastButtonState);
    void handleAxisEvent();
};

#endif

#endif // defined(hifi_3DConnexionClient_h)
