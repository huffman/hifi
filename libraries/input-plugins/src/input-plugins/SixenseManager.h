//
//  SixenseManager.h
//  input-plugins/src/input-plugins
//
//  Created by Andrzej Kapolka on 11/15/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SixenseManager_h
#define hifi_SixenseManager_h

#ifdef HAVE_SIXENSE
    #include <glm/glm.hpp>
    #include <glm/gtc/quaternion.hpp>
    #include "sixense.h"

#ifdef __APPLE__
    #include <QCoreApplication>
    #include <qlibrary.h>
#endif

#endif

#include <SimpleMovingAverage.h>

#include <controllers/InputDevice.h>
#include <controllers/StandardControls.h>

#include "InputPlugin.h"

class QLibrary;

const unsigned int BUTTON_0 = 1U << 0; // the skinny button between 1 and 2
const unsigned int BUTTON_1 = 1U << 5;
const unsigned int BUTTON_2 = 1U << 6;
const unsigned int BUTTON_3 = 1U << 3;
const unsigned int BUTTON_4 = 1U << 4;
const unsigned int BUTTON_FWD = 1U << 7;
const unsigned int BUTTON_TRIGGER = 1U << 8;

const bool DEFAULT_INVERT_SIXENSE_MOUSE_BUTTONS = false;

// Handles interaction with the Sixense SDK (e.g., Razer Hydra).
class SixenseManager : public InputPlugin, public controller::InputDevice {
    Q_OBJECT
public:
    SixenseManager();
    
    // Plugin functions
    virtual bool isSupported() const override;
    virtual bool isJointController() const override { return true; }
    const QString& getName() const override { return NAME; }
    const QString& getID() const override { return HYDRA_ID_STRING; }

    virtual void activate() override;
    virtual void deactivate() override;

    virtual void pluginFocusOutEvent() override { focusOutEvent(); }
    virtual void pluginUpdate(float deltaTime, bool jointsCaptured) override { update(deltaTime, jointsCaptured); }

    // Device functions
    virtual controller::Input::NamedVector getAvailableInputs() const override;
    virtual QString getDefaultMappingConfig() const override;

    virtual void update(float deltaTime, bool jointsCaptured) override;
    virtual void focusOutEvent() override;

    virtual void saveSettings() const override;
    virtual void loadSettings() override;

public slots:
    void setSixenseFilter(bool filter);

private:    
    void handleButtonEvent(unsigned int buttons, bool left);
    void handleAxisEvent(float x, float y, float trigger, bool left);
    void handlePoseEvent(float deltaTime, glm::vec3 position, glm::quat rotation, bool left);

    void updateCalibration(void* controllers);
    
    int _calibrationState;

    // these are calibration results
    glm::vec3 _avatarPosition; // in hydra-frame
    glm::quat _avatarRotation; // in hydra-frame
    float _reachLength;

    // these are measured values used to compute the calibration results
    quint64 _lockExpiry;
    glm::vec3 _averageLeft;
    glm::vec3 _averageRight;
    glm::vec3 _reachLeft;
    glm::vec3 _reachRight;
    float _lastDistance;
    bool _useSixenseFilter = true;


    static const int MAX_NUM_AVERAGING_SAMPLES = 50; // At ~100 updates per seconds this means averaging over ~.5s
    using Samples = std::pair<  MovingAverage< glm::vec3, MAX_NUM_AVERAGING_SAMPLES>, MovingAverage< glm::vec4, MAX_NUM_AVERAGING_SAMPLES> >;
    using MovingAverageMap = std::map< int, Samples >;
    MovingAverageMap _collectedSamples;
    
#ifdef __APPLE__
    QLibrary* _sixenseLibrary { nullptr };
#endif
    
    static const QString NAME;
    static const QString HYDRA_ID_STRING;
};

#endif // hifi_SixenseManager_h

